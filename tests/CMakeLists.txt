# Copyright (C) 2018 Francesc Alted, Aleix Alcacer.
# Copyright (C) 2019-present Blosc Development team <blosc@blosc.org>
# All rights reserved.
#
# This source code is licensed under both the BSD-style license (found in the
# LICENSE file in the root directory of this source tree) and the GPLv2 (found
# in the COPYING file in the root directory of this source tree).
# You may select, at your option, one of the above-listed licenses.

file(GLOB SOURCES test_*.c)

add_executable(caterva_tests main.c ${SOURCES})
if (BLOSC_LIB)
    # Allow linking with Blosc2 from external apps defining BLOSC_LIB
    target_link_libraries(caterva_tests ${CATERVA_LIB} ${BLOSC_LIB})
else(BLOSC_LIB)
    target_link_libraries(caterva_tests ${CATERVA_LIB} blosc2)
endif(BLOSC_LIB)

if(WIN32)
    # For a rational of this, see comments in https://github.com/bvdberg/ctest/pull/35
    set_property(TARGET caterva_tests APPEND_STRING PROPERTY LINK_FLAGS " /INCREMENTAL:NO")
endif()

foreach (source ${SOURCES})
    get_filename_component(target_name ${source} NAME_WE)
    set(target caterva_${target_name})
    add_executable(${target}  main.c ${target_name}.c)
    if (BLOSC_LIB)
        target_link_libraries(${target} ${CATERVA_LIB} ${BLOSC_LIB})
    else(BLOSC_LIB)
        target_link_libraries(${target} ${CATERVA_LIB} blosc2)
    endif(BLOSC_LIB)

    if(WIN32)
        # For a rational of this, see comments in https://github.com/bvdberg/ctest/pull/35
        set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " /INCREMENTAL:NO")
    endif()
    add_test(NAME ${target} COMMAND ${target})
    set_tests_properties(${target} PROPERTIES LABELS "caterva")
endforeach (source)
