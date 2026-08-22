# osgui es el backend nativo: la capa que habla con Win32, Cocoa y GTK3. No es
# API publica (docs/api-policy.md), asi que sus 24 cabeceras no se instalan. La
# libreria si se sigue exportando, porque osapp depende de ella y hay que
# enlazarla; lo que no se publica es su interfaz. Ver NAP-011.
nap_library(osgui "draw2d" "${NAPPGUI_SHARED}" NRC_NONE PRIVATE_HEADERS ALL)

# Avoid '#include <webkit2/webkit2.h>' C++ comments warnings
if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set_source_files_properties("gtk/osweb.c" PROPERTIES COMPILE_FLAGS "-std=gnu99")
endif()
