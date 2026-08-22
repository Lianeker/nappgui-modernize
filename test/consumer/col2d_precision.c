/* Prueba de regresion de NAP-003: precision en las funciones de colision.
 *
 * Tres funciones `d` declaraban tipos `f` en su firma mientras la
 * implementacion instanciaba la plantilla con real64_t y casteaba los
 * punteros. Efecto real: se escribian 40 bytes (Col2Dd) en un buffer de 20
 * (Col2Df) y se leian 32 donde el llamante habia puesto 16.
 *
 * Esta prueba pone centinelas alrededor de las estructuras y comprueba que
 * nadie escribe fuera. Si alguien revierte las firmas, los centinelas mueren.
 *
 * Ampliada en NAP-019 con el contrato del parametro 'col': que cada funcion
 * rellene lo que dice la tabla de docs/col2d-contacto.md, ni mas ni menos.
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

/* Las tres funciones que NAP-003 arreglo eran col2d_box_segmentd,
 * col2d_tri_circled y col2d_tri_boxd. Dos de ellas (box_segment y tri_box) van
 * por SAT y desde NAP-019 exigen NULL en 'col': pasarles un puntero es un error
 * del llamante y lo avisa un cassert. Se comprueban sin Col2D.
 *
 * Los centinelas se mantienen sobre col2d_tri_circled, que si escribe en el
 * Col2Dd, y se amplian a las funciones de circulo, que son las unicas que
 * rellenan el contacto entero. Ahi es donde una regresion de firma f/d se veria
 * de verdad: escribiendo 40 bytes en un hueco de 20. Ver docs/col2d-contacto.md.
 */
static void i_box_segment(void)
{
    Box2Dd box;
    Seg2Dd seg;
    Box2Df boxf;
    Seg2Df segf;

    box.min = v2dd(0, 0);
    box.max = v2dd(10, 10);
    seg.p0 = v2dd(-5, 5);
    seg.p1 = v2dd(5, 5);
    boxf.min = v2df(0, 0);
    boxf.max = v2df(10, 10);
    segf.p0 = v2df(-5, 5);
    segf.p1 = v2df(5, 5);

    i_check(col2d_box_segmentd(&box, &seg, NULL) == TRUE, "col2d_box_segmentd detecta el cruce");
    i_check(col2d_box_segmentd(&box, &seg, NULL) == col2d_box_segmentf(&boxf, &segf, NULL),
            "col2d_box_segment: las variantes f y d coinciden");
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
    Tri2Dd tri;
    Box2Dd box;

    tri = tri2dd(0, 0, 10, 0, 5, 10);
    box.min = v2dd(3, 3);
    box.max = v2dd(7, 7);

    i_check(col2d_tri_boxd(&tri, &box, NULL) == TRUE, "col2d_tri_boxd detecta el solapamiento");
}

/*---------------------------------------------------------------------------*/

/* Contacto completo (NAP-019): circulo-circulo y circulo-punto son las unicas
 * que rellenan p, n y d. Con centinelas alrededor, porque son las que mas
 * bytes escriben en el Col2Dd. */
static void i_contacto_completo(void)
{
    Guarded g;
    Cir2Dd c1, c2;
    V2Dd pt;

    c1.c = v2dd(0, 0);
    c1.r = 5;
    c2.c = v2dd(8, 0);
    c2.r = 5;

    i_reset(&g);
    i_check(col2d_circle_circled(&c1, &c2, &g.col) == TRUE, "col2d_circle_circled detecta el solapamiento");
    i_check(i_intactos(&g), "col2d_circle_circled no escribe fuera del Col2Dd");
    i_check(g.col.n.x == 1., "la normal apunta de c1 a c2");
    i_check(g.col.p.x == 5., "el punto de contacto esta en el borde de c1");
    i_check(g.col.d == 2., "la profundidad es (5 + 5) - 8");

    pt = v2dd(2, 0);
    i_reset(&g);
    i_check(col2d_circle_pointd(&c1, &pt, &g.col) == TRUE, "col2d_circle_pointd detecta el punto dentro");
    i_check(i_intactos(&g), "col2d_circle_pointd no escribe fuera del Col2Dd");
    i_check(g.col.d == 3., "la profundidad es 5 - 2");

    /* Y la variante f debe dar lo mismo. */
    {
        Col2Df colf;
        Cir2Df f1, f2;
        f1.c = v2df(0, 0);
        f1.r = 5;
        f2.c = v2df(8, 0);
        f2.r = 5;
        bmem_set_zero(cast(&colf, byte_t), sizeof32(Col2Df));
        i_check(col2d_circle_circlef(&f1, &f2, &colf) == TRUE, "col2d_circle_circlef detecta el solapamiento");
        i_check(colf.d == 2.f, "la variante f da la misma profundidad que la d");
    }
}

/*---------------------------------------------------------------------------*/

/* Las variantes f y d deben coincidir en si hay colision o no.
 *
 * Nota: NO se compara el contenido de Col2D. Las funciones basadas en SAT
 * (tri_box, tri_obb, poly_box...) descartan 'col' en i_sat_box
 * (src/geom2d/col2d.cpp:844) y nunca rellenan el punto de contacto: solo
 * devuelven si hay solapamiento. Desde NAP-019 exigen NULL y lo comprueban con
 * cassert. Las demos ya lo pasaban asi. Ver docs/col2d-contacto.md. */
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
    i_contacto_completo();
    i_f_y_d_coinciden();

    core_finish();

    if (i_fallos == 0)
        bstd_printf("col2d_precision: todas las comprobaciones pasan\n");

    return (int)i_fallos;
}
