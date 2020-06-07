/*
 * Copyright (C) 2018 Francesc Alted, Aleix Alcacer.
 * Copyright (C) 2019-present Blosc Development team <blosc@blosc.org>
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

#include "test_append.c"

int tests_run = 0;
int tests_failed = 0;

int main(int argc, char **argv) {
    char* filter = "";
    if (argc == 2) {
        filter = argv[1];
    }

    append_tests(filter);

    int tests_ok = tests_run - tests_failed;
    printf("RESULTS: %d tests (%d ok, %d failed)\n", tests_run, tests_ok, tests_failed);

    return tests_failed;
}