# Copyright (C) 2018  Francesc Alted
# Copyright (C) 2018  Aleix Alcacer


cmake_minimum_required(VERSION 2.8.10)

project(caterva)

set(CATERVA ${CMAKE_SOURCE_DIR}/caterva)
include_directories(${CATERVA})

enable_testing()

add_library(caterva STATIC ${CATERVA}/caterva.c)

install(TARGETS caterva DESTINATION lib)

install(FILES ${CATERVA}/caterva.h DESTINATION include)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/tests/test_*.c)

foreach (source ${SOURCES})
    get_filename_component(target ${source} NAME_WE)
    add_executable(${target} ${CMAKE_SOURCE_DIR}/tests/${target}.c)
    target_link_libraries(${target} LINK_PUBLIC caterva blosc)
    add_test(NAME ${target} COMMAND ${target} "${CMAKE_SOURCE_DIR}/tests/${target}.csv")
endforeach (source)
