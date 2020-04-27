# Copyright (C) 2018 Francesc Alted, Aleix Alcacer.
# Copyright (C) 2019-present Blosc Development team <blosc@blosc.org>
# All rights reserved.
#
# This source code is licensed under both the BSD-style license (found in the
# LICENSE file in the root directory of this source tree) and the GPLv2 (found
# in the COPYING file in the root directory of this source tree).
# You may select, at your option, one of the above-listed licenses.

cmake_minimum_required(VERSION 2.8.10)

project(caterva)

option(SHARED_LIB "Create shared library" ON)
option(STATIC_LIB "Create static library" ON)

set(CATERVA_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(CATERVA_SRC ${CMAKE_CURRENT_LIST_DIR}/caterva)

if (NOT BLOSC_LIB)
    find_library(DEP_LIB blosc2 PATHS ${BLOSC_DIR})
    if (NOT DEP_LIB)
        message(FATAL_ERROR "Blosc2 library not found")
    else()
        set(BLOSC_LIB ${DEP_LIB})
        message(STATUS "Blosc2 library found at: ${BLOSC_LIB}")
    endif()
    find_path(BLOSC_INCLUDE "blosc2.h")
    if (BLOSC_INCLUDE)
        message(STATUS "Blosc2 header found at: ${BLOSC_INCLUDE}")
    endif()
endif()


if (SHARED_LIB)
    set(CATERVA_LIB caterva_shared)
elseif (STATIC_LIB)
    set(CATERVA_LIB caterva_static)
else()
    error("No library is created")
endif ()

include_directories(${CATERVA_SRC})
if (BLOSC_INCLUDE)
    include_directories(${BLOSC_INCLUDE})
    message(STATUS "BLOSC_INCLUDE: ${BLOSC_INCLUDE}")
endif()

include(CTest)

file(GLOB SRC_FILES ${CATERVA_SRC}/*.c)

if (SHARED_LIB)
    add_library(caterva_shared SHARED ${SRC_FILES})
    if (DO_COVERAGE)
        target_compile_options(caterva_shared PRIVATE -fprofile-arcs -ftest-coverage)
        target_link_libraries(caterva_shared ${BLOSC_LIB} -fprofile-arcs)
    else()
        target_link_libraries(caterva_shared ${BLOSC_LIB})
    endif()
    set_target_properties(caterva_shared PROPERTIES OUTPUT_NAME caterva)
    install(TARGETS caterva_shared DESTINATION lib)
endif ()

if (STATIC_LIB)
    add_library(caterva_static STATIC ${SRC_FILES})
    if (DO_COVERAGE)
        target_compile_options(caterva_static PRIVATE -fprofile-arcs -ftest-coverage)
        target_link_libraries(caterva_static ${BLOSC_LIB} -fprofile-arcs)
    else()
        target_link_libraries(caterva_static ${BLOSC_LIB})
    endif()
    set_target_properties(caterva_static PROPERTIES OUTPUT_NAME caterva)
    install(TARGETS caterva_static DESTINATION lib)
endif()

install(FILES ${CATERVA_SRC}/caterva.h DESTINATION include)

enable_testing()
add_subdirectory(tests)
add_subdirectory(examples)

