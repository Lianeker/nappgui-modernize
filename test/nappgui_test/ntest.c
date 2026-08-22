/*
 * NAppGUI Cross-platform C SDK
 * MIT Licence
 *
 * File: ntest.c
 *
 */

/* Comprobaciones minimas para la suite de tests */

#include "ntest.h"
#include <sewer/bmath.h>
#include <sewer/blib.h>
#include <sewer/bstd.h>

static uint32_t i_TOTAL = 0;
static uint32_t i_FALLOS = 0;
static uint32_t i_PENDIENTES = 0;
static const char_t *i_MODULO = NULL;

/*---------------------------------------------------------------------------*/

void ntest_begin(const char_t *modulo)
{
    i_MODULO = modulo;
    i_TOTAL = 0;
    i_FALLOS = 0;
    i_PENDIENTES = 0;
}

/*---------------------------------------------------------------------------*/

uint32_t ntest_end(void)
{
    if (i_FALLOS > 0)
        bstd_printf("[FALLA] %s: %u de %u comprobaciones fallan\n", i_MODULO, i_FALLOS, i_TOTAL);
    else if (i_PENDIENTES > 0)
        bstd_printf("[ok]   %s: %u comprobaciones, %u pendientes de un defecto conocido\n", i_MODULO, i_TOTAL, i_PENDIENTES);
    else
        bstd_printf("[ok]   %s: %u comprobaciones\n", i_MODULO, i_TOTAL);
    return i_FALLOS;
}

/*---------------------------------------------------------------------------*/

/* Recorta la ruta para que el mensaje sea legible. */
static const char_t *i_short(const char_t *file)
{
    const char_t *p = file;
    const char_t *last = file;
    while (*p != '\0')
    {
        if (*p == '/' || *p == '\\')
            last = p + 1;
        p += 1;
    }
    return last;
}

/*---------------------------------------------------------------------------*/

static void i_fallo(const char_t *que, const char_t *file, const uint32_t line)
{
    i_FALLOS += 1;
    bstd_printf("  FALLO %s:%u  %s\n", i_short(file), line, que);
}

/*---------------------------------------------------------------------------*/

bool_t ntest_true_imp(const bool_t cond, const char_t *que, const char_t *file, const uint32_t line)
{
    i_TOTAL += 1;
    if (cond == FALSE)
        i_fallo(que, file, line);
    return cond;
}

/*---------------------------------------------------------------------------*/

void ntest_equ_u32_imp(const uint32_t obtenido, const uint32_t esperado, const char_t *que, const char_t *file, const uint32_t line)
{
    i_TOTAL += 1;
    if (obtenido != esperado)
    {
        i_FALLOS += 1;
        bstd_printf("  FALLO %s:%u  %s (obtenido %u, esperado %u)\n", i_short(file), line, que, obtenido, esperado);
    }
}

/*---------------------------------------------------------------------------*/

void ntest_equ_i32_imp(const int32_t obtenido, const int32_t esperado, const char_t *que, const char_t *file, const uint32_t line)
{
    i_TOTAL += 1;
    if (obtenido != esperado)
    {
        i_FALLOS += 1;
        bstd_printf("  FALLO %s:%u  %s (obtenido %d, esperado %d)\n", i_short(file), line, que, obtenido, esperado);
    }
}

/*---------------------------------------------------------------------------*/

void ntest_equ_r32_imp(const real32_t obtenido, const real32_t esperado, const real32_t tol, const char_t *que, const char_t *file, const uint32_t line)
{
    i_TOTAL += 1;
    if (bmath_absf(obtenido - esperado) > tol)
    {
        i_FALLOS += 1;
        bstd_printf("  FALLO %s:%u  %s (obtenido %.6f, esperado %.6f)\n", i_short(file), line, que, (real64_t)obtenido, (real64_t)esperado);
    }
}

/*---------------------------------------------------------------------------*/

void ntest_pending_imp(const char_t *que, const char_t *tarea, const char_t *file, const uint32_t line)
{
    i_PENDIENTES += 1;
    bstd_printf("  PENDIENTE %s:%u  %s [%s]\n", i_short(file), line, que, tarea);
}

/*---------------------------------------------------------------------------*/

void ntest_equ_str_imp(const char_t *obtenido, const char_t *esperado, const char_t *que, const char_t *file, const uint32_t line)
{
    i_TOTAL += 1;
    if (obtenido == NULL || esperado == NULL || blib_strcmp(obtenido, esperado) != 0)
    {
        i_FALLOS += 1;
        bstd_printf("  FALLO %s:%u  %s (obtenido '%s', esperado '%s')\n", i_short(file), line, que,
                    obtenido != NULL ? obtenido : "(null)",
                    esperado != NULL ? esperado : "(null)");
    }
}
