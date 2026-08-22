/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osmain_win.h
 *
 */

/* Cross-platform main */

#include <sewer/nowarn.hxx>
#include <Windows.h>
#include <sewer/warn.hxx>

/* Los controles comunes version 6 (estilos visuales) requieren que el ejecutable
   declare la dependencia en su manifiesto. Sin ella, InitCommonControlsEx()
   devuelve FALSE y la aplicacion arranca con los controles anteriores a XP.

   Se emite desde aqui, y no solo desde el sistema de build, para que tambien la
   reciba quien consume el SDK con find_package() o con otro build system.
   Ver NAP-018. */
#if defined(_MSC_VER)
#if defined(_M_IX86)
#define NAPPGUI_MANIFEST_ARCH "x86"
#elif defined(_M_AMD64)
#define NAPPGUI_MANIFEST_ARCH "amd64"
#elif defined(_M_ARM64)
#define NAPPGUI_MANIFEST_ARCH "arm64"
#elif defined(_M_ARM)
#define NAPPGUI_MANIFEST_ARCH "arm"
#else
#error Arquitectura desconocida para el manifiesto de Common Controls
#endif

#pragma comment(linker, "\"/manifestdependency:type='Win32' " \
                        "name='Microsoft.Windows.Common-Controls' " \
                        "version='6.0.0.0' " \
                        "processorArchitecture='" NAPPGUI_MANIFEST_ARCH "' " \
                        "publicKeyToken='6595b64144ccf1df' " \
                        "language='*'\"")
#endif

#define osmain(func_create, func_destroy, options, type) \
    int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) \
    { \
        (void)(hPrevInstance), \
            (void)(lpCmdLine), \
            (void)(nCmdShow), \
            FUNC_CHECK_APP_CREATE(func_create, type), \
            FUNC_CHECK_DESTROY(func_destroy, type), \
            osmain_imp( \
                0, NULL, cast(hInstance, void), 0., \
                (FPtr_app_create)func_create, \
                (FPtr_app_update)NULL, \
                (FPtr_destroy)func_destroy, \
                options); \
        return 0; \
    }

#define osmain_sync(lframe, func_create, func_destroy, func_update, options, type) \
    int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) \
    { \
        (void)(hPrevInstance), \
            (void)(lpCmdLine), \
            (void)(nCmdShow), \
            FUNC_CHECK_APP_CREATE(func_create, type), \
            FUNC_CHECK_APP_UPDATE(func_update, type), \
            FUNC_CHECK_DESTROY(func_destroy, type), \
            osmain_imp( \
                0, NULL, cast(hInstance, void), lframe, \
                (FPtr_app_create)func_create, \
                (FPtr_app_update)func_update, \
                (FPtr_destroy)func_destroy, \
                options); \
        return 0; \
    }
