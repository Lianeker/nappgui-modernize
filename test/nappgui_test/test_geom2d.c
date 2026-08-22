/*
 * NAppGUI Cross-platform C SDK
 * MIT Licence
 *
 * File: test_geom2d.c
 *
 */

/* Tests del modulo geom2d: vectores, transformadas y colisiones. */

#include "ntest.h"
#include <geom2d/v2d.h>
#include <geom2d/s2d.h>
#include <geom2d/r2d.h>
#include <geom2d/t2d.h>
#include <geom2d/seg2d.h>
#include <geom2d/cir2d.h>
#include <geom2d/box2d.h>
#include <geom2d/tri2d.h>
#include <geom2d/obb2d.h>
#include <geom2d/col2d.h>
#include <sewer/bmath.h>
#include <sewer/bmem.h>

/*---------------------------------------------------------------------------*/

static void i_v2d(void)
{
    V2Df a = v2df(3, 4);
    V2Df b = v2df(1, 2);
    V2Df r;

    ntest_equ_r32(v2d_lengthf(&a), 5.f, "longitud del triangulo 3-4-5");
    ntest_equ_r32(v2d_sqlengthf(&a), 25.f, "longitud al cuadrado sin raiz");

    r = v2d_addf(&a, &b);
    ntest_equ_r32(r.x, 4.f, "suma en x");
    ntest_equ_r32(r.y, 6.f, "suma en y");

    r = v2d_subf(&a, &b);
    ntest_equ_r32(r.x, 2.f, "resta en x");
    ntest_equ_r32(r.y, 2.f, "resta en y");

    r = v2d_mulf(&a, 2.f);
    ntest_equ_r32(r.x, 6.f, "producto por escalar en x");

    ntest_equ_r32(v2d_dotf(&a, &b), 11.f, "producto escalar 3*1 + 4*2");

    /* Dos vectores perpendiculares tienen producto escalar cero. */
    {
        V2Df u = v2df(1, 0);
        V2Df v = v2df(0, 1);
        ntest_equ_r32(v2d_dotf(&u, &v), 0.f, "perpendiculares: producto escalar 0");
    }

    /* Normalizar deja longitud 1 y dice si ha podido. */
    r = a;
    ntest_true(v2d_normf(&r), "normf tiene exito con un vector no nulo");
    ntest_equ_r32(v2d_lengthf(&r), 1.f, "tras normalizar la longitud es 1");

    /* Normalizar el vector cero no debe explotar: debe decir que no puede. */
    {
        V2Df z = v2df(0, 0);
        ntest_false(v2d_normf(&z), "normf falla con el vector cero, sin dividir por 0");
    }

    ntest_equ_r32(v2d_distf(&a, &b), bmath_sqrtf(8.f), "distancia entre dos puntos");
    ntest_equ_r32(v2d_sqdistf(&a, &b), 8.f, "distancia al cuadrado");

    /* Rotar 90 grados: (1,0) -> (0,1) */
    {
        V2Df u = v2df(1, 0);
        v2d_rotatef(&u, kBMATH_PIf / 2.f);
        ntest_near_r32(u.x, 0.f, 0.001f, "rotar 90 grados: x");
        ntest_near_r32(u.y, 1.f, 0.001f, "rotar 90 grados: y");
    }

    /* Rotar 360 grados es la identidad. */
    {
        V2Df u = v2df(3, 4);
        v2d_rotatef(&u, 2.f * kBMATH_PIf);
        ntest_near_r32(u.x, 3.f, 0.001f, "rotar 360 grados: x");
        ntest_near_r32(u.y, 4.f, 0.001f, "rotar 360 grados: y");
    }

    /* Punto medio. */
    r = v2d_midf(&a, &b);
    ntest_equ_r32(r.x, 2.f, "punto medio en x");
    ntest_equ_r32(r.y, 3.f, "punto medio en y");

    /* Constantes. */
    ntest_equ_r32(kV2D_ZEROf.x, 0.f, "kV2D_ZEROf es el origen");
    ntest_equ_r32(kV2D_Xf.x, 1.f, "kV2D_Xf es el unitario en x");
    ntest_equ_r32(kV2D_Yf.y, 1.f, "kV2D_Yf es el unitario en y");
}

/*---------------------------------------------------------------------------*/

static void i_t2d(void)
{
    T2Df t;
    V2Df p;

    /* La identidad no mueve nada. */
    t2d_movef(&t, kT2D_IDENTf, 0, 0);
    p = v2df(5, 7);
    t2d_vmultf(&p, &t, &p);
    ntest_equ_r32(p.x, 5.f, "la identidad no mueve x");
    ntest_equ_r32(p.y, 7.f, "la identidad no mueve y");

    /* Traslacion. */
    t2d_movef(&t, kT2D_IDENTf, 10, 20);
    p = v2df(1, 2);
    t2d_vmultf(&p, &t, &p);
    ntest_equ_r32(p.x, 11.f, "traslacion en x");
    ntest_equ_r32(p.y, 22.f, "traslacion en y");

    /* Escalado. */
    t2d_scalef(&t, kT2D_IDENTf, 2, 3);
    p = v2df(4, 5);
    t2d_vmultf(&p, &t, &p);
    ntest_equ_r32(p.x, 8.f, "escalado en x");
    ntest_equ_r32(p.y, 15.f, "escalado en y");

    /* Una transformada por su inversa es la identidad. */
    {
        T2Df a, inv, comp;
        t2d_movef(&a, kT2D_IDENTf, 10, 20);
        t2d_rotatef(&a, &a, kBMATH_PIf / 3.f);
        t2d_scalef(&a, &a, 2, 2);
        t2d_inversef(&inv, &a);
        t2d_multf(&comp, &a, &inv);

        p = v2df(7, -3);
        t2d_vmultf(&p, &comp, &p);
        ntest_near_r32(p.x, 7.f, 0.01f, "t * inv(t) deja x igual");
        ntest_near_r32(p.y, -3.f, 0.01f, "t * inv(t) deja y igual");
    }

    /* Rotar 90 grados alrededor del origen. */
    {
        t2d_rotatef(&t, kT2D_IDENTf, kBMATH_PIf / 2.f);
        p = v2df(1, 0);
        t2d_vmultf(&p, &t, &p);
        ntest_near_r32(p.x, 0.f, 0.001f, "rotacion de la transformada: x");
        ntest_near_r32(p.y, 1.f, 0.001f, "rotacion de la transformada: y");
    }
}

/*---------------------------------------------------------------------------*/

static void i_r2d_box2d(void)
{
    R2Df r = r2df(10, 20, 100, 50);
    V2Df c;

    ntest_equ_r32(r.pos.x, 10.f, "R2D guarda la posicion");
    ntest_equ_r32(r.size.width, 100.f, "R2D guarda el tamano");

    c = r2d_centerf(&r);
    ntest_equ_r32(c.x, 60.f, "centro del rectangulo en x");
    ntest_equ_r32(c.y, 45.f, "centro del rectangulo en y");

    ntest_true(r2d_containsf(&r, 50, 40), "un punto de dentro esta contenido");
    ntest_false(r2d_containsf(&r, 5, 40), "un punto de fuera no esta contenido");

    {
        R2Df r2 = r2df(50, 30, 100, 50);
        R2Df r3 = r2df(500, 500, 10, 10);
        ntest_true(r2d_collidef(&r, &r2), "dos rectangulos que se solapan colisionan");
        ntest_false(r2d_collidef(&r, &r3), "dos rectangulos lejanos no colisionan");
    }

    /* Box2D a partir de puntos. */
    {
        V2Df pts[4];
        Box2Df box;
        pts[0] = v2df(1, 5);
        pts[1] = v2df(-3, 2);
        pts[2] = v2df(7, -1);
        pts[3] = v2df(0, 0);
        box = box2d_from_pointsf(pts, 4);
        ntest_equ_r32(box.min.x, -3.f, "box min en x");
        ntest_equ_r32(box.min.y, -1.f, "box min en y");
        ntest_equ_r32(box.max.x, 7.f, "box max en x");
        ntest_equ_r32(box.max.y, 5.f, "box max en y");
        ntest_equ_r32(box2d_areaf(&box), 60.f, "area de la caja envolvente");
    }
}

/*---------------------------------------------------------------------------*/

static void i_colisiones(void)
{
    /* Dos circulos que se tocan justo. */
    {
        Cir2Df c1, c2, c3;
        c1.c = v2df(0, 0);
        c1.r = 5;
        c2.c = v2df(8, 0);
        c2.r = 5;
        c3.c = v2df(20, 0);
        c3.r = 5;
        ntest_true(col2d_circle_circlef(&c1, &c2, NULL), "circulos que se solapan");
        ntest_false(col2d_circle_circlef(&c1, &c3, NULL), "circulos separados");

        /* Con punto de contacto: la profundidad debe ser 10 - 8 = 2.
           NAP-019 completo la 'd', que antes no se escribia nunca. */
        {
            Col2Df col;
            bmem_set_zero((byte_t *)&col, sizeof32(Col2Df));
            if (ntest_true(col2d_circle_circlef(&c1, &c2, &col), "colision con contacto"))
            {
                ntest_equ_r32(col.n.x, 1.f, "la normal del contacto apunta de c1 a c2");
                ntest_equ_r32(col.p.x, 5.f, "el punto de contacto esta en el borde de c1");
                ntest_equ_r32(col.d, 2.f, "profundidad de penetracion entre circulos");
            }
        }

        /* Circulos concentricos: la penetracion es la suma de los radios. */
        {
            Col2Df col;
            Cir2Df c4;
            c4.c = v2df(0, 0);
            c4.r = 3;
            bmem_set_zero((byte_t *)&col, sizeof32(Col2Df));
            if (ntest_true(col2d_circle_circlef(&c1, &c4, &col), "circulos concentricos colisionan"))
                ntest_equ_r32(col.d, 8.f, "circulos concentricos: la penetracion es r1 + r2");
        }

        /* Sin colision el Col2D no se toca. */
        {
            Col2Df col;
            bmem_set_zero((byte_t *)&col, sizeof32(Col2Df));
            ntest_false(col2d_circle_circlef(&c1, &c3, &col), "circulos separados, con Col2D");
            ntest_equ_r32(col.d, 0.f, "sin colision, col2d_circle_circlef no toca el Col2D");
        }

        /* La variante d rellena la profundidad igual que la f. */
        {
            Col2Dd col;
            Cir2Dd d1, d2;
            d1.c = v2dd(0, 0);
            d1.r = 5;
            d2.c = v2dd(8, 0);
            d2.r = 5;
            bmem_set_zero((byte_t *)&col, sizeof32(Col2Dd));
            if (ntest_true(col2d_circle_circled(&d1, &d2, &col), "colision con contacto en doble precision"))
                ntest_equ_r32(col.d, 2.f, "col2d_circle_circled tambien rellena la profundidad");
        }
    }

    /* Circulo contra punto: NAP-019 completo tambien aqui la profundidad. */
    {
        Col2Df col;
        Cir2Df cir;
        V2Df dentro = v2df(2, 0);
        V2Df fuera = v2df(20, 0);
        cir.c = v2df(0, 0);
        cir.r = 5;

        bmem_set_zero((byte_t *)&col, sizeof32(Col2Df));
        if (ntest_true(col2d_circle_pointf(&cir, &dentro, &col), "punto dentro del circulo"))
        {
            ntest_equ_r32(col.n.x, 1.f, "la normal apunta del centro al punto");
            ntest_equ_r32(col.p.x, 5.f, "el punto de contacto esta en el borde del circulo");
            ntest_equ_r32(col.d, 3.f, "profundidad: radio 5 menos distancia 2");
        }

        bmem_set_zero((byte_t *)&col, sizeof32(Col2Df));
        ntest_false(col2d_circle_pointf(&cir, &fuera, &col), "punto fuera del circulo");
        ntest_equ_r32(col.d, 0.f, "sin colision, col2d_circle_pointf no toca el Col2D");
    }

    /* Punto dentro y fuera de una caja. */
    {
        Box2Df box;
        V2Df dentro = v2df(5, 5);
        V2Df fuera = v2df(50, 5);
        box.min = v2df(0, 0);
        box.max = v2df(10, 10);
        ntest_true(col2d_box_pointf(&box, &dentro, NULL), "punto dentro de la caja");
        ntest_false(col2d_box_pointf(&box, &fuera, NULL), "punto fuera de la caja");
    }

    /* Regresion de NAP-024: i_box_point escribia en 'd' la distancia al borde
       max.y en vez de la minima de los cuatro bordes.

       La caja tiene que ser rectangular y el punto descentrado, o el fallo no
       se ve: con una caja cuadrada y el punto en el centro los cuatro bordes
       estan a la misma distancia y el valor malo coincide con el bueno.

       Caja (0,0)-(100,10) y punto (50,2): min.x esta a 50, max.x a 50,
       min.y a 2 y max.y a 8. El minimo es 2 (borde min.y); antes salia 8. */
    {
        Col2Df col;
        Box2Df box;
        V2Df pnt = v2df(50, 2);
        box.min = v2df(0, 0);
        box.max = v2df(100, 10);
        bmem_set_zero((byte_t *)&col, sizeof32(Col2Df));
        if (ntest_true(col2d_box_pointf(&box, &pnt, &col), "punto dentro de la caja rectangular"))
        {
            ntest_equ_r32(col.d, 2.f, "col2d_box_pointf: 'd' es la distancia al borde mas cercano (min.y)");

            /* 'd' es distancia a un borde, no una profundidad, y p y n siguen
               sin tocarse: box_point es del grupo 2 de docs/col2d-contacto.md.
               NAP-024 corrige el valor, no la semantica; eso es NAP-025. */
            ntest_equ_r32(col.p.x, 0.f, "col2d_box_pointf no escribe el punto de contacto");
            ntest_equ_r32(col.n.y, 0.f, "col2d_box_pointf no escribe la normal");
        }
    }

    /* El caso de la ficha NAP-024: caja (0,0)-(10,10) y punto (5,1). El borde
       mas cercano es min.y, a 1; el valor viejo era 9 (borde max.y). */
    {
        Col2Df col;
        Box2Df box;
        V2Df pnt = v2df(5, 1);
        box.min = v2df(0, 0);
        box.max = v2df(10, 10);
        bmem_set_zero((byte_t *)&col, sizeof32(Col2Df));
        if (ntest_true(col2d_box_pointf(&box, &pnt, &col), "punto descentrado dentro de la caja"))
            ntest_equ_r32(col.d, 1.f, "col2d_box_pointf: distancia 1 al borde min.y, no 9 al max.y");
    }

    /* La variante d recorre el mismo codigo (misma plantilla, otro real). */
    {
        Col2Dd col;
        Box2Dd box;
        V2Dd pnt = v2dd(50, 2);
        box.min = v2dd(0, 0);
        box.max = v2dd(100, 10);
        bmem_set_zero((byte_t *)&col, sizeof32(Col2Dd));
        if (ntest_true(col2d_box_pointd(&box, &pnt, &col), "punto dentro de la caja, doble precision"))
            ntest_equ_r32(col.d, 2.0, "col2d_box_pointd: mismo minimo que la variante f");
    }

    /* obb_point delega en box_point tras llevar el punto a coordenadas locales
       del OBB, asi que hereda el arreglo. Sin rotar, el OBB de centro (50,5) y
       de 100x10 es la caja (0,0)-(100,10) de arriba. */
    {
        Col2Df col;
        V2Df centro = v2df(50, 5);
        V2Df pnt = v2df(50, 2);
        OBB2Df *obb = obb2d_createf(&centro, 100, 10, 0);
        bmem_set_zero((byte_t *)&col, sizeof32(Col2Df));
        if (ntest_true(col2d_obb_pointf(obb, &pnt, &col), "punto dentro del OBB sin rotar"))
            ntest_equ_r32(col.d, 2.f, "col2d_obb_pointf: hereda la 'd' corregida de box_point");
        obb2d_destroyf(&obb);
    }

    /* Segmento que cruza un circulo y otro que no. */
    {
        Cir2Df cir;
        Seg2Df cruza, lejos;
        cir.c = v2df(0, 0);
        cir.r = 5;
        cruza.p0 = v2df(-10, 0);
        cruza.p1 = v2df(10, 0);
        lejos.p0 = v2df(-10, 50);
        lejos.p1 = v2df(10, 50);
        ntest_true(col2d_circle_segmentf(&cir, &cruza, NULL), "segmento que cruza el circulo");
        ntest_false(col2d_circle_segmentf(&cir, &lejos, NULL), "segmento que no lo toca");
    }

    /* Triangulo contra caja: solapado y separado.
       Nota: estas funciones van por SAT y no rellenan el Col2D, asi que desde
       NAP-019 exigen NULL y avisan con cassert si les pasas un puntero.
       La tabla completa esta en docs/col2d-contacto.md. */
    {
        Tri2Df tri = tri2df(0, 0, 10, 0, 5, 10);
        Box2Df dentro, fuera;
        dentro.min = v2df(3, 3);
        dentro.max = v2df(7, 7);
        fuera.min = v2df(50, 50);
        fuera.max = v2df(60, 60);
        ntest_true(col2d_tri_boxf(&tri, &dentro, NULL), "triangulo y caja que se solapan");
        ntest_false(col2d_tri_boxf(&tri, &fuera, NULL), "triangulo y caja separados");
    }

    /* Las variantes f y d deben coincidir en el veredicto. Es la regresion
       de NAP-003: las firmas d declaraban tipos f y corrompian la pila. */
    {
        Tri2Dd trid = tri2dd(0, 0, 10, 0, 5, 10);
        Tri2Df trif = tri2df(0, 0, 10, 0, 5, 10);
        Box2Dd boxd;
        Box2Df boxf;
        boxd.min = v2dd(3, 3);
        boxd.max = v2dd(7, 7);
        boxf.min = v2df(3, 3);
        boxf.max = v2df(7, 7);
        ntest_true(col2d_tri_boxd(&trid, &boxd, NULL) == col2d_tri_boxf(&trif, &boxf, NULL),
                   "col2d_tri_box: las variantes f y d coinciden");
    }

    /* Area y orientacion de un triangulo. */
    {
        Tri2Df tri = tri2df(0, 0, 4, 0, 0, 3);
        ntest_equ_r32(bmath_absf(tri2d_areaf(&tri)), 6.f, "area del triangulo 4x3/2");
    }
}

/*---------------------------------------------------------------------------*/

uint32_t ntest_geom2d(void)
{
    ntest_begin("geom2d");
    i_v2d();
    i_t2d();
    i_r2d_box2d();
    i_colisiones();
    return ntest_end();
}
