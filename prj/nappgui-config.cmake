#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------

# Lee NAppGUIOptions.txt, el fichero de 'clave:valor' con el que el paquete
# declara con que se compilo, y publica cada opcion como NAPPGUI_<clave>.
#
# Dos decisiones, las dos por NAP-035:
#
#   - **Con prefijo.** El nombre del fichero de opciones se usaba tal cual como
#     nombre de variable, asi que find_package(nappgui) metia VERSION, COMPILER,
#     CMAKE, GENERATOR, BUILD_TYPE... en el proyecto de quien lo llama. Un
#     paquete no tiene por que reservarse esos nombres. VERSION es el caso mas
#     feo, porque es tambien la palabra clave de project(... VERSION ...) y de
#     set_target_properties(... VERSION ...).
#   - **Sin CACHE INTERNAL**, que lleva FORCE implicito: pisaba lo que el
#     consumidor tuviera con ese nombre y ademas persistia entre
#     configuraciones. Son datos de solo lectura del paquete; una variable
#     normal de directorio basta y se hereda por los subdirectorios igual.
#
# read_options() es una macro, no una funcion, precisamente para que el set()
# ocurra en el ambito de quien llamo a find_package().
macro(read_options)
    set(optsFile "${CMAKE_CURRENT_LIST_DIR}/NAppGUIOptions.txt")
    if (EXISTS "${optsFile}")
        file(STRINGS "${optsFile}" ALL_OPTIONS)
        foreach(option ${ALL_OPTIONS})
            if (option)
                string(REPLACE ":" ";" KEY_VALUE "${option}")
                list(GET KEY_VALUE 0 optName)
                list(GET KEY_VALUE 1 opt)
                set(NAPPGUI_${optName} ${opt})
                message(STATUS "* NAPPGUI_${optName}: ${opt}")
            endif()
        endforeach()
    endif()
endmacro()

#------------------------------------------------------------------------------

# Config file for use find_package(NAppGUI)
include("${CMAKE_CURRENT_LIST_DIR}/nappgui-targets.cmake")

set(NAPPGUI_IS_PACKAGE True)

# nap_target() recorre CMAKE_CONFIGURATION_TYPES para fijar los directorios de
# salida y los defines CMAKE_DEBUG / CMAKE_RELEASE, asi que con un generador de
# una sola configuracion, donde esa lista viene vacia, hay que darle un valor.
#
# Se fija como variable de directorio y no con CACHE INTERNAL ... FORCE, que era
# lo de antes: eso metia en la cache del consumidor una variable estandar de
# CMake que ahi no le corresponde, y encima le decia a un generador de una sola
# configuracion que es multiconfiguracion. La variable de directorio la heredan
# los add_subdirectory(), que es donde nap_target() se usa. Ver NAP-035.
if (NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
endif()

get_filename_component(NAPPGUI_ROOT_PATH "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
set(NAPPGUI_INCLUDE_PATH "${NAPPGUI_ROOT_PATH}/inc")
set(NAPPGUI_NRC "${NAPPGUI_ROOT_PATH}/bin/nrc${CMAKE_EXECUTABLE_SUFFIX}")
set(NAPPGUI_LIBRARIES "nappgui::osapp;nappgui::encode;nappgui::gui;nappgui::osgui;nappgui::draw2d;nappgui::geom2d;nappgui::core;nappgui::osbs;nappgui::sewer")

# Welcome and options
message(STATUS "Found NAppGUI at '${NAPPGUI_ROOT_PATH}'")
message(STATUS "NAppGUI build options")
message(STATUS "---------------------")
read_options()
message(STATUS "---------------------")

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    if (NOT NAPPGUI_DEPLOYMENT_TARGET_OSX)
        message(FATAL_ERROR "NAPPGUI_DEPLOYMENT_TARGET_OSX is not set")
    endif()

    # Aqui se calculaban OSX_SYSROOT y COCOA_LIB, dos variables mas sin prefijo
    # en el proyecto del consumidor, y para ello se llamaba a `xcrun` en cada
    # configure. Nadie las lee desde que NAP-002 sustituyo ${COCOA_LIB} por
    # '-framework Cocoa' en los requisitos de uso de los targets. NAP-035.

    # El paquete declara con que deployment target se compilo. Sin esto el
    # consumidor usa el suyo por omision y el enlazador suelta un aviso
    # "object file was built for newer macOS version than being linked" por
    # cada objeto de cada libreria: casi doscientos la primera vez. NAP-002.
    if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "${NAPPGUI_DEPLOYMENT_TARGET_OSX}")
    endif()

endif()

#------------------------------------------------------------------------------
# CRT de Windows (NAP-002)
#
# El SDK se compila con una biblioteca de ejecucion de C concreta
# (NAppCompilers.cmake fija hoy la estatica, /MT y /MTd). El consumidor usa la
# dinamica por omision, y la unica senal era un LNK4098 al final del enlace.
# El paquete declara cual uso: si el consumidor no ha elegido, se le pone la
# misma; si ha elegido otra, se le dice cual es el problema y donde esta.
#------------------------------------------------------------------------------
if (MSVC AND NAPPGUI_MSVC_RUNTIME)
    if (NAPPGUI_MSVC_RUNTIME STREQUAL "static")
        set(_nappgui_crt "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    else()
        set(_nappgui_crt "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    endif()

    if (NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "${_nappgui_crt}")
    elseif (NOT "${CMAKE_MSVC_RUNTIME_LIBRARY}" STREQUAL "${_nappgui_crt}")
        message(FATAL_ERROR
            "NAppGUI se compilo con el CRT '${NAPPGUI_MSVC_RUNTIME}' "
            "(CMAKE_MSVC_RUNTIME_LIBRARY='${_nappgui_crt}') y este proyecto pide "
            "'${CMAKE_MSVC_RUNTIME_LIBRARY}'. Mezclar los dos da LNK4098 y un "
            "binario con dos heaps. Iguala CMAKE_MSVC_RUNTIME_LIBRARY o "
            "recompila NAppGUI con el otro CRT.")
    endif()

    # CMAKE_MSVC_RUNTIME_LIBRARY solo se respeta con CMP0091 en NEW, es decir,
    # con cmake_minimum_required(VERSION 3.15) o superior en el consumidor.
    cmake_policy(GET CMP0091 _nappgui_cmp0091)
    if (NOT "${_nappgui_cmp0091}" STREQUAL "NEW")
        message(WARNING
            "CMP0091 no esta en NEW: CMAKE_MSVC_RUNTIME_LIBRARY se ignora y el "
            "CRT de este proyecto puede no coincidir con el de NAppGUI "
            "('${NAPPGUI_MSVC_RUNTIME}'). Sube cmake_minimum_required a 3.15 o "
            "posterior.")
    endif()
    unset(_nappgui_cmp0091)
    unset(_nappgui_crt)
endif()

# Target definitions
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/NAppGUITargetsDefines.txt" NAppGUIDefines)
foreach(define ${NAppGUIDefines})
    if (define)
        add_definitions("-D${define}")
    endif()
endforeach()
