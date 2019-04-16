# Copyright (c) 2018-present, Francesc Alted, Aleix Alcacer.
# All rights reserved.
#
# This source code is licensed under both the BSD-style license (found in the
# LICENSE file in the root directory of this source tree) and the GPLv2 (found
# in the COPYING file in the root directory of this source tree).
# You may select, at your option, one of the above-listed licenses.

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
