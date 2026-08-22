/*
 * NAppGUI Cross-platform C SDK
 * 2015-2026 Francisco Garcia Collado
 * MIT Licence
 * https://nappgui.com/en/legal/license.html
 *
 * File: updown.h
 * https://nappgui.com/en/gui/updown.html
 *
 */

/* Up Down */

#ifndef __GUI_UPDOWN_H__
#define __GUI_UPDOWN_H__

#include "gui.hxx"

__EXTERN_C

_gui_api UpDown *updown_create(void);

_gui_api void updown_OnClick(UpDown *updown, Listener *listener);

_gui_api void updown_tooltip(UpDown *updown, const char_t *text);

__END_C

#endif /* __GUI_UPDOWN_H__ */
