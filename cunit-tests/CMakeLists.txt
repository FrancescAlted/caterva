add_executable(caterva_cunit main.c)

target_link_libraries(caterva_cunit ${CATERVA_LIB} ${BLOSC_LIB})
