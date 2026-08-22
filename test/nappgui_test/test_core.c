/*
 * NAppGUI Cross-platform C SDK
 * MIT Licence
 *
 * File: test_core.c
 *
 */

/* Tests del modulo core: contenedores, strings y streams. */

#include "ntest.h"
#include <core/arrst.h>
#include <core/arrpt.h>
#include <core/strings.h>
#include <core/stream.h>
#include <core/buffer.h>
#include <core/heap.h>
#include <sewer/bmem.h>

/*---------------------------------------------------------------------------*/

static int i_cmp_u32(const uint32_t *a, const uint32_t *b)
{
    if (*a < *b)
        return -1;
    if (*a > *b)
        return 1;
    return 0;
}

/*---------------------------------------------------------------------------*/

static void i_arrst(void)
{
    ArrSt(uint32_t) *a = arrst_create(uint32_t);
    uint32_t i = 0;

    ntest_equ_u32(arrst_size(a, uint32_t), 0, "un array recien creado esta vacio");

    for (i = 0; i < 10; ++i)
    {
        uint32_t *e = arrst_new(a, uint32_t);
        *e = 10 - i;
    }

    ntest_equ_u32(arrst_size(a, uint32_t), 10, "tamano tras 10 inserciones");
    ntest_equ_u32(*arrst_get(a, 0, uint32_t), 10, "primer elemento");
    ntest_equ_u32(*arrst_last(a, uint32_t), 1, "ultimo elemento");

    /* Ordenar deja el minimo delante. */
    arrst_sort(a, i_cmp_u32, uint32_t);
    ntest_equ_u32(*arrst_get(a, 0, uint32_t), 1, "tras ordenar, el primero es el menor");
    ntest_equ_u32(*arrst_last(a, uint32_t), 10, "tras ordenar, el ultimo es el mayor");

    /* Y queda realmente ordenado de principio a fin. */
    {
        bool_t ordenado = TRUE;
        for (i = 1; i < arrst_size(a, uint32_t); ++i)
        {
            if (*arrst_get(a, i - 1, uint32_t) > *arrst_get(a, i, uint32_t))
                ordenado = FALSE;
        }
        ntest_true(ordenado, "el array queda ordenado por completo");
    }

    /* Busqueda binaria sobre el array ordenado. */
    {
        uint32_t clave = 7;
        uint32_t pos = UINT32_MAX;
        uint32_t *hallado = arrst_bsearch(a, i_cmp_u32, &clave, &pos, uint32_t, uint32_t);
        if (ntest_true(hallado != NULL, "bsearch encuentra un elemento presente"))
            ntest_equ_u32(*hallado, 7, "bsearch devuelve el elemento correcto");

        clave = 999;
        hallado = arrst_bsearch(a, i_cmp_u32, &clave, &pos, uint32_t, uint32_t);
        ntest_true(hallado == NULL, "bsearch devuelve NULL si no esta");
    }

    /* Borrar reduce el tamano. */
    arrst_delete(a, 0, NULL, uint32_t);
    ntest_equ_u32(arrst_size(a, uint32_t), 9, "tamano tras borrar uno");
    ntest_equ_u32(*arrst_get(a, 0, uint32_t), 2, "el siguiente pasa a ser el primero");

    arrst_clear(a, NULL, uint32_t);
    ntest_equ_u32(arrst_size(a, uint32_t), 0, "clear vacia el array");

    arrst_destroy(&a, NULL, uint32_t);
    ntest_true(a == NULL, "destroy anula el puntero del llamante");
}

/*---------------------------------------------------------------------------*/

static void i_arrpt(void)
{
    ArrPt(String) *a = arrpt_create(String);

    arrpt_append(a, str_c("uno"), String);
    arrpt_append(a, str_c("dos"), String);
    arrpt_append(a, str_c("tres"), String);

    ntest_equ_u32(arrpt_size(a, String), 3, "tamano del array de punteros");
    ntest_equ_str(tc(arrpt_get(a, 0, String)), "uno", "primer elemento");
    ntest_equ_str(tc(arrpt_get(a, 2, String)), "tres", "ultimo elemento");

    /* destroy con func_destroy libera tambien los elementos: si no lo hiciera,
       el auditor de memoria lo delataria al cerrar. */
    arrpt_destroy(&a, str_destroy, String);
    ntest_true(a == NULL, "destroy anula el puntero");
}

/*---------------------------------------------------------------------------*/

static void i_strings(void)
{
    String *s = str_c("Hola Mundo");

    ntest_equ_str(tc(s), "Hola Mundo", "str_c copia el literal");
    ntest_equ_u32(str_len(s), 10, "str_len cuenta bytes");
    ntest_equ_u32(str_nchars(s), 10, "str_nchars cuenta caracteres");

    /* Con acentos, bytes y caracteres difieren. */
    {
        String *acc = str_c("a\xC3\xB1o");  /* "año" */
        ntest_equ_u32(str_len(acc), 4, "str_len con acentos cuenta bytes");
        ntest_equ_u32(str_nchars(acc), 3, "str_nchars con acentos cuenta caracteres");
        str_destroy(&acc);
    }

    ntest_equ_i32(str_cmp(s, "Hola Mundo"), 0, "str_cmp de dos iguales");
    ntest_true(str_cmp(s, "Otra cosa") != 0, "str_cmp de dos distintos");
    ntest_true(str_equ(s, "Hola Mundo"), "str_equ");
    ntest_true(str_equ_nocase(tc(s), "hola mundo"), "str_equ_nocase ignora mayusculas");

    ntest_true(str_empty_c(""), "la cadena vacia esta vacia");
    ntest_false(str_empty_c("x"), "una cadena con contenido no esta vacia");

    {
        String *upper = str_copy(s);
        str_upper(upper);
        ntest_equ_str(tc(upper), "HOLA MUNDO", "str_upper");
        str_lower(upper);
        ntest_equ_str(tc(upper), "hola mundo", "str_lower");
        str_destroy(&upper);
    }

    {
        String *p = str_printf("%s tiene %d anos", "Ana", 30);
        ntest_equ_str(tc(p), "Ana tiene 30 anos", "str_printf");
        str_destroy(&p);
    }

    {
        String *r = str_repl(tc(s), "Mundo", "Todos", NULL);
        ntest_equ_str(tc(r), "Hola Todos", "str_repl sustituye");
        str_destroy(&r);
    }

    /* Conversiones numericas, incluido el caso de error. El criterio esta
       documentado en core/strings.h: espacios alrededor si, basura no. */
    {
        bool_t err = FALSE;
        ntest_equ_i32(str_to_i32("-42", 10, &err), -42, "str_to_i32");
        ntest_false(err, "str_to_i32 no marca error con entrada valida");

        err = FALSE;
        ntest_equ_i32(str_to_i32("no soy un numero", 10, &err), 0, "str_to_i32 con entrada invalida devuelve 0");
        ntest_true(err, "str_to_i32 marca error con entrada invalida");

        /* Cadena vacia: no hay ni un digito. */
        err = FALSE;
        ntest_equ_i32(str_to_i32("", 10, &err), 0, "str_to_i32 con cadena vacia devuelve 0");
        ntest_true(err, "str_to_i32 marca error con la cadena vacia");

        /* Solo espacios: tampoco hay digitos. */
        err = FALSE;
        str_to_i32("   ", 10, &err);
        ntest_true(err, "str_to_i32 marca error con solo espacios");

        /* Basura detras del numero. */
        err = FALSE;
        ntest_equ_i32(str_to_i32("42abc", 10, &err), 0, "str_to_i32 con basura detras devuelve 0");
        ntest_true(err, "str_to_i32 marca error con basura detras del numero");

        /* Espacios alrededor: validos, se ignoran. */
        err = TRUE;
        ntest_equ_i32(str_to_i32(" 42 ", 10, &err), 42, "str_to_i32 ignora los espacios alrededor");
        ntest_false(err, "str_to_i32 no marca error con espacios alrededor");

        /* Un espacio en medio parte el numero: es error. */
        err = FALSE;
        str_to_i32("4 2", 10, &err);
        ntest_true(err, "str_to_i32 marca error con un espacio en medio");

        /* Signo mas explicito. */
        err = TRUE;
        ntest_equ_i32(str_to_i32("+42", 10, &err), 42, "str_to_i32 acepta el signo mas");
        ntest_false(err, "str_to_i32 no marca error con el signo mas");

        /* Negativo en una funcion sin signo: error de formato, no un valor enorme. */
        err = FALSE;
        ntest_equ_u32(str_to_u32("-5", 10, &err), 0, "str_to_u32 con un negativo devuelve 0");
        ntest_true(err, "str_to_u32 marca error con un negativo");

        /* Bases distintas de 10. */
        err = TRUE;
        ntest_equ_u32(str_to_u32("255", 16, &err), 597, "str_to_u32 en base 16");
        ntest_false(err, "str_to_u32 no marca error en base 16");

        err = TRUE;
        ntest_equ_u32(str_to_u32("ff", 16, &err), 255, "str_to_u32 en base 16 con letras minusculas");
        ntest_false(err, "str_to_u32 no marca error con letras hexadecimales validas");

        err = TRUE;
        ntest_equ_u32(str_to_u32("FF", 16, &err), 255, "str_to_u32 en base 16 con letras mayusculas");
        ntest_false(err, "str_to_u32 no marca error con letras hexadecimales mayusculas");

        err = FALSE;
        ntest_equ_u32(str_to_u32("gg", 16, &err), 0, "str_to_u32 con letras no hexadecimales devuelve 0");
        ntest_true(err, "str_to_u32 marca error con letras no hexadecimales");

        err = TRUE;
        ntest_equ_u32(str_to_u32("1010", 2, &err), 10, "str_to_u32 en base 2");
        ntest_false(err, "str_to_u32 no marca error en base 2");

        err = FALSE;
        ntest_equ_u32(str_to_u32("1012", 2, &err), 0, "str_to_u32 con un digito fuera de la base 2 devuelve 0");
        ntest_true(err, "str_to_u32 marca error con un digito fuera de la base");

        /* Desbordamiento: el valor satura al limite del tipo y avisa. */
        err = FALSE;
        ntest_equ_u32(str_to_u32("99999999999999999999", 10, &err), UINT32_MAX, "str_to_u32 satura al desbordar");
        ntest_true(err, "str_to_u32 marca error al desbordar");

        err = FALSE;
        ntest_equ_i32(str_to_i32("99999999999999999999", 10, &err), INT32_MAX, "str_to_i32 satura al desbordar");
        ntest_true(err, "str_to_i32 marca error al desbordar");

        err = FALSE;
        ntest_true(str_to_u64("99999999999999999999", 10, &err) == UINT64_MAX, "str_to_u64 satura al desbordar 64 bits");
        ntest_true(err, "str_to_u64 marca error al desbordar 64 bits");

        /* Los limites exactos si caben. */
        err = TRUE;
        ntest_true(str_to_i64("-9223372036854775808", 10, &err) == INT64_MIN, "str_to_i64 con el minimo de 64 bits");
        ntest_false(err, "str_to_i64 no marca error con el minimo de 64 bits");

        err = TRUE;
        ntest_true(str_to_u64("18446744073709551615", 10, &err) == UINT64_MAX, "str_to_u64 con el maximo de 64 bits");
        ntest_false(err, "str_to_u64 no marca error con el maximo de 64 bits");

        /* str_to_r32/r64 no siguen el mismo criterio: rechazan la basura detras
           pero aceptan la cadena vacia como 0 sin avisar. Ver NAP-023. */
        err = FALSE;
        str_to_r32("no soy un numero", &err);
        ntest_true(err, "str_to_r32 marca error con basura");

        err = FALSE;
        str_to_r32("", &err);
        if (err == FALSE)
            ntest_pending("str_to_r32 deberia marcar error con la cadena vacia", "NAP-023");
        else
            ntest_true(err, "str_to_r32 marca error con la cadena vacia");
    }

    /* split_pathname sobre una ruta. */
    {
        String *path = NULL;
        String *file = NULL;
        str_split_pathname("/una/ruta/fichero.txt", &path, &file);
        ntest_equ_str(tc(path), "/una/ruta", "split_pathname devuelve el directorio");
        ntest_equ_str(tc(file), "fichero.txt", "split_pathname devuelve el fichero");
        str_destroy(&path);
        str_destroy(&file);
    }

    str_destroy(&s);
    ntest_true(s == NULL, "str_destroy anula el puntero");

    /* str_destopt admite NULL sin quejarse. */
    {
        String *nulo = NULL;
        str_destopt(&nulo);
        ntest_true(nulo == NULL, "str_destopt tolera NULL");
    }
}

/*---------------------------------------------------------------------------*/

static void i_stream(void)
{
    Stream *stm = stm_memory(256);
    uint32_t u32 = 0;
    int16_t i16 = 0;
    real32_t r32 = 0;

    /* Ida y vuelta de tipos basicos. */
    stm_write_u32(stm, 123456789);
    stm_write_i16(stm, -1234);
    stm_write_r32(stm, 3.5f);
    stm_write(stm, (const byte_t *)"abc", 3);

    u32 = stm_read_u32(stm);
    ntest_equ_u32(u32, 123456789, "ida y vuelta de un uint32");
    i16 = stm_read_i16(stm);
    ntest_equ_i32(i16, -1234, "ida y vuelta de un int16");
    r32 = stm_read_r32(stm);
    ntest_equ_r32(r32, 3.5f, "ida y vuelta de un real32");

    {
        byte_t buf[4];
        bmem_set_zero(buf, 4);
        stm_read(stm, buf, 3);
        ntest_equ_str((const char_t *)buf, "abc", "ida y vuelta de bytes crudos");
    }

    ntest_true(stm_state(stm) == ekSTOK, "el stream sigue en buen estado");

    /* Leer mas alla del final marca el estado, no revienta. */
    stm_read_u32(stm);
    ntest_true(stm_state(stm) != ekSTOK, "leer pasado el final cambia el estado");

    stm_close(&stm);
    ntest_true(stm == NULL, "stm_close anula el puntero");

    /* Endianness explicito: el mismo valor debe salir distinto en cada orden. */
    {
        Stream *le = stm_memory(16);
        Stream *be = stm_memory(16);
        byte_t bufle[4];
        byte_t bufbe[4];

        stm_set_write_endian(le, ekLITEND);
        stm_set_write_endian(be, ekBIGEND);
        stm_write_u32(le, 0x01020304);
        stm_write_u32(be, 0x01020304);
        stm_read(le, bufle, 4);
        stm_read(be, bufbe, 4);

        ntest_equ_u32(bufle[0], 0x04, "little endian escribe el byte bajo primero");
        ntest_equ_u32(bufbe[0], 0x01, "big endian escribe el byte alto primero");

        stm_close(&le);
        stm_close(&be);
    }

    /* Texto por lineas. */
    {
        Stream *txt = stm_memory(128);
        const char_t *linea = NULL;
        stm_writef(txt, "primera\nsegunda\n");
        linea = stm_read_line(txt);
        ntest_equ_str(linea, "primera", "primera linea");
        linea = stm_read_line(txt);
        ntest_equ_str(linea, "segunda", "segunda linea");
        stm_close(&txt);
    }
}

/*---------------------------------------------------------------------------*/

static void i_buffer(void)
{
    Buffer *b = buffer_create(16);
    ntest_equ_u32(buffer_size(b), 16, "tamano del buffer");
    bmem_set1(buffer_data(b), 16, 0x5A);
    ntest_equ_u32(buffer_data(b)[0], 0x5A, "escritura y lectura en el buffer");
    buffer_destroy(&b);
    ntest_true(b == NULL, "buffer_destroy anula el puntero");

    {
        Buffer *c = buffer_with_data((const byte_t *)"hola", 4);
        ntest_equ_u32(buffer_size(c), 4, "buffer_with_data respeta el tamano");
        ntest_equ_u32(buffer_const(c)[0], 'h', "buffer_with_data copia el contenido");
        buffer_destroy(&c);
    }
}

/*---------------------------------------------------------------------------*/

uint32_t ntest_core(void)
{
    ntest_begin("core");
    i_arrst();
    i_arrpt();
    i_strings();
    i_stream();
    i_buffer();
    return ntest_end();
}
