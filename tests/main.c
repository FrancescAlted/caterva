/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#include <stdio.h>
#include "include/main.h"

int main(int argc, const char *argv[])
{
    int result = lwtest_main(argc, argv);

    //Return number of tests failed
    return result;
}
