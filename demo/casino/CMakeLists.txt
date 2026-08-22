# casino es una libreria de apoyo de las demos Dice y Die, no del SDK: NO_EXPORT
# la deja fuera del conjunto exportado y del install. Antes se exportaba como
# nappgui::casino y publicaba sus cabeceras en 'inc/casino'. Ver NAP-022.
nap_library(casino "draw2d" "${NAPPGUI_SHARED}" NRC_NONE NO_EXPORT)
set_target_properties(casino PROPERTIES FOLDER "demo")
