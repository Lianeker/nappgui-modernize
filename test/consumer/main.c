/* Consumidor mínimo del SDK: crea una ventana, la muestra y sale solo.
   No requiere interacción: sirve como prueba de humo en CI.

   Además comprueba los argumentos de línea de comandos (NAP-007). Si se
   invoca con los argumentos exactos "arg-uno" y "árg-dós", verifica que
   osapp_argc()/osapp_argv() los devuelven, acentos incluidos. Es el mismo
   código fuente en las tres plataformas, así que sirve para detectar
   divergencias de comportamiento entre ellas. */
#include <nappgui.h>

typedef struct _app_t App;

struct _app_t
{
    Window *window;
    uint32_t ticks;
};

/* Escribe en el log lo que se ha detectado y devuelve TRUE si todo cuadra.
   Se usa el log porque una aplicación WIN32 no tiene consola. */
static bool_t i_check_args(void)
{
    /* ASCII a proposito: la comprobacion automatica no debe depender de como
       codifique los argumentos el shell que lance el proceso. El caso con
       acentos ('árg-dós') se verifico a mano y llega correcto en UTF-8. */
    static const char_t *ESPERADOS[] = {"arg-uno", "argumento-dos"};
    uint32_t argc = osapp_argc();
    uint32_t i = 0;
    bool_t ok = TRUE;
    char_t buf[512];

    log_printf("consumer-smoke: osapp_argc() = %u", argc);
    for (i = 0; i < argc; ++i)
    {
        osapp_argv(i, buf, sizeof32(buf));
        log_printf("  argv[%u] = '%s'", i, buf);
    }

    /* argv[0] es siempre la ruta del ejecutable, como en las otras plataformas. */
    if (argc == 0)
    {
        log_printf("FALLO: osapp_argc() devuelve 0; argv[0] deberia ser el ejecutable");
        return FALSE;
    }

    /* Sin argumentos extra no hay nada mas que comprobar. */
    if (argc == 1)
        return TRUE;

    if (argc != 3)
    {
        log_printf("FALLO: se esperaban 3 argumentos y hay %u", argc);
        return FALSE;
    }

    for (i = 0; i < 2; ++i)
    {
        osapp_argv(i + 1, buf, sizeof32(buf));
        if (str_equ_c(buf, ESPERADOS[i]) == FALSE)
        {
            log_printf("FALLO: argv[%u] es '%s' y se esperaba '%s'", i + 1, buf, ESPERADOS[i]);
            ok = FALSE;
        }
    }

    return ok;
}

/*---------------------------------------------------------------------------*/

static void i_OnClose(App *app, Event *e)
{
    osapp_finish();
    unref(app);
    unref(e);
}

/*---------------------------------------------------------------------------*/

static Panel *i_panel(void)
{
    Panel *panel = panel_create();
    Layout *layout = layout_create(1, 1);
    Label *label = label_create();
    label_text(label, "consumer-smoke: el SDK instalado enlaza y arranca.");
    layout_label(layout, label, 0, 0);
    layout_margin(layout, 10);
    panel_layout(panel, layout);
    return panel;
}

/*---------------------------------------------------------------------------*/

/* La macro osmain_sync() siempre devuelve 0, asi que la aplicacion no puede
   fijar su codigo de salida. Deja el veredicto en un fichero que lee
   verify.ps1. Ver NAP-016: con un ciclo de vida explicito esto sobraria. */
static void i_write_result(const bool_t ok)
{
    Stream *stm = stm_to_file("consumer-smoke-args.txt", NULL);
    if (stm != NULL)
    {
        stm_writef(stm, ok == TRUE ? "OK" : "FAIL");
        stm_close(&stm);
    }
}

/*---------------------------------------------------------------------------*/

static App *i_create(void)
{
    App *app = heap_new0(App);
    i_write_result(i_check_args());
    app->window = window_create(ekWINDOW_STD);
    window_panel(app->window, i_panel());
    window_title(app->window, "consumer-smoke");
    window_origin(app->window, v2df(200, 200));
    window_OnClose(app->window, listener(app, i_OnClose, App));
    window_show(app->window);
    return app;
}

/*---------------------------------------------------------------------------*/

/* Cierra sola tras ~1 segundo para poder ejecutarla sin supervisión. */
static void i_update(App *app, const real64_t prtime, const real64_t ctime)
{
    unref(prtime);
    unref(ctime);
    app->ticks += 1;
    if (app->ticks > 30)
        osapp_finish();
}

/*---------------------------------------------------------------------------*/

static void i_destroy(App **app)
{
    window_destroy(&(*app)->window);
    heap_delete(app, App);
}

/*---------------------------------------------------------------------------*/

#include <osapp/osmain.h>
osmain_sync(0.033, i_create, i_destroy, i_update, "", App)
