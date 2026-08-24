/* Modo oscuro en los controles nativos de Windows (NAP-042).
 *
 * Windows sabe pintar sus controles en oscuro desde 10 1809, pero NO lo hace
 * solo: hay que pedirlo. Y las funciones que lo piden estan EXPORTADAS POR
 * ORDINAL y sin documentar, porque Microsoft las dejo para uso interno del
 * Explorer. Es lo que usan todas las aplicaciones que se ven bien en oscuro.
 *
 * Por eso aqui:
 *   - se resuelven en tiempo de ejecucion, nunca en tiempo de enlace;
 *   - si no estan, no pasa nada: se sigue en claro, sin avisos ni fallos.
 *
 * Un SDK no puede permitirse depender de que un ordinal siga ahi. Lo que si
 * puede es aprovecharlo cuando esta y degradar en silencio cuando no.
 */
#include "osdark_win.inl"
#include "osgui_win.inl"

#include <sewer/nowarn.hxx>
#include <windows.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <sewer/warn.hxx>

/* Los dos ordinales de uxtheme.dll que hacen el trabajo. Los nombres son los
   que usa la documentacion no oficial; el numero es el contrato real. */
#define ORD_ALLOW_DARK_MODE_FOR_WINDOW 133
#define ORD_SET_PREFERRED_APP_MODE 135

typedef enum
{
    APPMODE_DEFAULT = 0,
    APPMODE_ALLOW_DARK = 1,
    APPMODE_FORCE_DARK = 2,
    APPMODE_FORCE_LIGHT = 3,
    APPMODE_MAX = 4
} i_app_mode;

typedef BOOL(WINAPI *FPtr_allow_dark)(HWND, BOOL);
typedef i_app_mode(WINAPI *FPtr_set_app_mode)(i_app_mode);

static int i_INIT = 0;
static int i_AVAILABLE = 0;
static FPtr_allow_dark i_ALLOW_DARK = NULL;
static FPtr_set_app_mode i_SET_APP_MODE = NULL;

/*---------------------------------------------------------------------------*/

static void i_resolve(void)
{
    HMODULE ux;

    if (i_INIT != 0)
        return;
    i_INIT = 1;

    ux = LoadLibraryExW(L"uxtheme.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (ux == NULL)
        return;

    /* GetProcAddress devuelve FARPROC y hay que convertirlo al tipo real. El
       aviso 4191 avisa justamente de eso, y aqui es lo que se quiere hacer: la
       misma excepcion puntual que ya usa draw2d/win/osimage.cpp. */
#if defined(_MSC_VER)
#pragma warning(disable : 4191)
#endif
#if defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
    i_ALLOW_DARK = (FPtr_allow_dark)GetProcAddress(ux, MAKEINTRESOURCEA(ORD_ALLOW_DARK_MODE_FOR_WINDOW));
    i_SET_APP_MODE = (FPtr_set_app_mode)GetProcAddress(ux, MAKEINTRESOURCEA(ORD_SET_PREFERRED_APP_MODE));
#if defined(_MSC_VER)
#pragma warning(default : 4191)
#endif

    if (i_ALLOW_DARK == NULL || i_SET_APP_MODE == NULL)
        return;

    /* ALLOW, no FORCE: la aplicacion sigue al sistema en vez de imponer un
       tema. Forzar oscuro dejaria una ventana oscura en un escritorio claro. */
    i_SET_APP_MODE(APPMODE_ALLOW_DARK);
    i_AVAILABLE = 1;
}

/*---------------------------------------------------------------------------*/

void _osdark_start(void)
{
    i_resolve();
}

/*---------------------------------------------------------------------------*/

bool_t _osdark_available(void)
{
    i_resolve();
    return (i_AVAILABLE != 0) ? TRUE : FALSE;
}

/*---------------------------------------------------------------------------*/

/* El sistema esta en oscuro? Se lee del registro, que es donde vive el ajuste.
   `AppsUseLightTheme` a 0 significa aplicaciones en oscuro. */
bool_t _osdark_enabled(void)
{
    HKEY key = NULL;
    DWORD value = 1;
    DWORD size = sizeof(value);
    DWORD type = REG_DWORD;
    bool_t dark = FALSE;

    if (_osdark_available() == FALSE)
        return FALSE;

    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0, KEY_READ, &key) == ERROR_SUCCESS)
    {
        if (RegQueryValueExW(key, L"AppsUseLightTheme", NULL, &type,
                             (LPBYTE)&value, &size) == ERROR_SUCCESS)
        {
            dark = (value == 0) ? TRUE : FALSE;
        }
        RegCloseKey(key);
    }
    return dark;
}

/*---------------------------------------------------------------------------*/

/* Los colores del modo oscuro.
 *
 * Hacen falta porque `SetWindowTheme(DarkMode_*)` NO basta: pone en oscuro el
 * marco, las barras de desplazamiento y los estados del raton, pero el RELLENO
 * de los controles sale de los colores de sistema de Win32 (COLOR_3DFACE,
 * COLOR_WINDOW), y esos no cambian con el tema. Por eso una ventana con solo
 * SetWindowTheme se queda a medias: barra de titulo oscura y botonera blanca.
 *
 * Los valores son los del propio Explorer en oscuro. Se crean una vez y no se
 * liberan: son brochas de proceso que viven lo que la aplicacion. */
static HBRUSH i_BG_BRUSH = NULL;

COLORREF _osdark_bgcolor(void)
{
    return RGB(32, 32, 32);
}

COLORREF _osdark_textcolor(void)
{
    return RGB(235, 235, 235);
}

/* Los dos tonos de realce del Explorer en oscuro: apenas por encima del fondo.
   Un boton de barra tiene que quedarse invisible hasta que se le apunta. */
COLORREF _osdark_hotcolor(void)
{
    return RGB(58, 58, 58);
}

/*---------------------------------------------------------------------------*/

COLORREF _osdark_pressedcolor(void)
{
    return RGB(72, 72, 72);
}

/*---------------------------------------------------------------------------*/

HBRUSH _osdark_bgbrush(void)
{
    if (i_BG_BRUSH == NULL)
        i_BG_BRUSH = CreateSolidBrush(_osdark_bgcolor());
    return i_BG_BRUSH;
}

/*---------------------------------------------------------------------------*/

/* Aplica el tema a UNA ventana, sin mirar su descendencia.
   Cada familia de control tiene su propia clase de tema oscuro, y usar la que
   no toca no falla: simplemente no cambia nada. Se distingue por la clase en
   vez de pedirselo al llamante, para que esto siga funcionando si manana se
   anade otro control. */
static void i_theme_one(HWND hwnd, bool_t dark)
{
    wchar_t cls[64];
    cls[0] = 0;
    GetClassNameW(hwnd, cls, 64);

    if (i_ALLOW_DARK != NULL)
        i_ALLOW_DARK(hwnd, dark == TRUE ? TRUE : FALSE);

    if (lstrcmpiW(cls, L"COMBOBOX") == 0 || lstrcmpiW(cls, L"ComboBoxEx32") == 0 ||
        lstrcmpiW(cls, L"EDIT") == 0 || lstrcmpiW(cls, L"RICHEDIT50W") == 0)
    {
        /* CFD = "combo file dialog": es la unica variante que pinta en oscuro
           tambien la flecha, el borde y el desplegable. */
        SetWindowTheme(hwnd, dark ? L"DarkMode_CFD" : NULL, NULL);
    }
    else
    {
        SetWindowTheme(hwnd, dark ? L"DarkMode_Explorer" : NULL, NULL);
    }
}

/*---------------------------------------------------------------------------*/

static BOOL CALLBACK i_theme_child(HWND hwnd, LPARAM lparam)
{
    i_theme_one(hwnd, (bool_t)lparam);
    return TRUE;
}

void _osdark_control(HWND hwnd)
{
    bool_t dark;

    if (hwnd == NULL)
        return;
    dark = _osdark_enabled();
    if (_osdark_available() == FALSE)
        return;

    i_theme_one(hwnd, dark);

    /* Y a su descendencia. Hace falta de verdad: el desplegable de NAppGUI es
       un ComboBoxEx32, que crea DENTRO un COMBOBOX y un EDIT propios. Esos
       hijos no pasan por la creacion de controles del SDK, asi que si no se
       recorre aqui se quedan blancos dentro de un control ya oscuro. */
    EnumChildWindows(hwnd, i_theme_child, (LPARAM)dark);
}

/*---------------------------------------------------------------------------*/

void _osdark_window(HWND hwnd)
{
    BOOL dark;

    if (hwnd == NULL)
        return;
    dark = (_osdark_enabled() == TRUE) ? TRUE : FALSE;

    /* La barra de titulo si va por API documentada. El atributo 20 es
       DWMWA_USE_IMMERSIVE_DARK_MODE; en Windows 10 anteriores a 20H1 era el 19,
       asi que se intentan los dos y el que sobre falla sin consecuencias. */
    if (DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark)) != S_OK)
        DwmSetWindowAttribute(hwnd, 19, &dark, sizeof(dark));

    if (_osdark_available() == TRUE)
        i_ALLOW_DARK(hwnd, dark);
}

/*---------------------------------------------------------------------------*/

/* Vuelve a aplicar el tema a toda la descendencia. Es lo que hay que llamar
   cuando el sistema cambia de tema con la aplicacion abierta: los controles ya
   creados no se enteran solos. */
static BOOL CALLBACK i_refresh_child(HWND hwnd, LPARAM lparam)
{
    unref(lparam);
    i_theme_one(hwnd, _osdark_enabled());
    /* Repintado con marco: sin RDW_FRAME el borde se queda del color viejo. */
    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
    return TRUE;
}

void _osdark_refresh(HWND hwnd)
{
    if (hwnd == NULL)
        return;
    _osdark_window(hwnd);
    EnumChildWindows(hwnd, i_refresh_child, 0);
    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
}
