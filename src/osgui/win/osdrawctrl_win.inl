/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: osdrawctrl_win.inl
 *
 */

/* Drawing custom GUI controls */

#include "osgui_win.ixx"

__EXTERN_C

void _osdrawctrl_gdi_measuse(HDC hdc, const char_t *text, INT *width, INT *height);

/* `text_color` es un color_t de NAppGUI, NO un COLORREF: se consume con
   `color_get_rgb`, que trata el alfa a cero como "color indexado". Un COLORREF
   crudo lleva ese byte a cero y sale un color que no es el que se pidio.
   `kCOLOR_DEFAULT` significa "el que diga el tema". */
void _osdrawctrl_gdi_text(HDC hdc, HTHEME theme, const char_t *text, const int32_t x, const int32_t y, const halign_t align, const ellipsis_t trim, const int32_t text_width, const color_t text_color, const ctrl_state_t state);

__END_C
