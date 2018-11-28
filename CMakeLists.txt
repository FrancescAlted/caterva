# Copyright (C) 2018  Francesc Alted
# Copyright (C) 2018  Aleix Alcacer


cmake_minimum_required(VERSION 2.8.10)

project(caterva)

set(CATERVA_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(CATERVA_SRC ${CMAKE_CURRENT_LIST_DIR}/caterva)
include_directories(${CATERVA_SRC})

add_library(caterva STATIC ${CATERVA_SRC}/caterva.c)

install(TARGETS caterva DESTINATION lib)

install(FILES ${CATERVA_SRC}/caterva.h DESTINATION include)

enable_testing()

add_subdirectory(tests)
add_subdirectory(examples)
