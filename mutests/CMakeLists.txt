# Copyright (C) 2018 Francesc Alted, Aleix Alcacer.
# Copyright (C) 2019-present Blosc Development team <blosc@blosc.org>
# All rights reserved.
#
# This source code is licensed under both the BSD-style license (found in the
# LICENSE file in the root directory of this source tree) and the GPLv2 (found
# in the COPYING file in the root directory of this source tree).
# You may select, at your option, one of the above-listed licenses.

file(GLOB SOURCES test_*.c)

add_executable(caterva_mutest test_unit.c)

# Allow linking with Blosc2 from external apps defining BLOSC_LIB
target_link_libraries(caterva_mutest ${CATERVA_LIB} ${BLOSC_LIB})
add_test(NAME caterva_mutest COMMAND caterva_mutest)
