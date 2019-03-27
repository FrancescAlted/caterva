file(GLOB SOURCES *.c)

foreach (source ${SOURCES})
    get_filename_component(target ${source} NAME_WE)
    add_executable(${target} ${target}.c ${CATERVA_SRC}/caterva.c)
    if (BLOSC_LIB)
        # Allow linking with Blosc from IronArray without the need to install it
        target_link_libraries(${target} caterva blosc_static)
    else(BLOSC_LIB)
        target_link_libraries(${target} caterva blosc)
    endif(BLOSC_LIB)
endforeach (source)
