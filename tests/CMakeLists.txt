file(GLOB SOURCES test_*.c)

foreach (source ${SOURCES})
    get_filename_component(target ${source} NAME_WE)
    add_executable(${target}  main.c ${target}.c)
    target_link_libraries(${target} caterva blosc)
    add_test(NAME ${target} COMMAND ${target})
    set_tests_properties(${target} PROPERTIES LABELS "caterva")
endforeach (source)
