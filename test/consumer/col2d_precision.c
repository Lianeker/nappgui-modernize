/* Prueba de regresion de NAP-003: precision en las funciones de colision.
 *
 * Tres funciones `d` declaraban tipos `f` en su firma mientras la
 * implementacion instanciaba la plantilla con real64_t y casteaba los
 * punteros. Efecto real: se escribian 40 bytes (Col2Dd) en un buffer de 20
 * (Col2Df) y se leian 32 donde el llamante habia puesto 16.
 *
 * Esta prueba pone centinelas alrededor de las estructuras y comprueba que
 * nadie escribe fuera. Si alguien revierte las firmas, los centinelas mueren.
 */

#include <nappgui.h>

static uint32_t i_fallos = 0;

/*---------------------------------------------------------------------------*/

static void i_check(const bool_t ok, const char_t *que)
{
    if (ok == FALSE)
    {
        bstd_printf("FALLO: %s\n", que);
        i_fallos += 1;
    }
}

/*---------------------------------------------------------------------------*/

/* Col2Dd rodeado de centinelas: si la funcion escribe de mas, se ven. */
typedef struct _guarded_t Guarded;

struct _guarded_t
{
    uint64_t antes;
    Col2Dd col;
    uint64_t despues[4];
};

/*---------------------------------------------------------------------------*/

static void i_reset(Guarded *g)
{
    uint32_t i = 0;
    bmem_set_zero(cast(g, byte_t), sizeof32(Guarded));
    g->antes = 0xDEADBEEFCAFEBABEu;
    for (i = 0; i < 4; ++i)
        g->despues[i] = 0xDEADBEEFCAFEBABEu;
}

/*---------------------------------------------------------------------------*/

static bool_t i_intactos(const Guarded *g)
{
    uint32_t i = 0;
    if (g->antes != 0xDEADBEEFCAFEBABEu)
        return FALSE;
    for (i = 0; i < 4; ++i)
    {
        if (g->despues[i] != 0xDEADBEEFCAFEBABEu)
            return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*/

static void i_box_segment(void)
{
    Guarded g;
    Box2Dd box;
    Seg2Dd seg;

    i_reset(&g);
    box.min = v2dd(0, 0);
    box.max = v2dd(10, 10);
    seg.p0 = v2dd(-5, 5);
    seg.p1 = v2dd(5, 5);

    col2d_box_segmentd(&box, &seg, &g.col);
    i_check(i_intactos(&g), "col2d_box_segmentd no escribe fuera del Col2Dd");
}

/*---------------------------------------------------------------------------*/

static void i_tri_circle(void)
{
    Guarded g;
    Tri2Dd tri;
    Cir2Dd cir;

    i_reset(&g);
    tri = tri2dd(0, 0, 10, 0, 5, 10);
    cir.c = v2dd(5, 5);
    cir.r = 2;

    col2d_tri_circled(&tri, &cir, &g.col);
    i_check(i_intactos(&g), "col2d_tri_circled no escribe fuera del Col2Dd");
}

/*---------------------------------------------------------------------------*/

static void i_tri_box(void)
{
    Guarded g;
    Tri2Dd tri;
    Box2Dd box;

    i_reset(&g);
    tri = tri2dd(0, 0, 10, 0, 5, 10);
    box.min = v2dd(3, 3);
    box.max = v2dd(7, 7);

    col2d_tri_boxd(&tri, &box, &g.col);
    i_check(i_intactos(&g), "col2d_tri_boxd no escribe fuera del Col2Dd");
}

/*---------------------------------------------------------------------------*/

/* Las variantes f y d deben coincidir en si hay colision o no.
 *
 * Nota: NO se compara el contenido de Col2D. Las funciones basadas en SAT
 * (tri_box, tri_obb, poly_box...) hacen `unref(col)` en i_sat_box
 * (src/geom2d/col2d.cpp:780) y nunca rellenan el punto de contacto: solo
 * devuelven si hay solapamiento. Las demos lo pasan como NULL. Ver NAP-019. */
static void i_f_y_d_coinciden(void)
{
    Tri2Dd trid;
    Box2Dd boxd;
    Tri2Df trif;
    Box2Df boxf;
    bool_t hitd, hitf;

    trid = tri2dd(0, 0, 10, 0, 5, 10);
    boxd.min = v2dd(3, 3);
    boxd.max = v2dd(7, 7);
    trif = tri2df(0, 0, 10, 0, 5, 10);
    boxf.min = v2df(3, 3);
    boxf.max = v2df(7, 7);

    hitd = col2d_tri_boxd(&trid, &boxd, NULL);
    hitf = col2d_tri_boxf(&trif, &boxf, NULL);
    i_check(hitd == TRUE, "col2d_tri_boxd detecta el solapamiento");
    i_check(hitd == hitf, "las variantes f y d coinciden en si hay colision");

    /* Y sin solapamiento, ambas deben decir que no. */
    boxd.min = v2dd(50, 50);
    boxd.max = v2dd(60, 60);
    boxf.min = v2df(50, 50);
    boxf.max = v2df(60, 60);
    hitd = col2d_tri_boxd(&trid, &boxd, NULL);
    hitf = col2d_tri_boxf(&trif, &boxf, NULL);
    i_check(hitd == FALSE, "col2d_tri_boxd no inventa colisiones");
    i_check(hitd == hitf, "las variantes f y d coinciden tambien sin colision");
}

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    unref(argc);
    unref(argv);

    core_start();

    i_box_segment();
    i_tri_circle();
    i_tri_box();
    i_f_y_d_coinciden();

    core_finish();

    if (i_fallos == 0)
        bstd_printf("col2d_precision: todas las comprobaciones pasan\n");

    return (int)i_fallos;
}
