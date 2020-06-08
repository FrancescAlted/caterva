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

#include "test_common.h"

static char* test_squeeze(caterva_context_t *ctx, uint8_t itemsize, uint8_t ndim, int64_t *shape,
                          caterva_storage_backend_t backend, int64_t *chunkshape, int64_t *blockshape, bool enforceframe, 
                          char *filename, caterva_storage_backend_t backend2, int64_t *chunkshape2, int64_t *blockshape2,
                          bool enforceframe2, char *filename2, int64_t *start, int64_t *stop) {
    caterva_params_t params;
    params.itemsize = itemsize;
    params.ndim = ndim;
    for (int i = 0; i < ndim; ++i) {
        params.shape[i] = shape[i];
    }

    caterva_storage_t storage = {0};
    storage.backend = backend;
    switch (backend) {
        case CATERVA_STORAGE_PLAINBUFFER:
            break;
        case CATERVA_STORAGE_BLOSC:
            storage.properties.blosc.filename = filename;
            storage.properties.blosc.enforceframe = enforceframe;
            for (int i = 0; i < ndim; ++i) {
                storage.properties.blosc.chunkshape[i] = (int32_t) chunkshape[i];
                storage.properties.blosc.blockshape[i] = (int32_t) blockshape[i];
            }
            break;
        default:
            MU_ASSERT_CATERVA(CATERVA_ERR_INVALID_STORAGE);
    }

    /* Create original data */
    int64_t buffersize = itemsize;
    for (int i = 0; i < ndim; ++i) {
        buffersize *= shape[i];
    }
    uint8_t *buffer = malloc(buffersize);
    MU_ASSERT("Buffer filled incorrectly", fill_buf(buffer, itemsize, buffersize / itemsize));

    /* Create caterva_array_t with original data */
    caterva_array_t *src;
    MU_ASSERT_CATERVA(caterva_array_from_buffer(ctx, buffer, buffersize, &params, &storage, &src));


    /* Create storage for dest container */

    caterva_storage_t storage2 = {0};
    storage2.backend = backend2;
    switch (backend2) {
        case CATERVA_STORAGE_PLAINBUFFER:
            break;
        case CATERVA_STORAGE_BLOSC:
            storage2.properties.blosc.filename = filename2;
            storage2.properties.blosc.enforceframe = enforceframe2;
            for (int i = 0; i < ndim; ++i) {
                storage2.properties.blosc.chunkshape[i] = chunkshape2[i];
                storage2.properties.blosc.blockshape[i] = blockshape2[i];
            }
            break;
        default:
            MU_ASSERT_CATERVA(CATERVA_ERR_INVALID_STORAGE);
    }

    caterva_array_t *dest;
    MU_ASSERT_CATERVA(caterva_array_get_slice(ctx, src, start, stop, &storage2, &dest));

    MU_ASSERT_CATERVA(caterva_array_squeeze(ctx, dest));

    MU_ASSERT("Squeeze doesn't perform well", src->ndim != dest->ndim);

    free(buffer);
    MU_ASSERT_CATERVA(caterva_array_free(ctx, &src));
    MU_ASSERT_CATERVA(caterva_array_free(ctx, &dest));
    
    return 0;
}


caterva_context_t *ctx;

static char* squeeze_setup() {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_context_new(&cfg, &ctx);
    return 0;
}

static char* squeeze_teardown() {
    caterva_context_free(&ctx);
    return 0;
}


static char* squeeze_2_float_blosc_plainbuffer() {
    int64_t start[] = {5, 20};
    int64_t stop[] = {23, 21};

    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 2;
    int64_t shape[] = {100, 100};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {10, 10};
    int64_t blockshape[] = {7, 6};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape2[] = {0};
    int64_t blockshape2[] = {0};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    return test_squeeze(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename,
                 backend2, chunkshape2, blockshape2, enforceframe2, filename2, start, stop);
}

static char* squeeze_3_float_blosc_blosc() {
    int64_t start[] = {5, 20, 60};
    int64_t stop[] = {23, 21, 99};

    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 3;
    int64_t shape[] = {100, 100, 100};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {10, 10, 10};
    int64_t blockshape[] = {3, 4, 3};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape2[] = {21, 1, 10};
    int64_t blockshape2[] = {8, 1, 5};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    return test_squeeze(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename,
        backend2, chunkshape2, blockshape2, enforceframe2, filename2, start, stop);
}

static char* squeeze_4_double_plainbuffer_blosc() {
    int64_t start[] = {5, 20, 10, 10};
    int64_t stop[] = {23, 21, 33, 39};

    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 4;
    int64_t shape[] = {30, 41, 42, 55};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {0};
    int64_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape2[] = {12, 1, 4, 12};
    int64_t blockshape2[] = {8, 1, 2, 5};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    return test_squeeze(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename,
                 backend2, chunkshape2, blockshape2, enforceframe2, filename2, start, stop);
}


static char* squeeze_5_uint8_plainbuffer_plainbuffer() {
    int64_t start[] = {1, 12, 3, 12, 6};
    int64_t stop[] = {16, 21, 19, 13, 21};

    uint8_t itemsize = sizeof(uint8_t);
    uint8_t ndim = 5;
    int64_t shape[] = {22, 25, 31, 19, 31};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {0};
    int64_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape2[] = {0};
    int64_t blockshape2[] = {0};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    return test_squeeze(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename,
                 backend2, chunkshape2, blockshape2, enforceframe2, filename2, start, stop);
}

static char* squeeze_6_float_blosc_plainbuffer_frame() {
    int64_t start[] = {3, 3, 2, 1, 0, 4};
    int64_t stop[] = {8, 10, 5, 2, 1, 9};

    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 6;
    int64_t shape[] = {8, 12, 6, 7, 6, 9};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {0};
    int64_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape2[] = {3, 7, 2, 1, 1, 4};
    int64_t blockshape2[] = {2, 5, 2, 1, 1, 3};
    bool enforceframe2 = true;
    char *filename2 = NULL;

    return test_squeeze(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename,
                 backend2, chunkshape2, blockshape2, enforceframe2, filename2, start, stop);
}

static char* squeeze_7_uint64_blosc_frame_plainbuffer() {
    int64_t start[] = {5, 3, 3, 2, 1, 0, 4};
    int64_t stop[] = {6, 8, 10, 5, 2, 1, 9};

    uint8_t itemsize = sizeof(uint64_t);
    uint8_t ndim = 7;
    int64_t shape[] = {6, 8, 12, 6, 7, 6, 9};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {2, 3, 5, 2, 4, 3, 2};
    int64_t blockshape[] = {1, 2, 3, 2, 3, 2, 1};
    bool enforceframe = true;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape2[] = {0};
    int64_t blockshape2[] = {0};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    return test_squeeze(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename,
                 backend2, chunkshape2, blockshape2, enforceframe2, filename2, start, stop);
}

static char* squeeze_8_uint16_blosc_frame_plainbuffer() {
    int64_t start[] = {5, 4, 3, 1, 2, 1, 0, 3};
    int64_t stop[] = {6, 8, 7, 4, 7, 2, 1, 9};

    uint8_t itemsize = sizeof(uint16_t);
    uint8_t ndim = 8;
    int64_t shape[] = {6, 8, 12, 6, 7, 2, 3, 9};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {2, 3, 7, 2, 4, 1, 3, 2};
    int64_t blockshape[] = {1, 2, 7, 2, 2, 1, 2, 1};
    bool enforceframe = true;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape2[] = {0};
    int64_t blockshape2[] = {0};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    return test_squeeze(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename,
                 backend2, chunkshape2, blockshape2, enforceframe2, filename2, start, stop);
}

static char* append_tests() {
    MU_RUN_SETUP(squeeze_setup);

    MU_RUN_TEST(squeeze_2_float_blosc_plainbuffer);
    MU_RUN_TEST(squeeze_3_float_blosc_blosc);
    MU_RUN_TEST(squeeze_4_double_plainbuffer_blosc);
    MU_RUN_TEST(squeeze_5_uint8_plainbuffer_plainbuffer);
    MU_RUN_TEST(squeeze_6_float_blosc_plainbuffer_frame);
    MU_RUN_TEST(squeeze_7_uint64_blosc_frame_plainbuffer);
    MU_RUN_TEST(squeeze_8_uint16_blosc_frame_plainbuffer);

    MU_RUN_TEARDOWN(squeeze_teardown);
    return 0;
}

int tests_run = 0;
int tests_failed = 0;

int main(int argc, char **argv) {
    char* filter = "";
    if (argc == 2) {
        filter = argv[1];
    }
    if(strncmp("squeeze", filter, strlen(filter)) != 0) {
        return 0;
    }
    printf("SQUEEZE SUITE\n");

    append_tests();

    int tests_ok = tests_run - tests_failed;
    printf("RESULTS: %d tests (%d ok, %d failed)\n", tests_run, tests_ok, tests_failed);

    return tests_failed;
}
