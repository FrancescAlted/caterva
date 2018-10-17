cmake_minimum_required(VERSION 2.8.10)

project(caterva)

set(CATERVA ${CMAKE_SOURCE_DIR}/caterva)

add_library(caterva STATIC ${CATERVA}/caterva.c)

install(TARGETS caterva DESTINATION lib)

install(FILES ${CATERVA}/caterva.h DESTINATION include)