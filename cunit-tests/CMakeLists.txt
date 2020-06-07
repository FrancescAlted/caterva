add_executable(caterva_cunit test_unit.c)

target_link_libraries(caterva_cunit ${CATERVA_LIB} ${BLOSC_LIB})
if(WIN32)
    # For a rational of this, see comments in https://github.com/bvdberg/ctest/pull/35
    set_property(TARGET caterva_cunit APPEND_STRING PROPERTY LINK_FLAGS " /INCREMENTAL:NO")
endif()
