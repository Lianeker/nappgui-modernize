# Proyecto consumidor mínimo: valida que el SDK instalado se puede usar desde
# fuera con find_package(). Es el contrato que ve un usuario real, y por eso
# sirve de prueba de regresión del paquete exportado.
cmake_minimum_required(VERSION 3.20)
project(ConsumerSmoke C)

find_package(nappgui REQUIRED)

# ConsumerSmoke  : app de escritorio, comprueba que el SDK enlaza y arranca.
# MacroHazards   : consola, prueba de regresión de NAP-004 (higiene de macros).
# Col2dPrecision : consola, prueba de regresión de NAP-003 (precisión en geom2d).
add_executable(ConsumerSmoke WIN32 main.c)
add_executable(MacroHazards macro_hazards.c)
add_executable(Col2dPrecision col2d_precision.c)

foreach(tgt ConsumerSmoke MacroHazards Col2dPrecision)
    target_link_libraries(${tgt} PRIVATE ${NAPPGUI_LIBRARIES})

    # --- PARCHES: defecto NAP-002 --------------------------------------------
    # Los targets exportados nappgui::* no trasladan sus requisitos de uso.
    # Todo lo que sigue debería venir del propio paquete; hoy hay que repetirlo.
    # Criterio de aceptación de NAP-002: borrar este bloque y que la prueba
    # siga en verde.
    #
    #   a) sin INTERFACE_INCLUDE_DIRECTORIES  -> las cabeceras no se encuentran
    #   b) sin las librerías del sistema      -> 178 símbolos sin resolver
    #   c) sin el CRT declarado               -> LNK4098, el SDK fuerza /MT(d)
    #      (NAppCompilers.cmake:70-72 lo fija a "static", opción comentada)
    target_include_directories(${tgt} PRIVATE ${NAPPGUI_INCLUDE_PATH})
    set_property(TARGET ${tgt} PROPERTY
                 MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    if (WIN32)
        target_link_libraries(${tgt} PRIVATE ws2_32 gdiplus shlwapi comctl32 uxtheme)
    endif()
    # -------------------------------------------------------------------------
endforeach()
