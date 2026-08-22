/*
 * NAppGUI Cross-platform C SDK
 * MIT Licence
 *
 * File: main.c
 *
 */

/* Suite de tests de los modulos sin GUI.
 *
 * Un solo ejecutable con un modulo por argumento, para que ctest registre uno
 * por modulo sin necesitar varios targets:
 *
 *     nappgui_test            ejecuta todos
 *     nappgui_test geom2d     ejecuta solo geom2d
 *
 * Sale con codigo distinto de 0 si algo falla, incluidas las fugas de memoria.
 */

#include "ntest.h"
#include <core/core.h>
#include <core/heap.h>
#include <sewer/blib.h>
#include <sewer/bstd.h>

typedef struct _modulo_t Modulo;

struct _modulo_t
{
    const char_t *nombre;
    uint32_t (*func)(void);
};

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    Modulo modulos[4];
    uint32_t fallos = 0;
    uint32_t ejecutados = 0;
    uint32_t i = 0;
    const char_t *pedido = NULL;

    modulos[0].nombre = "sewer";
    modulos[0].func = ntest_sewer;
    modulos[1].nombre = "core";
    modulos[1].func = ntest_core;
    modulos[2].nombre = "geom2d";
    modulos[2].func = ntest_geom2d;
    modulos[3].nombre = "encode";
    modulos[3].func = ntest_encode;

    if (argc > 1)
        pedido = (const char_t *)argv[1];

    core_start();

    for (i = 0; i < sizeof(modulos) / sizeof(modulos[0]); ++i)
    {
        if (pedido != NULL && blib_strcmp(pedido, modulos[i].nombre) != 0)
            continue;
        fallos += modulos[i].func();
        ejecutados += 1;
    }

    core_finish();

    if (ejecutados == 0)
    {
        bstd_printf("modulo desconocido: '%s'\n", pedido != NULL ? pedido : "");
        return 2;
    }

    /* El auditor de memoria del SDK ya informa al cerrar core. Aqui solo
       reportamos el veredicto de las comprobaciones. */
    if (fallos > 0)
    {
        bstd_printf("\nRESULTADO: %u comprobaciones fallan\n", fallos);
        return 1;
    }

    bstd_printf("\nRESULTADO: todo pasa (%u modulo(s))\n", ejecutados);
    return 0;
}
