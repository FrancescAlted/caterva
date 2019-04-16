/*
 * Copyright (c) 2018-present, Francesc Alted, Aleix Alcacer.
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

#include <stdio.h>
#include "include/main.h"

int main(int argc, const char *argv[])
{
    int result = lwtest_main(argc, argv);

    //Return number of tests failed
    return result;
}
