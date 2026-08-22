/*
 * NAppGUI Cross-platform C SDK
 * MIT Licence
 *
 * File: test_sewer.c
 *
 */

/* Tests del modulo sewer: memoria cruda, matematicas y unicode. */

#include "ntest.h"
#include <sewer/bmem.h>
#include <sewer/bmath.h>
#include <sewer/types.h>
#include <sewer/unicode.h>

/*---------------------------------------------------------------------------*/

static void i_bmem(void)
{
    byte_t a[16];
    byte_t b[16];
    uint32_t i = 0;

    bmem_set_zero(a, 16);
    for (i = 0; i < 16; ++i)
        ntest_equ_u32(a[i], 0, "bmem_set_zero deja todo a cero");

    bmem_set1(a, 16, 0xAB);
    ntest_equ_u32(a[0], 0xAB, "bmem_set1 escribe el primer byte");
    ntest_equ_u32(a[15], 0xAB, "bmem_set1 escribe el ultimo byte");

    bmem_copy(b, a, 16);
    ntest_equ_i32(bmem_cmp(a, b, 16), 0, "bmem_copy produce una copia igual");

    b[7] = 0x00;
    ntest_true(bmem_cmp(a, b, 16) != 0, "bmem_cmp detecta una diferencia");

    /* Invertir dos veces debe devolver el original. */
    for (i = 0; i < 16; ++i)
        a[i] = (byte_t)i;
    bmem_copy(b, a, 16);
    bmem_rev(b, 16);
    ntest_equ_u32(b[0], 15, "bmem_rev pone el ultimo el primero");
    ntest_equ_u32(b[15], 0, "bmem_rev pone el primero el ultimo");
    bmem_rev(b, 16);
    ntest_equ_i32(bmem_cmp(a, b, 16), 0, "bmem_rev dos veces es la identidad");

    /* Solapamiento: bmem_move debe soportarlo, a diferencia de bmem_copy. */
    for (i = 0; i < 16; ++i)
        a[i] = (byte_t)i;
    bmem_move(a + 4, a, 8);
    ntest_equ_u32(a[4], 0, "bmem_move con solapamiento conserva el origen");
    ntest_equ_u32(a[11], 7, "bmem_move con solapamiento copia el ultimo byte");
}

/*---------------------------------------------------------------------------*/

static void i_bmath(void)
{
    ntest_equ_r32(bmath_sqrtf(9.f), 3.f, "sqrt(9) = 3");
    ntest_equ_r32(bmath_sqrtf(0.f), 0.f, "sqrt(0) = 0");
    ntest_equ_r32(bmath_absf(-3.5f), 3.5f, "abs(-3.5) = 3.5");
    ntest_equ_r32(bmath_maxf(2.f, 7.f), 7.f, "max(2, 7) = 7");
    ntest_equ_r32(bmath_minf(2.f, 7.f), 2.f, "min(2, 7) = 2");
    ntest_equ_r32(bmath_clampf(9.f, 0.f, 5.f), 5.f, "clamp por arriba");
    ntest_equ_r32(bmath_clampf(-9.f, 0.f, 5.f), 0.f, "clamp por abajo");
    ntest_equ_r32(bmath_clampf(3.f, 0.f, 5.f), 3.f, "clamp dentro del rango");

    ntest_equ_r32(bmath_cosf(0.f), 1.f, "cos(0) = 1");
    ntest_equ_r32(bmath_sinf(0.f), 0.f, "sin(0) = 0");
    ntest_near_r32(bmath_sinf(kBMATH_PIf / 2.f), 1.f, 0.001f, "sin(pi/2) = 1");
    ntest_near_r32(bmath_cosf(kBMATH_PIf), -1.f, 0.001f, "cos(pi) = -1");

    /* Identidad trigonometrica en varios angulos. */
    {
        uint32_t i = 0;
        for (i = 0; i < 8; ++i)
        {
            real32_t a = (real32_t)i * kBMATH_PIf / 4.f;
            real32_t s = bmath_sinf(a);
            real32_t c = bmath_cosf(a);
            ntest_near_r32(s * s + c * c, 1.f, 0.001f, "sin^2 + cos^2 = 1");
        }
    }

    ntest_equ_r32(bmath_roundf(2.4f), 2.f, "round(2.4) = 2");
    ntest_equ_r32(bmath_roundf(2.6f), 3.f, "round(2.6) = 3");
    ntest_equ_r32(bmath_floorf(2.9f), 2.f, "floor(2.9) = 2");
    ntest_equ_r32(bmath_ceilf(2.1f), 3.f, "ceil(2.1) = 3");
    ntest_equ_r32(bmath_floorf(-2.1f), -3.f, "floor(-2.1) = -3");
    ntest_equ_r32(bmath_ceilf(-2.9f), -2.f, "ceil(-2.9) = -2");

    ntest_equ_u32(max_u32(3, 9), 9, "max_u32");
    ntest_equ_u32(min_u32(3, 9), 3, "min_u32");

    /* max_val dentro de una expresion mayor: sin los parentesis exteriores
       esto daba un resultado incorrecto. Ver NAP-004. */
    {
        uint32_t a = 3, b = 7;
        ntest_equ_u32(max_val(a, b) * 2, 14, "max_val dentro de una expresion");
        ntest_equ_u32(min_val(a, b) * 2, 6, "min_val dentro de una expresion");
    }
}

/*---------------------------------------------------------------------------*/

static void i_unicode(void)
{
    char_t buf[64];
    uint32_t n = 0;

    /* ASCII */
    ntest_equ_u32(unicode_nbytes("hola", ekUTF8), 5, "nbytes de ASCII incluye el terminador");
    ntest_equ_u32(unicode_nchars("hola", ekUTF8), 4, "nchars de ASCII");

    /* 'aeiou' con acentos: 5 caracteres, 10 bytes en UTF-8. */
    ntest_equ_u32(unicode_nchars("\xC3\xA1\xC3\xA9\xC3\xAD\xC3\xB3\xC3\xBA", ekUTF8), 5,
                  "nchars cuenta caracteres, no bytes");
    ntest_equ_u32(unicode_nbytes("\xC3\xA1\xC3\xA9\xC3\xAD\xC3\xB3\xC3\xBA", ekUTF8), 11,
                  "nbytes cuenta bytes mas el terminador");

    /* Cadena vacia. */
    ntest_equ_u32(unicode_nchars("", ekUTF8), 0, "nchars de la cadena vacia");
    ntest_equ_u32(unicode_nbytes("", ekUTF8), 1, "nbytes de la cadena vacia es 1");

    /* Ida y vuelta UTF8 -> UTF32 -> UTF8. */
    {
        uint32_t utf32[16];
        char_t vuelta[64];
        const char_t *orig = "a\xC3\xB1o \xE2\x82\xAC";  /* "año €" */
        n = unicode_convers(orig, (char_t *)utf32, ekUTF8, ekUTF32, sizeof32(utf32));
        ntest_true(n > 0, "convers UTF8 -> UTF32 devuelve algo");
        unicode_convers((const char_t *)utf32, vuelta, ekUTF32, ekUTF8, sizeof32(vuelta));
        ntest_equ_str(vuelta, orig, "ida y vuelta UTF8 -> UTF32 -> UTF8");
    }

    /* Ida y vuelta UTF8 -> UTF16 -> UTF8, incluido un caracter fuera del BMP
       (que en UTF-16 ocupa un par suplente). */
    {
        uint16_t utf16[32];
        char_t vuelta[64];
        const char_t *orig = "\xC3\xB1 \xF0\x9F\x8E\xB2";  /* "ñ dado" */
        unicode_convers(orig, (char_t *)utf16, ekUTF8, ekUTF16, sizeof32(utf16));
        unicode_convers((const char_t *)utf16, vuelta, ekUTF16, ekUTF8, sizeof32(vuelta));
        ntest_equ_str(vuelta, orig, "ida y vuelta con un caracter fuera del BMP");
    }

    /* Recorrido caracter a caracter. */
    {
        const char_t *p = "a\xC3\xB1z";
        uint32_t c = unicode_to_u32(p, ekUTF8);
        ntest_equ_u32(c, 'a', "to_u32 del primer caracter");
        p = unicode_next(p, ekUTF8);
        ntest_equ_u32(unicode_to_u32(p, ekUTF8), 0xF1, "to_u32 de la enye");
        p = unicode_next(p, ekUTF8);
        ntest_equ_u32(unicode_to_u32(p, ekUTF8), 'z', "to_u32 del tercer caracter");
    }

    /* Clasificadores. */
    ntest_true(unicode_isalnum('a'), "'a' es alfanumerico");
    ntest_true(unicode_isalnum('7'), "'7' es alfanumerico");
    ntest_false(unicode_isalnum(' '), "el espacio no es alfanumerico");
    ntest_true(unicode_isspace(' '), "el espacio es espacio");
    ntest_equ_u32(unicode_tolower('A'), 'a', "tolower de ASCII");
    ntest_equ_u32(unicode_toupper('a'), 'A', "toupper de ASCII");

    /* Truncado: el destino no cabe, no debe desbordar. */
    n = unicode_convers("cadena larga de prueba", buf, ekUTF8, ekUTF8, 8);
    ntest_true(n <= 8, "convers respeta el tamano del destino");
}

/*---------------------------------------------------------------------------*/

uint32_t ntest_sewer(void)
{
    ntest_begin("sewer");
    i_bmem();
    i_bmath();
    i_unicode();
    return ntest_end();
}
