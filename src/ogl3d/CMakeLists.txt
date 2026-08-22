nap_library(ogl3d "sewer" "${NAPPGUI_SHARED}" NRC_NONE)

# Avoid GCC warnings in glew.c
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set_source_files_properties("glew.c" PROPERTIES COMPILE_FLAGS "-std=gnu99 -Wno-strict-prototypes -Wno-old-style-definition -Wno-address -w")
endif()

# OpenGL es un requisito de uso de esta libreria en los dos casos, estatico y
# dinamico, y lo declara nap_library_requires(). Antes solo se declaraba cuando
# se compilaba como dinamica, asi que 'nappgui::ogl3d' se exportaba con las
# cabeceras y nada mas y no habia forma de enlazarlo sin llamar a la macro
# nap_link_opengl() del sistema de build. Ver NAP-034.
