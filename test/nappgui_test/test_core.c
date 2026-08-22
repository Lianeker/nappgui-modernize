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
#include <core/date.h>
#include <core/dbind.h>
#include <core/dbindh.h>
#include <core/tfilter.h>
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

        /* Saturacion por abajo: un digito mas que INT64_MIN. */
        err = FALSE;
        ntest_true(str_to_i64("-9223372036854775809", 10, &err) == INT64_MIN, "str_to_i64 satura a INT64_MIN al desbordar por abajo");
        ntest_true(err, "str_to_i64 marca error al desbordar por abajo");

        /* El prefijo "0x" solo vale en base 16 y solo con un digito detras. */
        err = TRUE;
        ntest_equ_u32(str_to_u32("0x1f", 16, &err), 31, "str_to_u32 acepta el prefijo 0x en base 16");
        ntest_false(err, "str_to_u32 no marca error con el prefijo 0x en base 16");

        err = TRUE;
        ntest_equ_i32(str_to_i32("-0X10", 16, &err), -16, "str_to_i32 acepta el prefijo 0X con signo");
        ntest_false(err, "str_to_i32 no marca error con el prefijo 0X con signo");

        err = FALSE;
        ntest_equ_u32(str_to_u32("0x1f", 10, &err), 0, "str_to_u32 rechaza el prefijo 0x en base 10");
        ntest_true(err, "str_to_u32 marca error con el prefijo 0x fuera de la base 16");

        err = FALSE;
        ntest_equ_u32(str_to_u32("0x", 16, &err), 0, "str_to_u32 rechaza el prefijo 0x sin digitos detras");
        ntest_true(err, "str_to_u32 marca error con el prefijo 0x sin digitos detras");

        /* Bases fuera de 2..36: error, no una conversion silenciosa. */
        err = FALSE;
        ntest_equ_u32(str_to_u32("42", 0, &err), 0, "str_to_u32 rechaza la base 0");
        ntest_true(err, "str_to_u32 marca error con la base 0");

        err = FALSE;
        ntest_equ_u32(str_to_u32("42", 1, &err), 0, "str_to_u32 rechaza la base 1");
        ntest_true(err, "str_to_u32 marca error con la base 1");

        err = FALSE;
        ntest_equ_u32(str_to_u32("42", 37, &err), 0, "str_to_u32 rechaza una base mayor que 36");
        ntest_true(err, "str_to_u32 marca error con una base mayor que 36");

        /* str_to_r32/r64 siguen el mismo criterio que las enteras: toda la
           cadena tiene que ser el numero, los espacios de alrededor se ignoran
           y hace falta al menos un digito. Ver NAP-023. */
        err = FALSE;
        ntest_equ_r32(str_to_r32("no soy un numero", &err), 0, "str_to_r32 devuelve 0 con basura");
        ntest_true(err, "str_to_r32 marca error con basura");

        err = FALSE;
        ntest_equ_r32(str_to_r32("abc", &err), 0, "str_to_r32 devuelve 0 con letras");
        ntest_true(err, "str_to_r32 marca error con letras");

        /* La cadena vacia es un error, no un 0 silencioso: era el defecto. */
        err = FALSE;
        ntest_equ_r32(str_to_r32("", &err), 0, "str_to_r32 devuelve 0 con la cadena vacia");
        ntest_true(err, "str_to_r32 marca error con la cadena vacia");

        err = FALSE;
        ntest_equ_r32(str_to_r64("", &err), 0, "str_to_r64 devuelve 0 con la cadena vacia");
        ntest_true(err, "str_to_r64 marca error con la cadena vacia");

        err = FALSE;
        ntest_equ_r32(str_to_r32("  ", &err), 0, "str_to_r32 devuelve 0 con solo espacios");
        ntest_true(err, "str_to_r32 marca error con solo espacios");

        /* Los espacios de alrededor se aceptan, igual que en str_to_iXX. */
        err = TRUE;
        ntest_equ_r32(str_to_r32(" 42 ", &err), 42, "str_to_r32 acepta los espacios alrededor del numero");
        ntest_false(err, "str_to_r32 no marca error con los espacios alrededor");

        err = TRUE;
        ntest_equ_i32(str_to_i32(" 42 ", 10, &err), 42, "str_to_i32 si acepta el espacio detras del numero");
        ntest_false(err, "str_to_i32 no marca error con un espacio detras del numero");

        err = TRUE;
        ntest_equ_r32(str_to_r32("42 ", &err), 42, "str_to_r32 acepta un espacio detras del numero");
        ntest_false(err, "str_to_r32 no marca error con un espacio detras del numero");

        /* Pero la basura pegada al numero no. */
        err = FALSE;
        ntest_equ_r32(str_to_r32("42abc", &err), 0, "str_to_r32 devuelve 0 con basura detras del numero");
        ntest_true(err, "str_to_r32 marca error con basura detras del numero");

        /* Formatos validos: signo, decimales sin parte entera y exponente. */
        err = TRUE;
        ntest_equ_r32(str_to_r32("-3.5", &err), -3.5, "str_to_r32 con un negativo con decimales");
        ntest_false(err, "str_to_r32 no marca error con un negativo con decimales");

        err = TRUE;
        ntest_equ_r32(str_to_r32(".5", &err), 0.5, "str_to_r32 sin parte entera");
        ntest_false(err, "str_to_r32 no marca error sin parte entera");

        err = TRUE;
        ntest_equ_r32(str_to_r32("1e-3", &err), 0.001, "str_to_r32 con exponente negativo");
        ntest_false(err, "str_to_r32 no marca error con exponente");

        err = TRUE;
        ntest_true(str_to_r64("-3.5", &err) == -3.5, "str_to_r64 con un negativo con decimales");
        ntest_false(err, "str_to_r64 no marca error con un negativo con decimales");

        /* Un exponente sin digitos detras no es un numero. */
        err = FALSE;
        ntest_equ_r32(str_to_r32("1e", &err), 0, "str_to_r32 devuelve 0 con un exponente vacio");
        ntest_true(err, "str_to_r32 marca error con un exponente vacio");

        /* La coma no es separador decimal en ninguna localizacion. */
        err = FALSE;
        ntest_equ_r32(str_to_r32("1,5", &err), 0, "str_to_r32 devuelve 0 con una coma decimal");
        ntest_true(err, "str_to_r32 marca error con una coma decimal");

        /* Desbordamiento: la libc lo avisa por ERANGE. */
        err = FALSE;
        str_to_r32("1e400", &err);
        ntest_true(err, "str_to_r32 marca error al desbordar");

        err = FALSE;
        str_to_r64("1e400", &err);
        ntest_true(err, "str_to_r64 marca error al desbordar");

        /* Y ese ERANGE no se cuela en la llamada siguiente: antes se leia
           'errno' sin reiniciarlo antes de llamar a la libc. */
        err = TRUE;
        ntest_equ_r32(str_to_r32("1.5", &err), 1.5, "str_to_r32 convierte 1.5 tras un desbordamiento");
        ntest_false(err, "str_to_r32 no arrastra el ERANGE de la llamada anterior");

        err = TRUE;
        ntest_true(str_to_r64("1.5", &err) == 1.5, "str_to_r64 convierte 1.5 tras un desbordamiento");
        ntest_false(err, "str_to_r64 no arrastra el ERANGE de la llamada anterior");
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

/* Deja el destino en un valor centinela y aplica la cadena. Devuelve el
   resultado del enlace; el centinela permite comprobar que un rechazo no
   escribe nada. */
static bindset_t i_bind_set(const DBind *bind, int32_t *dest, const char_t *str)
{
    *dest = -1;
    return dbind_set_value_str(bind, cast(dest, byte_t), str);
}

/*---------------------------------------------------------------------------*/

/* 'dbind_set_value_str' sobre un entero: es por donde entra el texto de un
   Edit enlazado (gui/editimp.c -> gui/layout.c -> gui/gbind.c) y el de una
   cadena JSON sobre un campo entero (encode/json.c). Reintentaba la conversion
   en base 16 con cualquier cadena, asi que "abc" valia como 2748. Ver NAP-026. */
static void i_dbind_str(void)
{
    const DBind *bind = dbind_from_typename("int32_t", NULL);
    int32_t v = 0;

    if (ntest_true(bind != NULL, "el tipo int32_t esta registrado en dbind") == FALSE)
        return;

    /* Un decimal correcto entra. */
    ntest_true(i_bind_set(bind, &v, "42") != ekBINDSET_NOT_ALLOWED, "dbind acepta \"42\" en un entero");
    ntest_equ_i32(v, 42, "dbind convierte \"42\" en 42");

    /* Un hexadecimal explicito tambien: no hay forma de confundirlo. */
    ntest_true(i_bind_set(bind, &v, "0x1f") != ekBINDSET_NOT_ALLOWED, "dbind acepta \"0x1f\" en un entero");
    ntest_equ_i32(v, 31, "dbind convierte \"0x1f\" en 31");

    /* Sin el prefijo, las letras a-f no convierten la cadena en un numero. */
    ntest_true(i_bind_set(bind, &v, "abc") == ekBINDSET_NOT_ALLOWED, "dbind rechaza \"abc\" en un entero");
    ntest_equ_i32(v, -1, "dbind no toca el destino al rechazar \"abc\"");

    ntest_true(i_bind_set(bind, &v, "1f") == ekBINDSET_NOT_ALLOWED, "dbind rechaza \"1f\" en un entero");
    ntest_equ_i32(v, -1, "dbind no toca el destino al rechazar \"1f\"");

    ntest_true(i_bind_set(bind, &v, "12e") == ekBINDSET_NOT_ALLOWED, "dbind rechaza \"12e\" en un entero");
    ntest_equ_i32(v, -1, "dbind no toca el destino al rechazar \"12e\"");

    ntest_true(i_bind_set(bind, &v, "42abc") == ekBINDSET_NOT_ALLOWED, "dbind rechaza \"42abc\" en un entero");
    ntest_equ_i32(v, -1, "dbind no toca el destino al rechazar \"42abc\"");

    /* Y lo que ya se rechazaba antes se sigue rechazando. */
    ntest_true(i_bind_set(bind, &v, "hola") == ekBINDSET_NOT_ALLOWED, "dbind rechaza \"hola\" en un entero");
    ntest_equ_i32(v, -1, "dbind no toca el destino al rechazar \"hola\"");

    ntest_true(i_bind_set(bind, &v, "") == ekBINDSET_NOT_ALLOWED, "dbind rechaza la cadena vacia en un entero");
    ntest_equ_i32(v, -1, "dbind no toca el destino al rechazar la cadena vacia");

    /* El mismo criterio en un tipo sin signo. */
    {
        const DBind *ubind = dbind_from_typename("uint32_t", NULL);
        uint32_t u = 0;
        if (ntest_true(ubind != NULL, "el tipo uint32_t esta registrado en dbind"))
        {
            u = 7;
            ntest_true(dbind_set_value_str(ubind, cast(&u, byte_t), "abc") == ekBINDSET_NOT_ALLOWED, "dbind rechaza \"abc\" en un entero sin signo");
            ntest_equ_u32(u, 7, "dbind no toca el destino sin signo al rechazar \"abc\"");

            u = 7;
            ntest_true(dbind_set_value_str(ubind, cast(&u, byte_t), "0xff") != ekBINDSET_NOT_ALLOWED, "dbind acepta \"0xff\" en un entero sin signo");
            ntest_equ_u32(u, 255, "dbind convierte \"0xff\" en 255");
        }
    }
}

/*---------------------------------------------------------------------------*/

/* 'dbind_set_value_str' sobre un real: la misma puerta de entrada que el caso
   entero de arriba, pero descartaba el error de 'str_to_r64', asi que "hola"
   escribia un 0 y respondia ekBINDSET_OK. Dos campos del mismo struct, uno
   entero y otro real, respondian distinto al mismo texto. Ver NAP-030. */
static void i_dbind_str_real(void)
{
    const DBind *bind = dbind_from_typename("real32_t", NULL);
    const DBind *bind64 = dbind_from_typename("real64_t", NULL);
    real32_t v = 0;
    real64_t v64 = 0;

    if (ntest_true(bind != NULL, "el tipo real32_t esta registrado en dbind") == FALSE)
        return;

    /* Un numero correcto entra, con y sin decimales. */
    v = -1;
    ntest_true(dbind_set_value_str(bind, cast(&v, byte_t), "1.5") != ekBINDSET_NOT_ALLOWED, "dbind acepta \"1.5\" en un real");
    ntest_equ_r32(v, 1.5, "dbind convierte \"1.5\" en 1.5");

    v = -1;
    ntest_true(dbind_set_value_str(bind, cast(&v, byte_t), "-3.5") != ekBINDSET_NOT_ALLOWED, "dbind acepta \"-3.5\" en un real");
    ntest_equ_r32(v, -3.5, "dbind convierte \"-3.5\" en -3.5");

    v = -1;
    ntest_true(dbind_set_value_str(bind, cast(&v, byte_t), "42") != ekBINDSET_NOT_ALLOWED, "dbind acepta \"42\" en un real");
    ntest_equ_r32(v, 42, "dbind convierte \"42\" en 42");

    /* El texto que no es un numero se rechaza y no toca el destino. */
    v = -1;
    ntest_true(dbind_set_value_str(bind, cast(&v, byte_t), "hola") == ekBINDSET_NOT_ALLOWED, "dbind rechaza \"hola\" en un real");
    ntest_equ_r32(v, -1, "dbind no toca el destino al rechazar \"hola\"");

    v = -1;
    ntest_true(dbind_set_value_str(bind, cast(&v, byte_t), "1.5abc") == ekBINDSET_NOT_ALLOWED, "dbind rechaza \"1.5abc\" en un real");
    ntest_equ_r32(v, -1, "dbind no toca el destino al rechazar \"1.5abc\"");

    /* La cadena vacia tampoco es un 0 (NAP-023), igual que en el entero. */
    v = -1;
    ntest_true(dbind_set_value_str(bind, cast(&v, byte_t), "") == ekBINDSET_NOT_ALLOWED, "dbind rechaza la cadena vacia en un real");
    ntest_equ_r32(v, -1, "dbind no toca el destino al rechazar la cadena vacia");

    /* La conversion sigue sin entender la coma decimal: el separador es siempre
       el punto, no dependa de la localizacion (NAP-023). Quien traduce la coma
       del teclado europeo es el filtro del Edit, no esta funcion: ver
       'i_dbind_str_filter' mas abajo. NAP-031. */
    v = -1;
    ntest_true(dbind_set_value_str(bind, cast(&v, byte_t), "1,5") == ekBINDSET_NOT_ALLOWED, "dbind rechaza \"1,5\" en un real");
    ntest_equ_r32(v, -1, "dbind no toca el destino al rechazar \"1,5\"");

    /* El mismo criterio en real64_t. */
    if (ntest_true(bind64 != NULL, "el tipo real64_t esta registrado en dbind"))
    {
        v64 = -1;
        ntest_true(dbind_set_value_str(bind64, cast(&v64, byte_t), "1.5") != ekBINDSET_NOT_ALLOWED, "dbind acepta \"1.5\" en un real de 64 bits");
        ntest_true(v64 == 1.5, "dbind convierte \"1.5\" en 1.5 en un real de 64 bits");

        v64 = -1;
        ntest_true(dbind_set_value_str(bind64, cast(&v64, byte_t), "hola") == ekBINDSET_NOT_ALLOWED, "dbind rechaza \"hola\" en un real de 64 bits");
        ntest_true(v64 == -1, "dbind no toca el destino de 64 bits al rechazar \"hola\"");
    }
}

/*---------------------------------------------------------------------------*/

typedef struct _numbers_t Numbers;

struct _numbers_t
{
    real32_t real_val;
    int32_t int_val;
};

/*---------------------------------------------------------------------------*/

/* 'dbind_st_str_filter' es el filtro del Edit enlazado (gui/editimp.c ->
   gui/layout.c). Dejaba pasar la coma decimal tal cual, y la conversion que
   viene detras solo entiende el punto: los dos extremos de la misma ruta no
   estaban de acuerdo. Ahora el filtro la traduce. Ver NAP-031. */
static void i_dbind_str_filter(void)
{
    const DBind *stbind = NULL;
    uint32_t rid = 0, iid = 0;
    Numbers obj;
    char_t dest[64];

    dbind(Numbers, real32_t, real_val);
    dbind(Numbers, int32_t, int_val);

    stbind = dbind_from_typename("Numbers", NULL);
    if (ntest_true(stbind != NULL, "el struct Numbers esta registrado en dbind") == FALSE)
        return;

    rid = dbind_st_member_id(stbind, "real_val");
    iid = dbind_st_member_id(stbind, "int_val");

    /* La coma del teclado europeo llega al Edit como separador decimal y sale
       del filtro como punto, que es lo que entiende 'str_to_r64'. */
    ntest_true(dbind_st_str_filter(stbind, rid, "1,5", dest, sizeof(dest)), "el filtro se aplica en un campo real");
    ntest_equ_str(dest, "1.5", "el filtro traduce la coma decimal a punto");

    ntest_true(dbind_st_str_filter(stbind, rid, "0,25", dest, sizeof(dest)), "el filtro se aplica con parte entera cero");
    ntest_equ_str(dest, "0.25", "el filtro traduce la coma detras de un cero");

    ntest_true(dbind_st_str_filter(stbind, rid, ",5", dest, sizeof(dest)), "el filtro se aplica con la coma al principio");
    ntest_equ_str(dest, ".5", "el filtro traduce la coma inicial a punto");

    /* El punto no se toca, asi que la salida del filtro vuelve a entrar por el
       filtro sin cambiar. */
    ntest_true(dbind_st_str_filter(stbind, rid, "1.5", dest, sizeof(dest)), "el filtro se aplica con un punto");
    ntest_equ_str(dest, "1.5", "el filtro deja el punto como esta");

    /* Y lo que sale del filtro lo acepta la conversion: es el invariante que
       faltaba. */
    obj.real_val = -1;
    obj.int_val = 0;
    dbind_st_str_filter(stbind, rid, "1,5", dest, sizeof(dest));
    ntest_true(dbind_st_set_value_str(stbind, rid, cast(&obj, byte_t), dest) != ekBINDSET_NOT_ALLOWED, "dbind acepta el texto que sale del filtro");
    ntest_equ_r32(obj.real_val, 1.5f, "teclear \"1,5\" en un Edit enlazado guarda 1.5");

    /* En un campo entero no hay separador decimal que valga: la coma se cae,
       como cualquier otro caracter que no sea un digito. */
    ntest_true(dbind_st_str_filter(stbind, iid, "1,5", dest, sizeof(dest)), "el filtro se aplica en un campo entero");
    ntest_equ_str(dest, "15", "el filtro descarta la coma en un campo entero");

    dbind_unreg(Numbers);
}

/*---------------------------------------------------------------------------*/

/* 'tfilter_to_date' se inventaba una fecha con un texto que no lo era: el
   filtro trabaja en modo sobreescritura, asi que "1a/02/2020" tiene la longitud
   del patron y el error de 'str_to_u8' se descartaba, dejando el dia en 0. Ver
   NAP-029. */
static void i_tfilter_date(void)
{
    Date date;

    /* Un texto correcto se lee entero. */
    date = tfilter_to_date("01/02/2020", "dd/mm/yyyy");
    ntest_false(date_is_null(&date), "tfilter_to_date lee una fecha correcta");
    ntest_equ_u32(date.mday, 1, "tfilter_to_date lee el dia");
    ntest_equ_u32(date.month, 2, "tfilter_to_date lee el mes");
    ntest_equ_i32(date.year, 2020, "tfilter_to_date lee el ano");

    /* Una letra sobre un digito ya no da la fecha 00/02/2020. */
    date = tfilter_to_date("1a/02/2020", "dd/mm/yyyy");
    ntest_true(date_is_null(&date), "tfilter_to_date devuelve kDATE_NULL con un dia no numerico");

    date = tfilter_to_date("01/x2/2020", "dd/mm/yyyy");
    ntest_true(date_is_null(&date), "tfilter_to_date devuelve kDATE_NULL con un mes no numerico");

    date = tfilter_to_date("01/02/20a0", "dd/mm/yyyy");
    ntest_true(date_is_null(&date), "tfilter_to_date devuelve kDATE_NULL con un ano no numerico");

    /* Un texto mas corto que el patron nunca se ha leido. */
    date = tfilter_to_date("01/02", "dd/mm/yyyy");
    ntest_true(date_is_null(&date), "tfilter_to_date devuelve kDATE_NULL con un texto mas corto que el patron");

    /* Un patron sin los tres campos no puede dar una fecha: antes rellenaba el
       ano que falta con 2000. */
    date = tfilter_to_date("01/02", "dd/mm");
    ntest_true(date_is_null(&date), "tfilter_to_date devuelve kDATE_NULL con un patron sin ano");

    /* El ano de dos digitos se completa, como antes. */
    date = tfilter_to_date("01/02/85", "dd/mm/yy");
    ntest_equ_i32(date.year, 1985, "tfilter_to_date completa un ano de dos digitos alto");

    date = tfilter_to_date("01/02/20", "dd/mm/yy");
    ntest_equ_i32(date.year, 2020, "tfilter_to_date completa un ano de dos digitos bajo");

    /* Los tres campos son numeros, pero la fecha no existe: la funcion no
       valida el calendario, eso es cosa de 'date_is_valid'. */
    date = tfilter_to_date("31/02/2020", "dd/mm/yyyy");
    ntest_false(date_is_null(&date), "tfilter_to_date lee 31/02/2020, que son tres numeros");
    ntest_false(date_is_valid(&date), "31/02/2020 no pasa date_is_valid");
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
    i_dbind_str();
    i_dbind_str_real();
    i_dbind_str_filter();
    i_tfilter_date();
    i_stream();
    i_buffer();
    return ntest_end();
}
