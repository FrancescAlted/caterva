# Copyright (C) 2018  Francesc Alted
# Copyright (C) 2018  Aleix Alcacer


cmake_minimum_required(VERSION 2.8.10)

project(caterva)

set(CATERVA_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(CATERVA_SRC ${CMAKE_CURRENT_LIST_DIR}/caterva)
include_directories(${CATERVA_SRC})

enable_testing()

add_library(caterva STATIC ${CATERVA_SRC}/caterva.c)

install(TARGETS caterva DESTINATION lib)

install(FILES ${CATERVA_SRC}/caterva.h DESTINATION include)

file(GLOB SOURCES ${CATERVA_ROOT}/tests/test_*.c)

foreach (source ${SOURCES})
    get_filename_component(target ${source} NAME_WE)
    add_executable(${target} ${CATERVA_ROOT}/tests/${target}.c)
    target_link_libraries(${target} LINK_PUBLIC caterva ${BLOSC_LIB})
    add_test(NAME ${target} COMMAND ${target} "${CMAKE_SOURCE_DIR}/tests/${target}.csv")
endforeach (source)
