/* Modo oscuro en los controles nativos de Windows (NAP-042).
   Todo lo de aqui degrada en silencio: en un Windows sin las funciones que hacen
   falta, no se hace nada y la aplicacion se ve en claro, como hasta ahora. */

#include "osgui_win.ixx"

__EXTERN_C

/* Se llama una vez al arrancar osgui. Resuelve lo que haya y fija el modo de
   aplicacion a "permitir oscuro" (seguir al sistema, no imponer). */
void _osdark_start(void);

/* TRUE si este Windows sabe hacerlo. */
bool_t _osdark_available(void);

/* TRUE si el sistema esta AHORA en modo oscuro para aplicaciones. */
bool_t _osdark_enabled(void);

/* Aplica el tema que toque a un control recien creado. Es idempotente, asi que
   tambien sirve para reaplicarlo cuando el sistema cambia de tema. */
void _osdark_control(HWND hwnd);

/* Lo mismo para una ventana: incluye la barra de titulo, que va por DWM. */
void _osdark_window(HWND hwnd);

/* Reaplica a la ventana y a toda su descendencia, y repinta. Para cuando el
   sistema cambia de tema con la aplicacion abierta. */
void _osdark_refresh(HWND hwnd);

/* Colores del modo oscuro. SetWindowTheme no pinta el relleno: los controles lo
   sacan de los colores de sistema, que no cambian con el tema. */
COLORREF _osdark_bgcolor(void);
COLORREF _osdark_textcolor(void);
HBRUSH _osdark_bgbrush(void);

/* Fondo de un control con el raton encima y con el raton pulsando. El tema de
   TOOLBAR no tiene variante oscura, asi que quien dibuje un control plano en
   oscuro tiene que pintar estos dos estados a mano. */
COLORREF _osdark_hotcolor(void);
COLORREF _osdark_pressedcolor(void);

__END_C
