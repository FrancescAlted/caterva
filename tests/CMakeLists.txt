file(GLOB SOURCES test_*.c)

foreach (source ${SOURCES})
    get_filename_component(target ${source} NAME_WE)
    add_executable(${target}  main.c ${target}.c)
    target_link_libraries(${target} caterva blosc_static)
    if(WIN32)
        # For a rational of this, see comments in https://github.com/bvdberg/ctest/pull/35
        set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " /INCREMENTAL:NO")
    endif()
    add_test(NAME ${target} COMMAND ${target})
    set_tests_properties(${target} PROPERTIES LABELS "caterva")
endforeach (source)
