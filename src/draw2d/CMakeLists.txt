# guictx.h es la interfaz de registro de la vtable del backend
# (guictx_append_*_manager): no es API publica, solo la usan los .c de 'gui' y
# de 'osapp' dentro del propio SDK. Ver docs/api-policy.md y NAP-011.
#
# guictx.hxx si se sigue instalando, aunque tampoco sea publica: 'gui/gui.hxx'
# la incluye, y por ahi entra en la cadena de <nappgui.h>. Sin ella el
# consumidor no compila.
nap_library(draw2d "geom2d" "${NAPPGUI_SHARED}" NRC_NONE PRIVATE_HEADERS "guictx.h")
