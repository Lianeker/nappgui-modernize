#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------

set(NAP_TARGET_PUBLIC_HEADER_EXTENSION "*.h;*.hxx;*.hpp;*.hdf")
set(NAP_TARGET_HEADER_EXTENSION "${NAP_TARGET_PUBLIC_HEADER_EXTENSION};*.inl;*.ixx;*.ipp")
set(NAP_TARGET_SRC_EXTENSION "${NAP_TARGET_HEADER_EXTENSION};*.c;*.cpp")
if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(NAP_TARGET_SRC_EXTENSION "${NAP_TARGET_SRC_EXTENSION};*.m")
endif()

# https://cmake.org/cmake/help/latest/policy/CMP0068.html
# RPATH settings on macOS do not affect install_name.
# CMake 3.9 and newer remove any effect the following settings may have on the
# install_name of a target on macOS
if(${CMAKE_VERSION} VERSION_GREATER "3.8.999")
    cmake_policy(SET CMP0068 NEW)
endif()

if (NOT NAPPGUI_ROOT_PATH)
    message(FATAL_ERROR "NAPPGUI_ROOT_PATH is not set.")
endif()

# Defines required by NAppGUI-based targets after installation
set(NAPPGUI_INSTALL_DEFINES "${CMAKE_BINARY_DIR}/NAppGUITargetsDefines.txt")
if(EXISTS "${NAPPGUI_INSTALL_DEFINES}")
    file(REMOVE "${NAPPGUI_INSTALL_DEFINES}")
endif()
file(WRITE "${NAPPGUI_INSTALL_DEFINES}" "")

#------------------------------------------------------------------------------

# 90, 99, 11, 17, 23
function(nap_target_c_standard targetName std)

    if (${std} STREQUAL "90")
        # Ok!

    elseif (${std} STREQUAL "99")
        # Ok!

    elseif (${std} STREQUAL "11")
        # Ok!

    # New in version 3.21.
    elseif (${std} STREQUAL "17")
        if(${CMAKE_VERSION} VERSION_GREATER "3.20.999")
            # Ok!
        else()
            set(std "11")
        endif()

    # New in version 3.21.
    elseif (${std} STREQUAL "23")
        if(${CMAKE_VERSION} VERSION_GREATER "3.20.999")
            # Ok!
        else()
            set(std "11")
        endif()

    else()
        message(FATAL_ERROR "Unknown C standard")

    endif()

    # Language standard support in CMake 3.1
    if(${CMAKE_VERSION} VERSION_GREATER "3.0.999")
        set_property(TARGET ${targetName} PROPERTY C_STANDARD ${std})
    endif()

endfunction()

#------------------------------------------------------------------------------

# 98, 11, 14, 17, 20, 23, 26
function(nap_target_cxx_standard targetName std)

    if (${std} STREQUAL "98")
        # Ok!

    elseif (${std} STREQUAL "11")
        # Ok!

    elseif (${std} STREQUAL "14")
        # Ok!

    # New in version 3.8.
    elseif (${std} STREQUAL "17")
        if(${CMAKE_VERSION} VERSION_GREATER "3.7.999")
            # Ok!
        else()
            set(std "14")
        endif()

    # New in version 3.12.
    elseif (${std} STREQUAL "20")
        if(${CMAKE_VERSION} VERSION_GREATER "3.11.999")
            # Ok!
        elseif(${CMAKE_VERSION} VERSION_GREATER "3.7.999")
            set(std "17")
        else()
            set(std "14")
        endif()

    # New in version 3.20.
    elseif(${std} STREQUAL "23")
        if(${CMAKE_VERSION} VERSION_GREATER "3.19.999")
            # Ok!
        elseif(${CMAKE_VERSION} VERSION_GREATER "3.11.999")
            set(std "20")
        elseif(${CMAKE_VERSION} VERSION_GREATER "3.7.999")
            set(std "17")
        else()
            set(std "14")
        endif()

    # New in version 3.25.
    elseif(${std} STREQUAL "26")
        if(${CMAKE_VERSION} VERSION_GREATER "3.24.999")
            # Ok!
        elseif(${CMAKE_VERSION} VERSION_GREATER "3.19.999")
            set(std "23")
        elseif(${CMAKE_VERSION} VERSION_GREATER "3.11.999")
            set(std "20")
        elseif(${CMAKE_VERSION} VERSION_GREATER "3.7.999")
            set(std "17")
        else()
            set(std "14")
        endif()

    else()
        message(FATAL_ERROR "Unknown C++ standard")

    endif()

    # Language standard support in CMake 3.1
    if(${CMAKE_VERSION} VERSION_GREATER "3.0.999")
        set_property(TARGET ${targetName} PROPERTY CXX_STANDARD ${std})
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_target_rpath targetName isMacOsBundle rpath)

    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        set(RUNPATH "\${ORIGIN}")

        foreach(path ${rpath})
            set (RUNPATH "${RUNPATH}:${path}")
        endforeach(path )

        # Will disable the CMake automatic setting of the RPATH
        # RPaths included in target will be the current directory (ORIGIN)
        # and the provided in 'rpath' parameter
        set_property(TARGET ${targetName} PROPERTY SKIP_BUILD_RPATH FALSE)
        set_property(TARGET ${targetName} PROPERTY BUILD_WITH_INSTALL_RPATH TRUE)
        set_property(TARGET ${targetName} PROPERTY INSTALL_RPATH "${RUNPATH}")

    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        # otool -L libdraw2d.dylib
        # @rpath/libgeom2d.dylib (compatibility version 0.0.0, current version 0.0.0)
        # Force to use paths relative to @rpath in dylibs and execs
        set_property(TARGET ${targetName} PROPERTY MACOSX_RPATH TRUE)

        if (isMacOsBundle)
            set(RUNPATH "@executable_path/../../..")
        else()
            set(RUNPATH "@executable_path/.")
        endif()

        set_property(TARGET ${targetName} PROPERTY SKIP_BUILD_RPATH FALSE)
        set_property(TARGET ${targetName} PROPERTY BUILD_RPATH ${RUNPATH})
        set_property(TARGET ${targetName} PROPERTY INSTALL_RPATH ${RUNPATH})

        # Delete Build RPATH manually (only if bundle have dynamic lib dependencies)
        # if (isMacOsBundle)
        #     add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL} -delete_rpath "${CMAKE_BINARY_DIR}/$<CONFIG>/bin" $<TARGET_FILE:${targetName}>)
        # endif()

    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_get_subdirectories dir _ret)
    set(dirList "")

    file(GLOB children RELATIVE ${dir} ${dir}/[a-zA-z_]*)

    foreach(child ${children})
        if(IS_DIRECTORY ${dir}/${child})
            list(APPEND dirList ${child})
        endif()
    endforeach()

    set(${_ret} ${dirList} PARENT_SCOPE)
endfunction()

#------------------------------------------------------------------------------

function(nap_is_source_subdir subDirName _ret)

    string(TOLOWER ${subDirName} subDirLower)
    if (${subDirLower} STREQUAL win)
        if (WIN32)
            set(${_ret} TRUE PARENT_SCOPE)
        else()
            set(${_ret} FALSE PARENT_SCOPE)
        endif()
    elseif (${subDirLower} STREQUAL unix)
        if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin"
            OR ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            set(${_ret} TRUE PARENT_SCOPE)
        else()
            set(${_ret} FALSE PARENT_SCOPE)
        endif()
    elseif (${subDirLower} STREQUAL osx)
        if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
            set(${_ret} TRUE PARENT_SCOPE)
        else()
            set(${_ret} FALSE PARENT_SCOPE)
        endif()
    elseif (${subDirLower} STREQUAL linux)
        if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            set(${_ret} TRUE PARENT_SCOPE)
        else()
            set(${_ret} FALSE PARENT_SCOPE)
        endif()
    elseif (${subDirLower} STREQUAL gtk)
        if (CMAKE_TOOLKIT)
            if (${CMAKE_TOOLKIT} STREQUAL "GTK3")
                set(${_ret} TRUE PARENT_SCOPE)
            else()
                set(${_ret} FALSE PARENT_SCOPE)
            endif()
        else()
            set(${_ret} FALSE PARENT_SCOPE)
        endif()
    elseif (${subDirLower} STREQUAL res)
        set(${_ret} FALSE PARENT_SCOPE)
    else ()
        set(${_ret} TRUE PARENT_SCOPE)
    endif ()

endfunction()

#------------------------------------------------------------------------------

function(nap_add_source_subdir targetName subDir)

    if ("${${targetName}_SRCSUBDIRS}" STREQUAL "")
        set(${targetName}_SRCSUBDIRS "${subDir}" CACHE INTERNAL "")
    else()
        list (FIND ${targetName}_SRCSUBDIRS ${subDir} index)
        if (${index} EQUAL -1)
            set(${targetName}_SRCSUBDIRS "${${targetName}_SRCSUBDIRS};${subDir}" CACHE INTERNAL "")
        endif ()
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_add_source_file targetName file)

    if ("${${targetName}_SRCFILES}" STREQUAL "")
        set(${targetName}_SRCFILES "${file}" CACHE INTERNAL "")
    else()
        list (FIND ${targetName}_SRCFILES ${file} index)
        if (${index} EQUAL -1)
            set(${targetName}_SRCFILES "${${targetName}_SRCFILES};${file}" CACHE INTERNAL "")
        else()
            message(FATAL_ERROR "Duplicated source file '${file}'")
        endif()
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_add_public_header targetName file extLower publicHeaders)

    if (publicHeaders)
        list (FIND NAP_TARGET_PUBLIC_HEADER_EXTENSION "*${extLower}" index)
        if (${index} GREATER -1)
            if ("${${targetName}_PUBLICHEADERS}" STREQUAL "")
                set(${targetName}_PUBLICHEADERS "${file}" CACHE INTERNAL "")
            else()
                list (FIND ${targetName}_PUBLICHEADERS ${file} index)
                if (${index} EQUAL -1)
                    set(${targetName}_PUBLICHEADERS "${${targetName}_PUBLICHEADERS};${file}" CACHE INTERNAL "")
                else()
                    message(FATAL_ERROR "Duplicated public header file '${file}'")
                endif()
            endif()
        endif()
    endif()

endfunction()

#------------------------------------------------------------------------------
# Argumentos opcionales con palabra clave de nap_library(), nap_command_app() y
# nap_desktop_app().
#
#   EXPORT / NO_EXPORT                este target forma parte, o no, del SDK
#                                     que se instala y se exporta (NAP-022)
#   PRIVATE_HEADERS <ALL|fichero...>  cabeceras que se compilan pero no se
#                                     instalan (NAP-011)
#
# 'defaultExport' es lo que vale sin decir nada: TRUE en las librerias, FALSE en
# las aplicaciones. La idea es que la decision sea explicita en la funcion que
# crea el target y no una lista de casos particulares por ruta.
#------------------------------------------------------------------------------
function(nap_target_options targetName defaultExport _export _privateHeaders)

    set(napExport ${defaultExport})
    set(napPrivateHeaders "")
    set(napKeyword "")

    foreach(napArg ${ARGN})
        if (napArg STREQUAL "EXPORT")
            set(napExport TRUE)
            set(napKeyword "")
        elseif (napArg STREQUAL "NO_EXPORT")
            set(napExport FALSE)
            set(napKeyword "")
        elseif (napArg STREQUAL "PRIVATE_HEADERS")
            set(napKeyword "PRIVATE_HEADERS")
        elseif (napKeyword STREQUAL "PRIVATE_HEADERS")
            list(APPEND napPrivateHeaders ${napArg})
        else()
            message(FATAL_ERROR "- ${targetName}: argumento desconocido '${napArg}'")
        endif()
    endforeach()

    set(${_export} ${napExport} PARENT_SCOPE)
    set(${_privateHeaders} "${napPrivateHeaders}" PARENT_SCOPE)

endfunction()

#------------------------------------------------------------------------------
# Cabeceras que se compilan pero no se instalan (NAP-011)
#
# nap_target() marca como PUBLIC_HEADER todo fichero de cabecera que hay en la
# raiz del modulo, y install(TARGETS ... PUBLIC_HEADER ...) lo publica en
# 'inc/<modulo>'. Cada cabecera instalada es una promesa de estabilidad:
# publicar el backend nativo convierte cualquier refactor interno en un cambio
# de API.
#
# docs/api-policy.md fija que es publico; el argumento PRIVATE_HEADERS de
# nap_library() es como se aplica, y esta funcion quien lo resuelve. El filtro
# tiene que ocurrir aqui, antes de install(TARGETS ...): esa orden se queda con
# la lista de cabeceras tal como esta en el momento de llamarla, asi que
# vaciar la propiedad despues no sirve de nada.
#
#   PRIVATE_HEADERS ALL          el modulo entero es interno
#   PRIVATE_HEADERS "guictx.h"   solo esas cabeceras
#
function(nap_filter_public_headers publicHeaders privateHeaders _ret)

    if (NOT privateHeaders)
        set(${_ret} "${publicHeaders}" PARENT_SCOPE)
        return()
    endif()

    if ("${privateHeaders}" STREQUAL "ALL")
        set(${_ret} "" PARENT_SCOPE)
        return()
    endif()

    set(napResult "${publicHeaders}")
    foreach(napPrivate ${privateHeaders})
        set(napFound FALSE)
        foreach(napHeader ${publicHeaders})
            get_filename_component(napName ${napHeader} NAME)
            if (napName STREQUAL napPrivate)
                list(REMOVE_ITEM napResult ${napHeader})
                set(napFound TRUE)
            endif()
        endforeach()

        # Si la cabecera se renombra o desaparece, mejor enterarse en el
        # configure que descubrir meses despues que se volvio a instalar.
        if (NOT napFound)
            message(FATAL_ERROR "PRIVATE_HEADERS: '${napPrivate}' no es una cabecera de este target")
        endif()
    endforeach()

    set(${_ret} "${napResult}" PARENT_SCOPE)

endfunction()

#------------------------------------------------------------------------------

function(nap_source_files targetName dir group publicHeaders)

    file(GLOB children RELATIVE ${dir} ${dir}/[a-zA-z_]*)

    foreach(child ${children})
        if (IS_DIRECTORY ${dir}/${child})
            nap_is_source_subdir(${child} isSource)
            if (${isSource})
                nap_source_files(${targetName} "${dir}/${child}" "${group}/${child}" FALSE)
            endif()
        else()
            get_filename_component(ext ${child} EXT)
            string(TOLOWER ${ext} extLower)
            list (FIND NAP_TARGET_SRC_EXTENSION "*${extLower}" index)

            if (${index} GREATER -1)
                string(REPLACE "/" "\\" groupname ${group})
                source_group(${groupname} FILES ${dir}/${child})

                # Force header files 'Build errors with CMake >= 3.21.2'
                # https://gitlab.kitware.com/cmake/cmake/-/merge_requests/5926
                list (FIND NAP_TARGET_HEADER_EXTENSION "*${extLower}" index)
                if (${index} GREATER -1)
                    set_source_files_properties(${dir}/${child} PROPERTIES HEADER_FILE_ONLY ON)
                    if(CMAKE_GENERATOR STREQUAL "Xcode")
                        set_source_files_properties(${dir}/${child} PROPERTIES XCODE_EXPLICIT_FILE_TYPE sourcecode.c.h)
                    endif()
                endif()

                nap_add_source_subdir(${targetName} ${dir})
                nap_add_source_file(${targetName} ${dir}/${child})
                nap_add_public_header(${targetName} ${dir}/${child} ${extLower} ${publicHeaders})

            endif()

        endif()

    endforeach()

endfunction()

#------------------------------------------------------------------------------

function(nap_resource_pattern dir _ret)
    set(list_res "")

    foreach (item ${RES_EXTENSION})
        list(APPEND list_res ${dir}/${item})
    endforeach()

    set(${_ret} ${list_res} PARENT_SCOPE)
endfunction()

#------------------------------------------------------------------------------

function(nap_resource_packs targetName targetType nrcMode dir _resFiles _resIncludeDir)
    # All resource files in package
    set(resFiles "")
    set(resPath ${dir}/res)

    if (EXISTS ${resPath})
        # Process Win32 .rc files
        if (targetType STREQUAL WIN_DESKTOP)

            # VS2005 does not support .ico with 256 res
            if(MSVC_VERSION EQUAL 1400 OR MSVC_VERSION LESS 1400)
                file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/res.rc "APPLICATION_ICON ICON \"${resPath}/logo48.ico\"\n")
                set(globalRes ${resPath}/logo48.ico)
            else()
                file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/res.rc "APPLICATION_ICON ICON \"${resPath}/logo256.ico\"\n")
                set(globalRes ${resPath}/logo256.ico)
            endif()

            set(MANIFEST_FILE "${NAPPGUI_ROOT_PATH}/prj/templates/Application.manifest")
            file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/res.rc "1 24 \"${MANIFEST_FILE}\"\n")
            set(globalRes ${globalRes} ${CMAKE_CURRENT_BINARY_DIR}/res.rc)
        endif()

        if (EXISTS ${resPath}/license.txt)
            list(APPEND globalRes ${resPath}/license.txt)
        endif()

        if (EXISTS ${resPath}/pack.txt)
            list(APPEND globalRes ${resPath}/pack.txt)
        endif()

        source_group(res FILES ${globalRes})
        list(APPEND resFiles ${globalRes})

    endif()

    # Target Resources
    if (${nrcMode} STREQUAL "NRC_EMBEDDED" OR ${nrcMode} STREQUAL "NRC_PACKED")

        if (NOT NAPPGUI_NRC)
            message(FATAL_ERROR "NAPPGUI_NRC is not set")
        endif()

        nap_get_subdirectories(${resPath} resPackDirs)

        # Resource destiny directory
        set(DEST_RESDIR ${CMAKE_CURRENT_BINARY_DIR}/resgen)
        set(CMAKE_OUTPUT ${DEST_RESDIR}/NRCLog.txt)
        if (NOT EXISTS ${DEST_RESDIR})
            file(MAKE_DIRECTORY ${DEST_RESDIR})
        endif()

        foreach(resPack ${resPackDirs})
            # Add resources to IDE
            set(resPackPath ${resPath}/${resPack})
            nap_resource_pattern(${resPackPath} resGlob)
            file(GLOB resPackPathFiles ${resGlob})
            source_group("res\\${resPack}" FILES ${resPackPathFiles})
            list(APPEND resFiles ${resPackPathFiles})

            # Add localized resources to IDE
            nap_get_subdirectories(${resPath}/${resPack} resLocalDirs)
            foreach(resLocalDir ${resLocalDirs})
                set(resLocalPath ${resPath}/${resPack}/${resLocalDir})
                nap_resource_pattern(${resLocalPath} resLocalGlob)
                file(GLOB resLocalPathFiles ${resLocalGlob})
                source_group("res\\${resPack}\\${resLocalDir}" FILES ${resLocalPathFiles})
                list(APPEND resFiles ${resLocalPathFiles})
            endforeach()

            if (${nrcMode} STREQUAL "NRC_EMBEDDED")
                set(NRC_OPTION "-dc")
            # '*.res' package will be copied in executable location
            elseif (${nrcMode} STREQUAL "NRC_PACKED")
                set(NRC_OPTION "-dp")
            else()
                message (FATAL_ERROR "Unknown nrc mode")
            endif()

            file(TO_NATIVE_PATH ${resPackPath} RESPACK_NATIVE)
            file(TO_NATIVE_PATH ${DEST_RESDIR}/${resPack}.c RESDEST_NATIVE)
            execute_process(COMMAND "${NAPPGUI_NRC}" "${NRC_OPTION}" "${RESPACK_NATIVE}" "${RESDEST_NATIVE}" RESULT_VARIABLE nrcRes OUTPUT_VARIABLE nrcOut ERROR_VARIABLE nrcErr)
            file(WRITE ${CMAKE_OUTPUT} ${nrcOut})
            file(APPEND ${CMAKE_OUTPUT} ${nrcErr})

            if (${nrcRes} EQUAL "0")
                message(STATUS "- [OK] ${resPack}: Resource pack recompiled.")
            elseif (${nrcRes} EQUAL "1")
                message(STATUS "- [OK] ${resPack}: Resource pack is up-to-date.")
            elseif (${nrcRes} EQUAL "-1")
                message("- [RES] ${resPack}: warnings (See ${CMAKE_OUTPUT})")
            else()
                message("- [RES] ${resPack}: errors (${nrcRes}) (See ${CMAKE_OUTPUT})")
                message("- [RES] OutStd: ${nrcOut}")
                message("- [RES] OutErr: ${nrcErr}")
            endif()

            list(APPEND resCompiled ${DEST_RESDIR}/${resPack}.c)
            list(APPEND resCompiled ${DEST_RESDIR}/${resPack}.h)
            source_group("res\\${resPack}\\gen" FILES ${resCompiled})

            list(APPEND resFiles ${resCompiled})

            set(${_resIncludeDir} ${DEST_RESDIR} PARENT_SCOPE)

            endforeach()

    endif()

    set(${_resFiles} ${resFiles} PARENT_SCOPE)

endfunction()

#------------------------------------------------------------------------------

function(nap_install_resource_packs targetName targetType sourceDir nrcMode doInstall)
    set (resourcePath ${sourceDir}/res)

    # Apple Bundle always have a resource dir
    if (targetType STREQUAL APPLE_BUNDLE)
        set(resourceDestPath "../resources")
        add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${targetName}>/${resourceDestPath})
        add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${targetName}>/${resourceDestPath}/en.lproj)

        # Bundle icon
        if (EXISTS ${resourcePath}/logo.icns)
            add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${resourcePath}/logo.icns $<TARGET_FILE_DIR:${targetName}>/${resourceDestPath})
        else()
            message(WARNING "logo.icns doesn't exists in '${resourcePath}'")
        endif()

    # Linux needs the app icon near the executable
    elseif (targetType STREQUAL LINUX_DESKTOP)

        if (EXISTS ${resourcePath}/logo48.ico)
            add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${resourcePath}/logo48.ico $<TARGET_FILE_DIR:${targetName}>/${targetName}.ico)
            if (doInstall)
                install(FILES $<TARGET_FILE_DIR:${targetName}>/${targetName}.ico DESTINATION "bin")
            endif()
        else()
            message(WARNING "logo48.ico doesn't exists in '${resourcePath}'")
        endif()

    endif()

    if (${nrcMode} STREQUAL "NRC_PACKED")
        set(resPath ${sourceDir}/res)
        set(destResDir ${CMAKE_CURRENT_BINARY_DIR}/resgen)

        # Create 'res' directory for packed resources
        # In the same location as executable
        if (WIN32)
            set(resRelative "res")
            add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${targetName}>/${resRelative})

        elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
            # For macOS bundles, resource dir is created in 'macOSBundle'
            set(resRelative "../resources")

        elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            set(resRelative "res")
            add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${targetName}>/${resRelative})

            else()
            message(FATAL_ERROR "Unknown system")

        endif()

        nap_get_subdirectories(${resPath} resPackDirs)

        # Copy all resource packs
        foreach(resSubDir ${resPackDirs})
            add_custom_command(TARGET ${targetName} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${destResDir}/${resSubDir}.res $<TARGET_FILE_DIR:${targetName}>/${resRelative})
            if (doInstall)
                install(FILES ${destResDir}/${resSubDir}.res DESTINATION "bin/res")
            endif()
            endforeach()

    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_target_relpath targetSrcDir _ret)
    string(REPLACE "${CMAKE_SOURCE_DIR}/" "" relPath ${targetSrcDir})
    set(${_ret} ${relPath} PARENT_SCOPE)
endfunction()

#------------------------------------------------------------------------------

function(nap_add_dependency targetName depend)

    if ("${${targetName}_LINKDEPENDS}" STREQUAL "")
        set(${targetName}_LINKDEPENDS "${depend}" CACHE INTERNAL "")
    else()
        set(${targetName}_LINKDEPENDS "${${targetName}_LINKDEPENDS};${depend}" CACHE INTERNAL "")
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_direct_dependencies targetName _ret)

    if (NAPPGUI_CACHE_DEPENDS_${targetName})
        set(${_ret} ${NAPPGUI_CACHE_DEPENDS_${targetName}} PARENT_SCOPE)
    else()
        set(${_ret} "" PARENT_SCOPE)
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_target_dependencies targetName dependList)

    foreach(depend ${dependList})

        # Dependency is a Target of this solution
        if (TARGET ${depend})
            get_target_property(TARGET_TYPE ${depend} TYPE)
            if (${TARGET_TYPE} STREQUAL "STATIC_LIBRARY" OR ${TARGET_TYPE} STREQUAL "SHARED_LIBRARY")
                nap_add_dependency(${targetName} ${depend})
                nap_direct_dependencies(${depend} childDependList)
            else()
                message(FATAL_ERROR "- ${targetName}: Unknown dependency type '${depend}-${TARGET_TYPE}'")
            endif ()
        else()
            message(FATAL_ERROR "- ${targetName}: Unknown dependency '${depend}'")
        endif()

        if (childDependList)
            nap_target_dependencies(${targetName} "${childDependList}")
        endif()

    endforeach()

endfunction()

#------------------------------------------------------------------------------

# Devuelve la visibilidad con la que enlazar: la que se haya pasado como
# argumento extra, o PUBLIC, que es lo que hacia la firma antigua de
# target_link_libraries.
function(nap_link_scope _ret)

    if (ARGC GREATER 1)
        set(${_ret} "${ARGV1}" PARENT_SCOPE)
    else()
        set(${_ret} "PUBLIC" PARENT_SCOPE)
    endif()

endfunction()

#------------------------------------------------------------------------------

# El segundo argumento, opcional, es la visibilidad (PUBLIC por omision, que es
# lo que hacia la firma antigua de target_link_libraries). Todas las llamadas de
# prj/ usan ya la firma con palabra clave: mezclar las dos en un mismo target es
# un error de CMake, asi que se convirtieron todas a la vez (NAP-002).
function(nap_link_inet_depends targetName)

    nap_link_scope(napScope ${ARGN})

    if(NAPPGUI_IS_PACKAGE)
        target_link_libraries(${targetName} ${napScope} nappgui::encode)
    else()
        target_link_libraries(${targetName} ${napScope} "encode")
    endif()

    if(WIN32)
        target_link_libraries(${targetName} ${napScope} wininet)

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        find_package(CURL)
        if (${CURL_FOUND})
            target_link_libraries(${targetName} ${napScope} ${CURL_LIBRARY})
        else()
            message(FATAL_ERROR "NAppGUI necesita libCURL para el modulo 'inet'. En Debian/Ubuntu: sudo apt-get install libcurl4-openssl-dev")
        endif()

    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_link_inet targetName)

    nap_link_scope(napScope ${ARGN})

    if(NAPPGUI_IS_PACKAGE)
        target_link_libraries(${targetName} ${napScope} nappgui::inet)
    else()
        target_link_libraries(${targetName} ${napScope} "inet")
    endif()

    nap_link_inet_depends(${targetName} ${napScope})

endfunction()

#------------------------------------------------------------------------------

function(nap_link_opengl_depends targetName)

    nap_link_scope(napScope ${ARGN})

    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        find_package(OpenGL REQUIRED EGL)
        target_link_libraries(${targetName} ${napScope} ${OPENGL_LIBRARY})

        if (OPENGL_egl_LIBRARY)
            target_link_libraries(${targetName} ${napScope} ${OPENGL_egl_LIBRARY})

        else()
            # CMake older than 3.10 not detect EGL
            if (NOT EGL_INCLUDE_DIR OR NOT EGL_LIBRARY)
                message("-- Direct search of EGL")
                find_path(EGL_INCLUDE_DIR "egl.h" PATHS "/usr/include/*" "/usr/local/include/*")
                find_file(EGL_LIBRARY "libEGL.so" PATHS "/usr/lib/*" "/usr/local/lib/*")

                if (EGL_INCLUDE_DIR)
                    message(STATUS "-- EGL_INCLUDE_DIR: ${EGL_INCLUDE_DIR}")
                else()
                    message(FATAL_ERROR "-- EGL_INCLUDE_DIR: NOT FOUND")
                endif()

                if (EGL_LIBRARY)
                    message(STATUS "-- EGL_LIBRARY: ${EGL_LIBRARY}")
                else()
                    message(FATAL_ERROR "-- EGL_LIBRARY: NOT FOUND")
                endif()

            endif()

            target_include_directories(${targetName} PRIVATE ${EGL_INCLUDE_DIR})
            target_link_libraries(${targetName} ${napScope} ${EGL_LIBRARY})

        endif()

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        # En macOS ${OPENGL_LIBRARY} es la ruta absoluta del framework dentro
        # del sysroot del Xcode con el que se compilo el SDK, y meterla en el
        # paquete exportado lo ataria a esa instalacion. '-framework OpenGL' no.
        # Es el mismo motivo por el que NAP-002 dejo de usar ${COCOA_LIB}.
        # find_package se sigue llamando para fallar pronto y con un mensaje
        # claro si OpenGL no esta.
        #
        # ogl3d/osx/ogl3dimp.m usa NSOpenGLContext, que es de AppKit, asi que
        # tambien necesita Cocoa y el runtime de Objective-C.
        find_package(OpenGL REQUIRED)
        target_link_libraries(${targetName} ${napScope} "-framework OpenGL" "-framework Cocoa" "objc")

    else()
        find_package(OpenGL REQUIRED)
        target_link_libraries(${targetName} ${napScope} ${OPENGL_LIBRARY})

    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_link_opengl targetName)

    nap_link_scope(napScope ${ARGN})

    # OpenGL ya no se anade aqui: es un requisito de uso de la propia libreria
    # 'ogl3d' (nap_library_requires), asi que llega igual por el arbol y por el
    # paquete exportado. Ver NAP-034.
    if(NAPPGUI_IS_PACKAGE)
        target_link_libraries(${targetName} ${napScope} nappgui::ogl3d)
    else()
        target_link_libraries(${targetName} ${napScope} ogl3d)
    endif()

    if(NOT NAPPGUI_IS_PACKAGE)
        get_target_property(TARGET_TYPE ogl3d TYPE)
        if (${TARGET_TYPE} STREQUAL "SHARED_LIBRARY")
            set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS NAPPGUI_OGL3D_IMPORT_DLL)
        endif()
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_link_with_libraries targetName targetType firstLevelDepends)

    # Las librerias del sistema ya no se anaden aqui: son requisitos de uso de
    # la libreria de NAppGUI que de verdad las necesita (nap_library_requires),
    # asi que llegan igual al ejecutable, tanto en el arbol como a traves del
    # paquete exportado. Ver NAP-002.
    if (${targetType} STREQUAL "DYNAMIC_LIB")
        set(napScope PUBLIC)
    else()
        set(napScope PRIVATE)
    endif()

    #
    # Link with direct target dependencies
    #
    set(${targetName}_LINKDEPENDS "" CACHE INTERNAL "")
    nap_target_dependencies(${targetName} "${firstLevelDepends}")

    if (${targetName}_LINKDEPENDS)
        foreach(depend ${${targetName}_LINKDEPENDS})
            target_link_libraries(${targetName} ${napScope} ${depend})
            get_target_property(DEPEND_TARGET_TYPE ${depend} TYPE)
            if (${DEPEND_TARGET_TYPE} STREQUAL "SHARED_LIBRARY")
                nap_target_relpath(${${depend}_SRCPATH} dependPath)
                get_filename_component(dependPathUpper ${dependPath} NAME)
                string(TOUPPER ${dependPathUpper} dependPathUpper)
                set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS NAPPGUI_${dependPathUpper}_IMPORT_DLL)
            endif()
        endforeach()

    endif()

    #
    # Link with precompiled NAppGUI
    #
    if (NAPPGUI_IS_PACKAGE)
        if (${targetType} STREQUAL "DESKTOP_APP" OR ${targetType} STREQUAL "DYNAMIC_LIB")
            target_link_libraries(${targetName} ${napScope} "nappgui::osapp;nappgui::gui;nappgui::osgui;nappgui::draw2d;nappgui::geom2d;nappgui::core;nappgui::osbs;nappgui::sewer")
        elseif (${targetType} STREQUAL "COMMAND_APP")
            target_link_libraries(${targetName} ${napScope} "nappgui::draw2d;nappgui::geom2d;nappgui::core;nappgui::osbs;nappgui::sewer")
        endif()
    endif()

    # In GCC the g++ linker must be used
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux" OR ${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        set_target_properties(${targetName} PROPERTIES LINKER_LANGUAGE CXX)
    endif()

endfunction()

#------------------------------------------------------------------------------
# Requisitos de uso de las librerias del SDK (NAP-002)
#
# Antes, ws2_32/gdiplus/GTK3/Cocoa/... se anadian al ejecutable final dentro de
# nap_desktop_app() y nap_command_app(). Asi solo las conseguia quien usaba las
# macros del SDK: un consumidor que hace lo idiomatico
#
#     find_package(nappgui REQUIRED)
#     target_link_libraries(app PRIVATE ${NAPPGUI_LIBRARIES})
#
# se quedaba sin ellas y no enlazaba en ninguna plataforma. Ahora cada libreria
# declara lo que necesita, el paquete exportado lo traslada y los dos caminos
# (arbol e instalado) convergen.
#
# 'scope' es INTERFACE en las librerias estaticas (no enlazan, solo propagan) y
# PUBLIC en las dinamicas (enlazan y ademas propagan).
#------------------------------------------------------------------------------
function(nap_library_requires libName scope)

    #
    # Runtime de C++ y libm
    #
    # sewer, core, geom2d y osgui tienen unidades .cpp. En el arbol lo tapa el
    # LINKER_LANGUAGE CXX que fuerza nap_link_with_libraries; un consumidor en C
    # puro enlaza con el driver de C y se queda sin typeinfo/vtable.
    #
    if (${libName} STREQUAL "sewer")
        if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            target_link_libraries(sewer ${scope} "stdc++" "m")
        elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
            target_link_libraries(sewer ${scope} "c++")
        endif()
    endif()

    #
    # osbs: sockets, hilos y carga dinamica
    #
    if (${libName} STREQUAL "osbs")
        if (WIN32)
            target_link_libraries(osbs ${scope} ws2_32)

        elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            find_package(Threads)
            if (NOT Threads_FOUND)
                message(FATAL_ERROR "NAppGUI necesita pthreads para el modulo 'osbs'. En Debian/Ubuntu: sudo apt-get install build-essential")
            endif()

            # Nombres planos, no Threads::Threads: un target importado obligaria
            # al consumidor a un find_dependency() antes de importar el paquete.
            # Con glibc >= 2.34 esto queda vacio, porque libpthread y libdl se
            # fusionaron dentro de libc.
            if (CMAKE_THREAD_LIBS_INIT)
                target_link_libraries(osbs ${scope} ${CMAKE_THREAD_LIBS_INIT})
            endif()

            if (CMAKE_DL_LIBS)
                target_link_libraries(osbs ${scope} ${CMAKE_DL_LIBS})
            endif()
        endif()
    endif()

    #
    # draw2d y osgui: el backend grafico nativo
    #
    if (${libName} STREQUAL "draw2d" OR ${libName} STREQUAL "osgui")
        if (WIN32)
            if (${libName} STREQUAL "draw2d")
                target_link_libraries(draw2d ${scope} gdiplus shlwapi)
            else()
                # dwmapi: la barra de titulo en modo oscuro va por
                # DwmSetWindowAttribute, que es API documentada (NAP-042).
                target_link_libraries(osgui ${scope} comctl32 uxtheme dwmapi)
            endif()

        elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            if (NOT CMAKE_TOOLKIT)
                message(FATAL_ERROR "CMAKE_TOOLKIT is not set")
            endif()

            if (${CMAKE_TOOLKIT} STREQUAL "GTK3")
                find_package(PkgConfig REQUIRED)
                pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
                if (GTK3_LIBRARY_DIRS)
                    target_link_directories(${libName} ${scope} ${GTK3_LIBRARY_DIRS})
                endif()
                target_link_libraries(${libName} ${scope} ${GTK3_LIBRARIES})
            endif()

        endif()
    endif()

    #
    # macOS: Cocoa y el runtime de Objective-C
    #
    # No solo el backend grafico: 'osbs' tambien tiene una unidad .m
    # (src/osbs/osx/sinfo.m, que incluye <Cocoa/Cocoa.h>), asi que hasta una
    # aplicacion de consola necesita el framework y el runtime.
    #
    if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        if (${libName} STREQUAL "osbs" OR ${libName} STREQUAL "draw2d" OR ${libName} STREQUAL "osgui")
            # '-framework X' y no la ruta absoluta de ${COCOA_LIB}: el paquete no
            # debe llevar cableado el sysroot del Xcode con el que se compilo.
            # 'objc' resuelve _objc_retain / _objc_release de las unidades .m.
            target_link_libraries(${libName} ${scope} "-framework Cocoa" "objc")
        endif()

        # UniformTypeIdentifiers solo lo usa el backend grafico (dialogos de
        # fichero), y solo existe desde macOS 12.
        if (${libName} STREQUAL "draw2d" OR ${libName} STREQUAL "osgui")
            if (CMAKE_OSX_DEPLOYMENT_TARGET VERSION_GREATER 11.9999)
                target_link_libraries(${libName} ${scope} "-framework UniformTypeIdentifiers")
            endif()
        endif()
    endif()

    #
    # ogl3d: OpenGL (NAP-034)
    #
    # Se declara sin condiciones, igual que 'inet' declara libCURL/wininet. La
    # busqueda no anade ningun requisito nuevo para construir el SDK: 'ogl3d' se
    # compila siempre y su 'glew.h' ya incluye <GL/glu.h> (o <OpenGL/glu.h> en
    # macOS), asi que sin las cabeceras de OpenGL el SDK no compilaba tampoco
    # antes. Lo unico que cambia es que ahora tambien se busca la libreria.
    #
    if (${libName} STREQUAL "ogl3d")
        nap_link_opengl_depends(${libName} ${scope})
    endif()

    #
    # osgui: WebView
    #
    if (${libName} STREQUAL "osgui" AND NAPPGUI_WEB_SUPPORT)
        if (WIN32)
            # El loader se instala en 'lib/<arch>' y se compila desde
            # 'src/osgui/win/depend/<arch>'. $<INSTALL_PREFIX> es lo que
            # install(EXPORT) reescribe a ${_IMPORT_PREFIX} en el paquete.
            target_link_libraries(osgui ${scope}
                $<BUILD_INTERFACE:${NAPPGUI_ROOT_PATH}/src/osgui/win/depend/${CMAKE_ARCHITECTURE}/WebView2LoaderStatic.lib>
                $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/lib/${CMAKE_ARCHITECTURE}/WebView2LoaderStatic.lib>
                version)

        elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
            target_link_libraries(osgui ${scope} "-framework WebKit")

        elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            nap_find_webview_linux(WEBVIEW_FOUND WEBVIEW_HEADERS WEBVIEW_LIBS)
            if (WEBVIEW_FOUND)
                target_link_libraries(osgui ${scope} ${WEBVIEW_LIBS})
            endif()
        endif()
    endif()

endfunction()

#------------------------------------------------------------------------------

function(nap_target targetName targetType dependList nrcMode privateHeaders targetExport)

    # Get source files
    set(${targetName}_SRCFILES "" CACHE INTERNAL "")
    set(${targetName}_SRCSUBDIRS "" CACHE INTERNAL "")
    set(${targetName}_PUBLICHEADERS "" CACHE INTERNAL "")
    nap_source_files(${targetName} ${CMAKE_CURRENT_SOURCE_DIR} "src" TRUE)
    set(srcFiles ${${targetName}_SRCFILES})
    set(srcSubDirs ${${targetName}_SRCSUBDIRS})
    set(publicHeaders ${${targetName}_PUBLICHEADERS})

    # Las cabeceras internas se compilan como el resto, pero no se instalan.
    # Ver nap_filter_public_headers() y NAP-011.
    nap_filter_public_headers("${publicHeaders}" "${privateHeaders}" publicHeaders)

    # Get resources
    set(${targetName}_SRCPATH "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "")
    nap_resource_packs(${targetName} ${targetType} ${nrcMode} ${CMAKE_CURRENT_SOURCE_DIR} resFiles resIncludeDir)
    nap_target_relpath(${${targetName}_SRCPATH} targetPath)

    # Generate target (library, executable)
    if (targetType STREQUAL STATIC_LIB)
        message(STATUS "- [OK] ${targetName}: Static library")
        add_library(${targetName} STATIC ${srcFiles} ${resFiles})

        # Clang, GNU, Intel, MSVC
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${targetName} PUBLIC "-fPIC")
        endif()

        # Install the public headers
        if (publicHeaders)
            set_target_properties(${targetName} PROPERTIES PUBLIC_HEADER "${publicHeaders}")
        endif()

    elseif (targetType STREQUAL DYNAMIC_LIB)
        message(STATUS "- [OK] ${targetName}: Dynamic library")
        add_library(${targetName} SHARED ${srcFiles} ${resFiles})

        get_filename_component(targetPathUpper ${targetPath} NAME)
        string(TOUPPER ${targetPathUpper} targetPathUpper)
        set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS NAPPGUI_${targetPathUpper}_EXPORT_DLL)

        # Append import for use in NAppGUI-based future targets
        file(APPEND "${NAPPGUI_INSTALL_DEFINES}" "NAPPGUI_${targetPathUpper}_IMPORT_DLL\n")

        # Clang, GNU, Intel, MSVC
        if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${targetName} PUBLIC "-fPIC;-fvisibility=hidden")
            set_target_properties(${targetName} PROPERTIES LINK_FLAGS "-fPIC")
        endif()

        # Install the public headers
        if (publicHeaders)
            set_target_properties(${targetName} PROPERTIES PUBLIC_HEADER "${publicHeaders}")
        endif()

    elseif (targetType STREQUAL WIN_DESKTOP)
        message(STATUS "- [OK] ${targetName}: Desktop application")
        add_executable(${targetName} WIN32 ${srcFiles} ${resFiles})

    elseif (targetType STREQUAL WIN_CONSOLE)
        message(STATUS "- [OK] ${targetName}: Command-line application")
        add_executable(${targetName} ${srcFiles} ${resFiles})

    elseif (targetType STREQUAL APPLE_BUNDLE)
        message(STATUS "- [OK] ${targetName}: Desktop application")
        add_executable(${targetName} MACOSX_BUNDLE ${srcFiles} ${resFiles})

    elseif (targetType STREQUAL APPLE_CONSOLE)
        message(STATUS "- [OK] ${targetName}: Command-line application")
        add_executable(${targetName} ${srcFiles} ${resFiles})

    elseif (targetType STREQUAL LINUX_DESKTOP)
        message(STATUS "- [OK] ${targetName}: Desktop application")
        add_executable(${targetName} ${srcFiles} ${resFiles})

    elseif (targetType STREQUAL LINUX_CONSOLE)
        message(STATUS "- [OK] ${targetName}: Command-line application")
        add_executable(${targetName} ${srcFiles} ${resFiles})

    else()
        message(FATAL_ERROR "Unknown target type")

    endif()

    # Output directories for generated binaries
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${config} configUpper)
        set_property(TARGET ${targetName} APPEND PROPERTY ARCHIVE_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/${config}/lib")
        set_property(TARGET ${targetName} APPEND PROPERTY LIBRARY_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/${config}/bin")
        set_property(TARGET ${targetName} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY_${configUpper} "${CMAKE_BINARY_DIR}/${config}/bin")
    endforeach()

    # ¿Se instala este target, y entra en el conjunto exportado? (NAP-022)
    #
    # El conjunto exportado es un contrato: quien esta en el, el proyecto se
    # compromete a mantenerlo. Antes entraban los 32 targets del arbol, asi que
    # find_package(nappgui) importaba nappgui::Bricks y un consumidor podia
    # escribir target_link_libraries(app nappgui::GuiHello) y funcionaba.
    #
    # La decision la toma cada funcion que crea targets, y llega hasta aqui en
    # 'targetExport': nap_library() si, nap_command_app() y nap_desktop_app()
    # no, con EXPORT / NO_EXPORT para los pocos casos al reves. Sustituye al
    # 'string(FIND "${targetPath}" "test/")' de NAP-010, que era el mismo
    # criterio escrito como caso particular.
    #
    # En el proyecto de un consumidor (NAPPGUI_IS_PACKAGE) no hay SDK que
    # publicar: sus targets se instalan como siempre, para que su
    # 'cmake --install' siga sirviendo, pero no entran en ningun conjunto
    # exportado (que ademas seria el de NAppGUI, no el suyo).
    if (NAPPGUI_IS_PACKAGE)
        set(napInstall TRUE)
        set(napExportArgs "")
    else()
        set(napInstall ${targetExport})
        if (targetExport)
            set(napExportArgs EXPORT nappgui-targets)
        else()
            set(napExportArgs "")
        endif()
    endif()

    # Install binaries and headers
    get_filename_component(targetPathSingle ${targetPath} NAME)
    if (napInstall)
        install(TARGETS ${targetName} ${napExportArgs}
                    LIBRARY DESTINATION "bin" PERMISSIONS ${INSTALL_PERM}
                    RUNTIME DESTINATION "bin" PERMISSIONS ${INSTALL_PERM}
                    ARCHIVE DESTINATION "lib" PERMISSIONS ${INSTALL_PERM}
                    BUNDLE DESTINATION "bin"
                    PUBLIC_HEADER DESTINATION "inc/${targetPathSingle}")

        # Install the .pdb files
        if (targetType STREQUAL STATIC_LIB)
            install(FILES "$<TARGET_FILE_DIR:${targetName}>/${targetName}.pdb" DESTINATION "lib" PERMISSIONS ${INSTALL_PERM} OPTIONAL)
        else()
            install(FILES "$<TARGET_FILE_DIR:${targetName}>/${targetName}.pdb" DESTINATION "bin" PERMISSIONS ${INSTALL_PERM} OPTIONAL)
        endif()
    endif()

    # Install the .exp files
    # install(FILES "$<TARGET_LINKER_FILE_DIR:${targetName}>/${targetName}.exp" CONFIGURATIONS "${config}" DESTINATION "lib/${config}" PERMISSIONS ${INSTALL_PERM} OPTIONAL)

    # Install resource packs
    #
    # Los .res empaquetados y el icono de Linux se copian siempre junto al
    # ejecutable, para poder ejecutarlo desde el arbol de build; instalarlos
    # solo tiene sentido si el ejecutable tambien se instala.
    nap_install_resource_packs(${targetName} ${targetType} ${CMAKE_CURRENT_SOURCE_DIR} ${nrcMode} ${napInstall})

    # Target Definitions
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${config} configUpper)
        set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:${config}>:CMAKE_${configUpper}>)
    endforeach()

    if (WIN32)
        # Visual Studio 2005/2008 doesn't have <stdint.h>
        if(MSVC_VERSION EQUAL 1500 OR MSVC_VERSION LESS 1500)
            target_include_directories(${targetName} PRIVATE $<BUILD_INTERFACE:${NAPPGUI_ROOT_PATH}/prj/depend>)
        endif()

        # Platform toolset macro
        #set_property(TARGET ${targetName} APPEND PROPERTY COMPILE_DEFINITIONS VS_PLATFORM=${VS_TOOLSET_NUMBER})

        # Force the name of the pdb (vc110.pdb in VS2012)
        set_target_properties(${targetName} PROPERTIES COMPILE_PDB_NAME ${targetName})
    endif()

    # WebView support
    if (${targetName} STREQUAL "osgui")
        if (NAPPGUI_WEB_SUPPORT)
            if (WIN32)
                target_compile_definitions("osgui" PUBLIC "-DNAPPGUI_WEB_SUPPORT")
                # Install Win32 WebView binaries
                install(DIRECTORY "${NAPPGUI_ROOT_PATH}/src/osgui/win/depend/x64" DESTINATION "lib")
                install(DIRECTORY "${NAPPGUI_ROOT_PATH}/src/osgui/win/depend/x86" DESTINATION "lib")
                install(DIRECTORY "${NAPPGUI_ROOT_PATH}/src/osgui/win/depend/arm64" DESTINATION "lib")

            elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
                target_compile_definitions("osgui" PUBLIC "-DNAPPGUI_WEB_SUPPORT")

            elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
                nap_find_webview_linux(WEBVIEW_FOUND WEBVIEW_HEADERS WEBVIEW_LIBS)
                if (WEBVIEW_FOUND)
                    foreach(dir ${WEBVIEW_HEADERS})
                        target_include_directories(${targetName} PRIVATE $<BUILD_INTERFACE:${dir}>)
                    endforeach()

                    target_compile_definitions("osgui" PUBLIC "-DNAPPGUI_WEB_SUPPORT")
                endif()
            endif()
        endif()
    endif()

    # GTK Include directories
    if (CMAKE_TOOLKIT)
        if (${CMAKE_TOOLKIT} STREQUAL "GTK3")
            if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/gtk)
                # Use the package PkgConfig to detect GTK+ headers/library files
                find_package(PkgConfig REQUIRED)
                pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
                foreach(dir ${GTK3_INCLUDE_DIRS})
                    target_include_directories(${targetName} PRIVATE $<BUILD_INTERFACE:${dir}>)
                endforeach()
                target_compile_definitions(${targetName} PUBLIC "-D__GTK3_TOOLKIT__")
            endif()
        endif()
    endif()

    # Target global include directories
    #
    # En las librerias esto es PUBLIC, no PRIVATE: la ruta de cabeceras es un
    # requisito de uso, no un detalle privado. Con PRIVATE, los targets
    # exportados nappgui::* se instalaban sin INTERFACE_INCLUDE_DIRECTORIES y
    # el consumidor externo no encontraba 'nappgui.h' (NAP-002).
    # En los ejecutables sigue siendo PRIVATE: nadie enlaza contra ellos.
    if (targetType STREQUAL STATIC_LIB OR targetType STREQUAL DYNAMIC_LIB)
        set(napIncScope PUBLIC)
    else()
        set(napIncScope PRIVATE)
    endif()

    if (NOT NAPPGUI_IS_PACKAGE)
        target_include_directories(${targetName} ${napIncScope} $<BUILD_INTERFACE:${NAPPGUI_ROOT_PATH}/src>)
    else()
        target_include_directories(${targetName} ${napIncScope} $<BUILD_INTERFACE:${NAPPGUI_INCLUDE_PATH}>)
    endif()

    target_include_directories(${targetName} ${napIncScope} $<INSTALL_INTERFACE:inc>)

    # Include dir for target generated resources
    if (resIncludeDir)
        target_include_directories(${targetName} PRIVATE $<BUILD_INTERFACE:${resIncludeDir}>)
    endif()

    # Target dependency for compile order
    if (dependList)
        foreach(depend ${dependList})
            add_dependencies(${targetName} ${depend})
        endforeach()
    endif()

    # Target default C/C++ standards
    nap_target_c_standard(${targetName} "90")
    nap_target_cxx_standard(${targetName} "98")

endfunction()

#------------------------------------------------------------------------------

function(nap_library libName dependList buildShared nrcMode)

    # Una libreria es, por omision, parte del SDK que se publica. Ver
    # nap_target_options() y NAP-022.
    nap_target_options(${libName} TRUE napExport napPrivateHeaders ${ARGN})

    if (buildShared)
        nap_target(${libName} DYNAMIC_LIB "${dependList}" ${nrcMode} "${napPrivateHeaders}" ${napExport})
        nap_link_with_libraries(${libName} DYNAMIC_LIB "${dependList}")
        nap_target_rpath(${libName} NO "")
        set(napReqScope PUBLIC)

    else()
        nap_target(${libName} STATIC_LIB "${dependList}" ${nrcMode} "${napPrivateHeaders}" ${napExport})

        # # In Linux, static libs must link with other libs
        # if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        #     nap_link_with_libraries(${libName} STATIC_LIB "${dependList}")
        # endif()
        set(napReqScope INTERFACE)
    endif()

    #
    # El grafo entre las propias librerias, declarado (NAP-033)
    #
    # 'dependList' servia solo para el orden de compilacion (add_dependencies en
    # nap_target). El orden de enlace vivia en una cadena de texto,
    # NAPPGUI_LIBRARIES, que el consumidor estaba obligado a copiar entera y en
    # el orden correcto. Declarado el grafo, 'nappgui::osapp' a secas arrastra
    # el resto y es CMake quien calcula el orden.
    #
    # Una estatica no enlaza nada, solo propaga: INTERFACE. Una dinamica si
    # enlaza, y de eso ya se encarga nap_link_with_libraries() con PUBLIC.
    #
    # Los nombres van sin prefijo a proposito: en el arbol los targets se llaman
    # 'core', 'sewer'..., y es install(EXPORT ... NAMESPACE nappgui::) quien los
    # reescribe a 'nappgui::core' al generar el paquete.
    if (dependList AND NOT buildShared)
        target_link_libraries(${libName} INTERFACE ${dependList})
    endif()

    # Requisitos de uso de esta libreria: solo para las del propio SDK, que son
    # las que se compilan cuando NAppGUI no viene de un paquete. Ver NAP-002.
    if (NOT NAPPGUI_IS_PACKAGE)
        nap_library_requires(${libName} ${napReqScope})
    endif()

    set(NAPPGUI_CACHE_DEPENDS_${libName} "${dependList}" CACHE INTERNAL "")

endfunction()

#------------------------------------------------------------------------------

function(nap_command_app appName dependList nrcMode)

    # Una aplicacion no forma parte, por omision, del SDK que se publica: no se
    # instala ni entra en el conjunto exportado. 'nrc' lo pide con EXPORT. Ver
    # nap_target_options() y NAP-022.
    nap_target_options(${appName} FALSE napExport napPrivateHeaders ${ARGN})

    # This is for demos and apps that are compiled with the SDK.
    # For apps that use NAppGUI with find_package(), NAppGUI is linked in nap_link_with_libraries().
    if (NOT NAPPGUI_IS_PACKAGE)
        if (dependList)
            set(dependList "core;${dependList}")
        else()
            set(dependList "core")
        endif()
    endif()

    if (WIN32)
        nap_target("${appName}" WIN_CONSOLE "${dependList}" ${nrcMode} "${napPrivateHeaders}" ${napExport})

        if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
            foreach(config ${CMAKE_CONFIGURATION_TYPES})
                string(TOUPPER ${config} configUpper)
                set_target_properties(${appName} PROPERTIES LINK_FLAGS_${configUpper} "/SUBSYSTEM:CONSOLE")
            endforeach()
        endif()

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        nap_target("${appName}" APPLE_CONSOLE "${dependList}" ${nrcMode} "${napPrivateHeaders}" ${napExport})

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        nap_target("${appName}" LINUX_CONSOLE "${dependList}" ${nrcMode} "${napPrivateHeaders}" ${napExport})

    else()
        message(FATAL_ERROR "No se puede construir la aplicacion '${appName}': sistema no soportado (${CMAKE_SYSTEM_NAME}). NAppGUI soporta Windows, Darwin y Linux.")

    endif()

    nap_link_with_libraries(${appName} COMMAND_APP "${dependList}")
    nap_target_rpath(${appName} NO "")

endfunction()

#------------------------------------------------------------------------------

function(nap_desktop_app appName dependList nrcMode)

    # Una aplicacion no forma parte, por omision, del SDK que se publica: no se
    # instala ni entra en el conjunto exportado. Ver nap_target_options() y
    # NAP-022.
    nap_target_options(${appName} FALSE napExport napPrivateHeaders ${ARGN})

    # This is for demos and apps that are compiled with the SDK.
    # For apps that use NAppGUI with find_package(), NAppGUI is linked in nap_link_with_libraries().
    if (NOT NAPPGUI_IS_PACKAGE)
        if (dependList)
            set(dependList "osapp;${dependList}")
        else()
            set(dependList "osapp")
        endif()
    endif()

    if (WIN32)
        nap_target(${appName} WIN_DESKTOP "${dependList}" ${nrcMode} "${napPrivateHeaders}" ${napExport})
        if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
            foreach(config ${CMAKE_CONFIGURATION_TYPES})
                string(TOUPPER ${config} configUpper)
                set_target_properties(${appName} PROPERTIES LINK_FLAGS_${configUpper} "/SUBSYSTEM:WINDOWS")
            endforeach()
        endif()
        set(macOSBundle NO)

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
        nap_target(${appName} APPLE_BUNDLE "${dependList}" ${nrcMode} "${napPrivateHeaders}" ${napExport})

        # Info.plist configure
        # Proyect provides its own Info.plist?
        if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
            set_target_properties(${appName} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
        # Use default template
        else()
            set_target_properties(${appName} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${NAPPGUI_ROOT_PATH}/prj/templates/Info.plist)
        endif()

        # Overwrite some properties
        # bundleProp(${bundleName} "NSHumanReadableCopyright" "${CURRENT_YEAR} ${PACK_VENDOR}")
        # bundleProp(${bundleName} "CFBundleVersion" "${PACK_VERSION}")

        set(macOSBundle YES)

    elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
        nap_target("${appName}" LINUX_DESKTOP "${dependList}" ${nrcMode} "${napPrivateHeaders}" ${napExport})
        set(macOSBundle NO)

    else()
        message("Unknown platform")

    endif()

    nap_link_with_libraries(${appName} DESKTOP_APP "${dependList}")
    nap_target_rpath(${appName} ${macOSBundle} "")

endfunction()
