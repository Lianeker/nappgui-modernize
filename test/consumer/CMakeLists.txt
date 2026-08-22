# Proyecto consumidor mínimo: valida que el SDK instalado se puede usar desde
# fuera con find_package(). Es el contrato que ve un usuario real, y por eso
# sirve de prueba de regresión del paquete exportado.
#
# Aquí no hay nada específico de NAppGUI más allá de las dos líneas idiomáticas
# de CMake: find_package() y target_link_libraries(). Las cabeceras, las
# librerías del sistema, el CRT de Windows y el runtime de C++ son requisitos de
# uso de los targets nappgui::* y llegan solos. Si algún día vuelve a hacer
# falta un parche en este fichero, es que el paquete exportado ha vuelto a
# mentir (NAP-002).
#
# Se enlaza contra `nappgui::osapp` **a secas**, no contra ${NAPPGUI_LIBRARIES}.
# Esa es la prueba de que el grafo de dependencias entre las librerías del SDK
# viaja dentro del paquete: osapp arrastra osgui y gui, esas arrastran draw2d, y
# así hasta sewer, y es CMake quien calcula el orden de enlace. Si alguna vez
# hay que volver a listarlas todas, el grafo se ha perdido (NAP-033).
cmake_minimum_required(VERSION 3.20)
project(ConsumerSmoke C)

find_package(nappgui REQUIRED)

# ConsumerSmoke  : app de escritorio, comprueba que el SDK enlaza y arranca.
# MacroHazards   : consola, prueba de regresión de NAP-004 (higiene de macros).
# Col2dPrecision : consola, prueba de regresión de NAP-003 (precisión en geom2d).
# Ogl3dLink      : consola, prueba de regresión de NAP-034 (ogl3d con OpenGL).
add_executable(ConsumerSmoke WIN32 main.c)
add_executable(MacroHazards macro_hazards.c)
add_executable(Col2dPrecision col2d_precision.c)
add_executable(Ogl3dLink ogl3d_link.c)

foreach(tgt ConsumerSmoke MacroHazards Col2dPrecision Ogl3dLink)
    target_link_libraries(${tgt} PRIVATE nappgui::osapp)
endforeach()

# ogl3d es opcional y no cuelga de osapp, así que el consumidor lo nombra. Lo
# que ya no tiene que hacer es llamar a nap_link_opengl(): OpenGL viaja dentro
# del propio target exportado (NAP-034).
target_link_libraries(Ogl3dLink PRIVATE nappgui::ogl3d)
