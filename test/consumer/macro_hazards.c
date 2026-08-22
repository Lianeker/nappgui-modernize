/* Prueba de regresion de NAP-004: higiene de macros publicas.
 *
 * Cada bloque de aqui NO compilaba, o compilaba y daba un resultado erroneo,
 * antes del arreglo. Si esta prueba deja de compilar o devuelve != 0, alguien
 * ha vuelto a romper una macro.
 *
 * Se compila como aplicacion de consola y no necesita ventana ni interaccion.
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

/* (a) max_val / min_val sin parentesis exteriores.
   Antes: max_val(a,b) * 2 se expandia como ((a)>(b)) ? (a) : (b)*2 */
static void i_max_min_en_expresion(void)
{
    uint32_t a = 3, b = 7;
    uint32_t r = max_val(a, b) * 2;
    i_check(r == 14, "max_val dentro de una expresion mayor");

    r = min_val(a, b) * 2;
    i_check(r == 6, "min_val dentro de una expresion mayor");

    /* Antes tampoco compilaba en una lista de declaracion. */
    {
        uint32_t m = max_val(a, b), n = 0;
        unref(n);
        i_check(m == 7, "max_val en una lista de declaracion");
    }
}

/*---------------------------------------------------------------------------*/

/* (b) ptr_assign con un `if` desnudo: el else del llamante se enganchaba al
   `if` interno de la macro, asi que la rama else nunca se ejecutaba. */
static void i_ptr_assign_con_else(void)
{
    uint32_t destino = 0;
    uint32_t *p = &destino;
    bool_t rama_else = FALSE;

    if (p == NULL)
        ptr_assign(p, 99);
    else
        rama_else = TRUE;

    i_check(rama_else == TRUE, "ptr_assign no se traga el else del llamante");
    i_check(destino == 0, "ptr_assign no escribio en la rama equivocada");

    ptr_assign(p, 42);
    i_check(destino == 42, "ptr_assign asigna cuando el puntero no es NULL");
}

/*---------------------------------------------------------------------------*/

/* (c) pixbuf_get1 con `data` sin parentesis: `p + 1` se parseaba como
   p + (1[...]). Aqui basta con que compile y lea el bit correcto. */
static void i_pixbuf_get1_con_expresion(void)
{
    byte_t datos[2];
    const byte_t *p = datos;
    uint32_t bit;

    datos[0] = 0x00;
    datos[1] = 0x01; /* bit 0 del segundo byte */

    bit = pixbuf_get1(p + 1, 0, 0, 8);
    i_check(bit == 1, "pixbuf_get1 con una expresion como primer argumento");
}

/*---------------------------------------------------------------------------*/

/* (d) macros con bloque {}: `if (c) MACRO(...); else ...` era un error de
   sintaxis porque el `;` tras el bloque cerraba el if. */
static void i_bloques_con_else(void)
{
    uint32_t buf[4];
    bool_t rama_else = FALSE;

    if (i_fallos == 999)
        bmem_set_u32(buf, 4, 7);
    else
        rama_else = TRUE;

    i_check(rama_else == TRUE, "bmem_set_u32 admite un else detras");

    bmem_set_u32(buf, 4, 5);
    i_check(buf[0] == 5 && buf[3] == 5, "bmem_set_u32 sigue funcionando");
}

/*---------------------------------------------------------------------------*/

/* (e) cassert_no_nullf con `if` desnudo: mismo problema del else. */
static void i_cassert_no_nullf_con_else(void)
{
    bool_t rama_else = FALSE;
    FPtr_destroy f = NULL;

    if (f != NULL)
        cassert_no_nullf(f);
    else
        rama_else = TRUE;

    i_check(rama_else == TRUE, "cassert_no_nullf admite un else detras");
}

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    unref(argc);
    unref(argv);

    core_start();

    i_max_min_en_expresion();
    i_ptr_assign_con_else();
    i_pixbuf_get1_con_expresion();
    i_bloques_con_else();
    i_cassert_no_nullf_con_else();

    core_finish();

    if (i_fallos == 0)
        bstd_printf("macro_hazards: todas las comprobaciones pasan\n");

    return (int)i_fallos;
}
