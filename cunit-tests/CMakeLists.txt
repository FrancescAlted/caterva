add_executable(caterva_cunit test_unit.c)

target_link_libraries(caterva_cunit ${CATERVA_LIB} ${BLOSC_LIB})

add_test(NAME caterva_cunit COMMAND caterva_cunit)
