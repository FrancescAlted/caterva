file(GLOB SOURCES test_*.c)

add_executable(caterva_tests main.c ${SOURCES})
if (BLOSC_LIB)
    # Allow linking with Blosc from IronArray without the need to install it
    target_link_libraries(caterva_tests caterva blosc_static)
else(BLOSC_LIB)
    target_link_libraries(caterva_tests caterva blosc)
endif(BLOSC_LIB)

if(WIN32)
    # For a rational of this, see comments in https://github.com/bvdberg/ctest/pull/35
    set_property(TARGET tests APPEND_STRING PROPERTY LINK_FLAGS " /INCREMENTAL:NO")
endif()

foreach (source ${SOURCES})
    get_filename_component(target ${source} NAME_WE)
    add_executable(${target}  main.c ${target}.c)
    if (BLOSC_LIB)
        target_link_libraries(${target} caterva blosc_static)
    else(BLOSC_LIB)
        target_link_libraries(${target} caterva blosc)
    endif(BLOSC_LIB)

    if(WIN32)
        # For a rational of this, see comments in https://github.com/bvdberg/ctest/pull/35
        set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " /INCREMENTAL:NO")
    endif()
    add_test(NAME ${target} COMMAND ${target})
    set_tests_properties(${target} PROPERTIES LABELS "caterva")
endforeach (source)
