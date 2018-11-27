file(GLOB SOURCES *.c)

foreach (source ${SOURCES})
    get_filename_component(target ${source} NAME_WE)
    add_executable(${target} ${target}.c)
    target_link_libraries(${target} caterva blosc)
endforeach (source)
