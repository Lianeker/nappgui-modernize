# nrc es la unica aplicacion del arbol que forma parte del SDK publicado, y por
# eso lleva EXPORT: el consumidor la necesita para compilar sus paquetes de
# recursos (NRC_EMBEDDED / NRC_PACKED). Las 20 demos no, y por eso ya no se
# exportan ni se instalan. Ver NAP-022.
#
# Ojo, hay dos usos distintos y los dos hacen falta:
#
#   - ${NAPPGUI_NRC}, una **ruta** a 'bin/nrc', es lo que usa
#     nap_resource_packs(): compila los recursos con execute_process() en el
#     configure, y ahi no valen ni los targets ni las expresiones de generador.
#   - nappgui::nrc, el target importado, es lo que necesita un consumidor que
#     quiera invocar la herramienta en su propio add_custom_command(), en el
#     build. Sin exportarla no tiene forma de nombrarla.
nap_command_app(nrc "" NRC_NONE EXPORT)
