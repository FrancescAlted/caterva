# Copyright (C) 2018 Francesc Alted, Aleix Alcacer.
# Copyright (C) 2019-present Blosc Development team <blosc@blosc.org>
# All rights reserved.
#
# This source code is licensed under both the BSD-style license (found in the
# LICENSE file in the root directory of this source tree) and the GPLv2 (found
# in the COPYING file in the root directory of this source tree).
# You may select, at your option, one of the above-listed licenses.

cmake_minimum_required(VERSION 3.12)


cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0077 NEW)

project(caterva VERSION 0.3.3 LANGUAGES C)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(SHARED_LIB "Create shared library" ON)
option(STATIC_LIB "Create static library" ON)
option(CATERVA_BUILD_TESTS "Build tests" ON)
option(CATERVA_BUILD_EXAMPLES "Build examples" ON)

if (MSVC)
    # warning level 4 and all warnings as errors
    # add_compile_options(/W4 /WX)
    add_compile_options(/W4)
    add_compile_options(/wd4232)  # TODO: fix this (warning C4232: nonstandard extension used)
    add_compile_options(/wd4127)  # TODO: fix this (warning C4127: conditional expression is constant)
else()
    # lots of warnings and all warnings as errors
    # add_compile_options(-Wall -Wextra -pedantic -Werror)
    add_compile_options(-Wall -Wextra)
endif()

if (ENABLE_ASAN)
    message(STATUS "Enable sanitizers")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Os -fno-omit-frame-pointer -fsanitize=address")
endif()

if (ENABLE_COVERAGE)
    message(STATUS "Coverage is enabled")
endif()

set(CATERVA_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(CATERVA_SRC ${CMAKE_CURRENT_LIST_DIR}/caterva)

if (SHARED_LIB)
    set(CATERVA_LIB caterva_shared)
elseif (STATIC_LIB)
    set(CATERVA_LIB caterva_static)
else()
    message(FATAL_ERROR "No library is created")
endif ()

add_subdirectory(contribs/c-blosc2)
include_directories(contribs/c-blosc2/blosc)
set(BLOSC_LIB blosc2_static)

include_directories(${CATERVA_SRC})

include(CTest)

file(GLOB SRC_FILES ${CATERVA_SRC}/*.c)

if (SHARED_LIB)
    message(STATUS "Building caterva shared lib")
    add_library(caterva_shared SHARED ${SRC_FILES})
    if (ENABLE_COVERAGE)
        target_compile_options(caterva_shared PRIVATE -fprofile-arcs -ftest-coverage)
        target_link_libraries(caterva_shared blosc2_static -fprofile-arcs)
    else()
        target_compile_options(caterva_shared PRIVATE)
        target_link_libraries(caterva_shared blosc2_static)
    endif()
    set_target_properties(caterva_shared PROPERTIES OUTPUT_NAME caterva)
    install(TARGETS caterva_shared DESTINATION lib)
endif ()

if (STATIC_LIB)
    message(STATUS "Building caterva static lib")
    add_library(caterva_static STATIC ${SRC_FILES})
    if (ENABLE_COVERAGE)
        target_compile_options(caterva_static PRIVATE -fprofile-arcs -ftest-coverage)
        target_link_libraries(caterva_static blosc2_static -fprofile-arcs)
    else()
        target_compile_options(caterva_static PRIVATE)
        target_link_libraries(caterva_static blosc2_static)
    endif()
    set_target_properties(caterva_static PROPERTIES OUTPUT_NAME caterva)
    install(TARGETS caterva_static DESTINATION lib)
endif()

install(FILES ${CATERVA_SRC}/caterva.h DESTINATION include)

if(CATERVA_BUILD_EXAMPLES)
    message(STATUS "Adding Caterva examples")
    add_subdirectory(examples)
endif()

if (CATERVA_BUILD_TESTS)
    message(STATUS "Adding Caterva tests")
    enable_testing()
    add_subdirectory(tests)
endif()

