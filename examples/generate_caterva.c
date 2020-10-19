/*
    Copyright (C) 2014  Francesc Alted
    http://blosc.org
    License: BSD 3-Clause (see LICENSE.txt)

    Example program demonstrating the use of a Blosc from C code.

    To compile this program:

    $ gcc -O contexts.c -o contexts -lblosc2

    To run:

    $ ./contexts
    Blosc version info: 2.0.0a2 ($Date:: 2016-01-08 #$)
    Compression: 40000000 -> 999393 (40.0x)
    Correctly extracted 5 elements from compressed chunk!
    Decompression succesful!
    Succesful roundtrip!

*/

#include <stdio.h>
#include "blosc2.h"
#include "caterva.h"
#include "caterva_blosc.h"

#define SIZE 10 * 1000 * 1000
#define NTHREADS 2


int main(void) {

    int ndim = 2;
    int typesize = 4;
    int32_t shape[8] = {256, 256};
    int32_t chunkshape[8] = {256, 256};
    int32_t blockshape[8] = {64, 64};
    int isize = (int)(shape[0] * shape[1]);
    int nbytes = typesize * isize;
    uint32_t data[isize];
    FILE *f = fopen("/mnt/c/Users/sosca/CLionProjects/Caterva/examples/res6.bin", "rb");
    fread(data, sizeof(data), 1, f);
    fclose(f);

    uint8_t *data2 = (uint8_t*) data;
    caterva_array_t *array;
    caterva_context_t *ctx;
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_context_new(&cfg, &ctx);

    caterva_params_t params;
    params.itemsize = typesize;
    params.ndim = ndim;
    for (int i = 0; i < ndim; ++i) {
        params.shape[i] = shape[i];
    }

    caterva_storage_t storage = {0};
    storage.backend = CATERVA_STORAGE_BLOSC;
    storage.properties.blosc.filename = "image6.cat";
    storage.properties.blosc.enforceframe = true;
    for (int i = 0; i < ndim; ++i) {
        storage.properties.blosc.chunkshape[i] = chunkshape[i];
        storage.properties.blosc.blockshape[i] = blockshape[i];
    }

    int result = caterva_array_from_buffer(ctx, data2, nbytes, &params, &storage, &array);

    printf("data2: \n");
    for (int i = 0; i < nbytes; i++) {
        printf("%u, ", data2[i]);
    }

    caterva_array_free(ctx, &array);
    caterva_context_free(&ctx);

    return result;
}
