/*
  Copyright (C) 2021  The Blosc Developers <blosc@blosc.org>
  https://blosc.org
  License: BSD 3-Clause (see LICENSE.txt)

  Example program demonstrating use of the Blosc filter from C code.

  To compile this program:

  $ gcc urcodecs.c -o urcodecs -lblosc2

  To run:

  $ ./urcodecs
*/



#include <caterva.h>
#include <stdio.h>
#include <blosc2.h>
#include "../contribs/c-blosc2/plugins/codecs/codecs-registry.c"

int main() {
    blosc_timestamp_t t0, t1;

    blosc_init();
    int8_t ndim = 2;
    uint8_t itemsize = sizeof(int64_t);

    int64_t shape[] = {745, 400};
    int32_t chunkshape[] = {150, 100};
    int32_t blockshape[] = {21, 30};

    int64_t nbytes = itemsize;
    for (int i = 0; i < ndim; ++i) {
        nbytes *= shape[i];
    }

    int64_t *src = malloc((size_t) nbytes);
    for (int i = 0; i < nbytes / itemsize; ++i) {
        src[i] = (int64_t) i;
    }

    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    cfg.nthreads = 1;
    cfg.splitmode = BLOSC_ALWAYS_SPLIT;
    cfg.compcodec = BLOSC_CODEC_NDLZ;
    cfg.compmeta = 4;
    cfg.complevel = 5;

    caterva_ctx_t *ctx;
    caterva_ctx_new(&cfg, &ctx);

    caterva_params_t params;
    params.itemsize = itemsize;
    params.ndim = ndim;
    for (int i = 0; i < ndim; ++i) {
        params.shape[i] = shape[i];
    }

    caterva_storage_t storage = {0};
    storage.backend = CATERVA_STORAGE_BLOSC;
    for (int i = 0; i < ndim; ++i) {
        storage.properties.blosc.chunkshape[i] = chunkshape[i];
        storage.properties.blosc.blockshape[i] = blockshape[i];
    }

    caterva_array_t *arr;
    blosc_set_timestamp(&t0);
    CATERVA_ERROR(caterva_from_buffer(ctx, src, nbytes, &params, &storage, &arr));
    blosc_set_timestamp(&t1);
    printf("from_buffer: %.4f s\n", blosc_elapsed_secs(t0, t1));

    int64_t *buffer = malloc(nbytes);
    int64_t buffer_size = nbytes;
    blosc_set_timestamp(&t0);
    CATERVA_ERROR(caterva_to_buffer(ctx, arr, buffer, buffer_size));
    blosc_set_timestamp(&t1);
    printf("get_slice: %.4f s\n", blosc_elapsed_secs(t0, t1));

    blosc_destroy();

    for (int i = 0; i < buffer_size / itemsize; i++) {
        if (src[i] != buffer[i]) {
            printf("i: %d, data %lld, dest %lld", i, src[i], buffer[i]);
            printf("\n Decompressed data differs from original!\n");
            return -1;
        }
    }

    free(src);
    free(buffer);

    caterva_free(ctx, &arr);
    caterva_ctx_free(&ctx);

    return 0;
}
