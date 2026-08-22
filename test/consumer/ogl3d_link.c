/* Prueba de regresion de NAP-034: nappgui::ogl3d se enlaza solo.
 *
 * Antes, `nappgui::ogl3d` se exportaba con las cabeceras y nada mas: le
 * faltaban opengl32 / libGL+EGL / -framework OpenGL, y la unica forma de
 * conseguirlos era llamar a nap_link_opengl(), una macro del sistema de build
 * de NAppGUI que el paquete ni documenta ni exporta.
 *
 * Aqui no se llama a nada de eso: el CMakeLists enlaza `nappgui::ogl3d` y ya.
 * Si esta prueba deja de enlazar, ogl3d ha vuelto a exportarse sin OpenGL.
 *
 * No se abre ningun contexto: hacerlo necesitaria una ventana y los runners de
 * CI no tienen sesion grafica. Basta con **referenciar** los simbolos para que
 * el enlazador tenga que sacar de la libreria los objetos del backend nativo,
 * que son los que usan OpenGL de verdad.
 */

#include <nappgui.h>
#include <ogl3d/ogl3d.h>

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    /* Punteros a funcion, no llamadas: obligan al enlazador a resolver los
       simbolos de ogl3d sin ejecutar nada. */
    void (*start)(void) = ogl3d_start;
    void (*finish)(void) = ogl3d_finish;
    OGLCtx *(*context)(const OGLProps *, void *, oglerr_t *) = ogl3d_context;
    void (*destroy)(OGLCtx **) = ogl3d_destroy;
    const char_t *(*err_str)(const oglerr_t) = ogl3d_err_str;

    unref(argc);
    unref(argv);

    if (start == NULL || finish == NULL || context == NULL || destroy == NULL || err_str == NULL)
        return 1;

    bstd_printf("ogl3d_link: nappgui::ogl3d enlaza sin nap_link_opengl()\n");
    return 0;
}
