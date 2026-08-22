/*
 * NAppGUI Cross-platform C SDK
 * MIT Licence
 *
 * File: test_encode.c
 *
 */

/* Tests del modulo encode: base64, url y json. */

#include "ntest.h"
#include <encode/base64.h>
#include <encode/url.h>
#include <encode/json.h>
#include <core/strings.h>
#include <core/stream.h>
#include <core/dbind.h>
#include <sewer/bmem.h>

/*---------------------------------------------------------------------------*/

static void i_base64(void)
{
    /* Vectores del RFC 4648. */
    static const char_t *ENTRADA[] = {"", "f", "fo", "foo", "foob", "fooba", "foobar"};
    static const char_t *SALIDA[] = {"", "Zg==", "Zm8=", "Zm9v", "Zm9vYg==", "Zm9vYmE=", "Zm9vYmFy"};
    uint32_t i = 0;

    for (i = 0; i < sizeof(ENTRADA) / sizeof(ENTRADA[0]); ++i)
    {
        char_t enc[64];
        byte_t dec[64];
        uint32_t size = (uint32_t)str_len_c(ENTRADA[i]);
        uint32_t n = 0;

        bmem_set_zero(cast(enc, byte_t), sizeof32(enc));
        n = b64_encode(cast_const(ENTRADA[i], byte_t), size, enc, sizeof32(enc));
        ntest_equ_str(enc, SALIDA[i], "b64_encode contra el vector del RFC 4648");

        bmem_set_zero(dec, sizeof32(dec));
        n = b64_decode(enc, n, dec);
        ntest_equ_u32(n, size, "b64_decode devuelve el tamano original");
        ntest_equ_i32(bmem_cmp(dec, cast_const(ENTRADA[i], byte_t), size), 0,
                      "b64_decode recupera el contenido original");
    }

    /* Los tamanos anunciados deben bastar. */
    ntest_true(b64_encoded_size(3) >= 4, "encoded_size de 3 bytes es al menos 4");
    ntest_true(b64_decoded_size(4) >= 3, "decoded_size de 4 caracteres es al menos 3");

    /* Datos binarios con ceros intercalados. */
    {
        byte_t bin[8];
        char_t enc[32];
        byte_t dec[32];
        uint32_t n = 0;
        uint32_t i2 = 0;

        for (i2 = 0; i2 < 8; ++i2)
            bin[i2] = (byte_t)(i2 * 37);
        bin[3] = 0;

        n = b64_encode(bin, 8, enc, sizeof32(enc));
        n = b64_decode(enc, n, dec);
        ntest_equ_u32(n, 8, "ida y vuelta de datos binarios: tamano");
        ntest_equ_i32(bmem_cmp(dec, bin, 8), 0, "ida y vuelta de datos binarios: contenido");
    }
}

/*---------------------------------------------------------------------------*/

static void i_url(void)
{
    Url *url = url_parse("http://usuario:clave@ejemplo.com:8080/una/ruta?a=1&b=2#trozo");

    if (ntest_true(url != NULL, "url_parse acepta una URL completa"))
    {
        ntest_equ_str(url_scheme(url), "http", "esquema");
        ntest_equ_str(url_host(url), "ejemplo.com", "host");
        ntest_equ_str(url_user(url), "usuario", "usuario");
        ntest_equ_str(url_pass(url), "clave", "clave");
        ntest_equ_u32(url_port(url), 8080, "puerto");
        ntest_equ_str(url_path(url), "/una/ruta", "ruta");
        ntest_equ_str(url_fragment(url), "trozo", "fragmento");
        url_destroy(&url);
        ntest_true(url == NULL, "url_destroy anula el puntero");
    }

    /* Una URL minima tambien debe funcionar. */
    {
        Url *simple = url_parse("http://ejemplo.com");
        if (ntest_true(simple != NULL, "url_parse acepta una URL minima"))
        {
            ntest_equ_str(url_host(simple), "ejemplo.com", "host de la URL minima");
            url_destroy(&simple);
        }
    }
}

/*---------------------------------------------------------------------------*/

typedef struct _punto_t Punto;

struct _punto_t
{
    int32_t x;
    int32_t y;
    String *nombre;
};

/*---------------------------------------------------------------------------*/

static void i_json(void)
{
    Punto *p = NULL;

    dbind(Punto, int32_t, x);
    dbind(Punto, int32_t, y);
    dbind(Punto, String *, nombre);

    /* Lectura de un JSON valido. */
    {
        Stream *stm = stm_from_block(cast_const("{\"x\":10,\"y\":-20,\"nombre\":\"origen\"}", byte_t), 34);
        p = json_read(stm, NULL, Punto);
        stm_close(&stm);

        if (ntest_true(p != NULL, "json_read acepta un JSON valido"))
        {
            ntest_equ_i32(p->x, 10, "campo x");
            ntest_equ_i32(p->y, -20, "campo y");
            ntest_equ_str(tc(p->nombre), "origen", "campo de texto");
        }
    }

    /* Ida y vuelta: escribir y volver a leer debe dar lo mismo. */
    if (p != NULL)
    {
        Stream *out = stm_memory(256);
        Punto *q = NULL;
        json_write(out, p, NULL, Punto);
        q = json_read(out, NULL, Punto);
        if (ntest_true(q != NULL, "ida y vuelta json_write -> json_read"))
        {
            ntest_equ_i32(q->x, p->x, "ida y vuelta conserva x");
            ntest_equ_i32(q->y, p->y, "ida y vuelta conserva y");
            ntest_equ_str(tc(q->nombre), tc(p->nombre), "ida y vuelta conserva el texto");
            json_destroy(&q, Punto);
        }
        stm_close(&out);
        json_destroy(&p, Punto);
    }

    /* Un JSON malformado debe devolver NULL, no reventar. */
    {
        Stream *malo = stm_from_block(cast_const("{\"x\":", byte_t), 5);
        Punto *r = json_read(malo, NULL, Punto);
        ntest_true(r == NULL, "json_read devuelve NULL con un JSON truncado");
        json_destopt(&r, Punto);
        stm_close(&malo);
    }

    /* Una cadena JSON sobre un campo entero tiene que ser un numero. La
       conversion se reintentaba en base 16 sin mirar el prefijo, asi que "abc"
       entraba como 2748 sin avisar. Un campo que no convierte se queda en su
       valor por defecto (json.c no aborta el objeto entero). Ver NAP-026. */
    {
        const char_t *js = "{\"x\":\"abc\",\"y\":7,\"nombre\":\"n\"}";
        Stream *stm = stm_from_block(cast_const(js, byte_t), str_len_c(js));
        Punto *r = json_read(stm, NULL, Punto);
        if (ntest_true(r != NULL, "json_read sigue con el resto del objeto"))
        {
            ntest_equ_i32(r->x, 0, "json_read no acepta la cadena \"abc\" como hexadecimal");
            ntest_equ_i32(r->y, 7, "json_read conserva el resto de campos");
        }
        json_destopt(&r, Punto);
        stm_close(&stm);
    }

    /* Con el prefijo explicito si es un numero. */
    {
        const char_t *js = "{\"x\":\"0x1f\",\"y\":0,\"nombre\":\"n\"}";
        Stream *stm = stm_from_block(cast_const(js, byte_t), str_len_c(js));
        Punto *r = json_read(stm, NULL, Punto);
        if (ntest_true(r != NULL, "json_read acepta la cadena \"0x1f\" en un campo entero"))
            ntest_equ_i32(r->x, 31, "json_read convierte \"0x1f\" en 31");
        json_destopt(&r, Punto);
        stm_close(&stm);
    }

    dbind_unreg(Punto);
}

/*---------------------------------------------------------------------------*/

uint32_t ntest_encode(void)
{
    ntest_begin("encode");
    i_base64();
    i_url();
    i_json();
    return ntest_end();
}
