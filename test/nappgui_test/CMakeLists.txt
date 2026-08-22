nap_command_app(nappgui_test "encode;geom2d" NRC_NONE)
set_target_properties(nappgui_test PROPERTIES FOLDER "test")

# Un test de ctest por modulo: el ejecutable los selecciona por argumento.
foreach(modulo sewer core geom2d encode)
    add_test(NAME ${modulo} COMMAND nappgui_test ${modulo})
    set_tests_properties(${modulo} PROPERTIES TIMEOUT 60)
endforeach()
