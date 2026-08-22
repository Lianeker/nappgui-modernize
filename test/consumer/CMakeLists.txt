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
    # Todo lo que sigue debería venir del propio paquete; hoy hay que repetirlo,
    # y hace falta en las tres plataformas: lo midió el CI de NAP-001 (ejecución
    # 32552507804), la primera vez que este consumidor se compiló fuera de
    # Windows.
    # Criterio de aceptación de NAP-002: borrar este bloque y que la prueba
    # siga en verde.
    #
    #   a) sin INTERFACE_INCLUDE_DIRECTORIES  -> las cabeceras no se encuentran
    #   b) sin las librerías del sistema      -> símbolos sin resolver:
    #        Windows: 178 (ws2_32, gdiplus, shlwapi, comctl32, uxtheme)
    #        macOS  : Cocoa/UniformTypeIdentifiers y el runtime de Objective-C
    #        Linux  : libm y GTK3 (469 símbolos cairo/pango/gdk/gtk/g_*)
    #   c) sin el CRT declarado               -> LNK4098, el SDK fuerza /MT(d)
    #      (NAppCompilers.cmake:70-72 lo fija a "static", opción comentada)
    #   d) sin el runtime de C++              -> typeinfo/vtable sin resolver en
    #      macOS y Linux. sewer, core, geom2d y osgui tienen unidades .cpp, pero
    #      el consumidor es C puro y CMake enlaza con el compilador de C. En el
    #      árbol del SDK lo tapa el LINKER_LANGUAGE CXX que fuerza nap_target
    #      (NAppTarget.cmake:879-881); el paquete exportado no lo traslada.
    target_include_directories(${tgt} PRIVATE ${NAPPGUI_INCLUDE_PATH})
    set_property(TARGET ${tgt} PROPERTY
                 MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    if (WIN32)
        target_link_libraries(${tgt} PRIVATE ws2_32 gdiplus shlwapi comctl32 uxtheme)

    elseif (APPLE)
        # COCOA_LIB lo calcula nappgui-config.cmake (Cocoa.framework, más
        # UniformTypeIdentifiers.framework si DEPLOYMENT_TARGET_OSX >= 12), pero
        # no lo pone en ningún target. objc: _objc_retain / _objc_release, que
        # emiten los .m de osgui y osapp. c++: libc++ (typeinfo/vtable).
        target_link_libraries(${tgt} PRIVATE ${COCOA_LIB} objc c++)

    elseif (UNIX)
        # Réplica mínima de NAppTarget.cmake:832-861, que es donde el SDK añade
        # esto a sus propios targets y solo a ellos. Medido con una sonda en el
        # CI (ejecución 32553191837, ubuntu-24.04): sin GTK3 y sin stdc++ el
        # enlace falla; sin pthread/dl pasa, porque glibc >= 2.34 los fusionó
        # dentro de libc. Se dejan igual: osbs sí los usa (src/osbs/unix/
        # bthread.c, bmutex.c, dlib.c) y en una distro más vieja harían falta.
        find_package(Threads REQUIRED)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
        target_link_directories(${tgt} PRIVATE ${GTK3_LIBRARY_DIRS})
        target_link_libraries(${tgt} PRIVATE
                              ${GTK3_LIBRARIES} Threads::Threads ${CMAKE_DL_LIBS}
                              stdc++ m)
    endif()
    # -------------------------------------------------------------------------
endforeach()
