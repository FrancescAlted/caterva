/*
 * Copyright (c) 2018 Francesc Alted, Aleix Alcacer.
 * Copyright (C) 2019-present Blosc Development team <blosc@blosc.org>
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

#include "test_common.h"

static char* test_roundtrip(caterva_context_t *ctx, uint8_t itemsize, uint8_t ndim, int64_t *shape,
                           caterva_storage_backend_t backend, int64_t *chunkshape, int64_t *blockshape,
                           bool enforceframe, char *filename) {

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
    uint8_t *buffer = malloc((size_t) buffersize);
    MU_ASSERT("Buffer filled incorrectly", fill_buf(buffer, itemsize,(size_t) buffersize / itemsize));

    /* Create caterva_array_t with original data */
    caterva_array_t *src;
    MU_ASSERT_CATERVA(caterva_array_from_buffer(ctx, buffer, buffersize, &params, &storage, &src));

    /* Fill dest array with caterva_array_t data */
    uint8_t *buffer_dest = malloc((size_t) buffersize);
    MU_ASSERT_CATERVA(caterva_array_to_buffer(ctx, src, buffer_dest, buffersize));

    /* Testing */
    MU_ASSERT_BUFFER(buffer, buffer_dest, buffersize);

    /* Free mallocs */
    free(buffer);
    free(buffer_dest);
    MU_ASSERT_CATERVA(caterva_array_free(ctx, &src));
    return 0;
}


caterva_context_t *ctx;

static char* roundtrip_setup() {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    cfg.nthreads = 2;
    cfg.compcodec = BLOSC_BLOSCLZ;
    caterva_context_new(&cfg, &ctx);
    return 0;
}

static char* roundtrip_teardown() {
    caterva_context_free(&ctx);
    return 0;
}


static char* roundtrip_1_uint16_plainbuffer() {
    uint8_t itemsize = sizeof(uint16_t);
    uint8_t ndim = 1;
    int64_t shape[] = {7};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {0};
    int64_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    return test_roundtrip(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}

static char* roundtrip_2_uint8_plainbuffer() {
    uint8_t itemsize = sizeof(uint8_t);
    uint8_t ndim = 2;
    int64_t shape[] = {4, 3};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {0};
    int64_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    return test_roundtrip(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}


static char* roundtrip_3_double_blosc() {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 3;
    int64_t shape[] = {134, 56, 204};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {26, 17, 34};
    int64_t blockshape[] = {11, 8, 13};
    bool enforceframe = false;
    char *filename = NULL;

    return test_roundtrip(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);

}


static char* roundtrip_4_float_blosc() {
    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 4;
    int64_t shape[] = {4, 3, 8, 5};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {2, 2, 3, 3};
    int64_t blockshape[] = {2, 2, 2, 3};
    bool enforceframe = false;
    char *filename = NULL;

    return test_roundtrip(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}

static char* roundtrip_4_uint8_plainbuffer() {
    uint8_t itemsize = sizeof(uint8_t);
    uint8_t ndim = 4;
    int64_t shape[] = {78, 85, 34, 56};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {0};
    int64_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    return test_roundtrip(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}

static char* roundtrip_5_double_plainbuffer() {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 5;
    int64_t shape[] = {35, 55, 24, 36, 12};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {0};
    int64_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    return test_roundtrip(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}

static char* roundtrip_6_uint16_blosc() {
    uint8_t itemsize = sizeof(uint16_t);
    uint8_t ndim = 6;
    int64_t shape[] = {4, 3, 8, 5, 10, 12};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {2, 2, 3, 3, 4, 5};
    int64_t blockshape[] = {2, 1, 2, 2, 3, 4};
    bool enforceframe = false;
    char *filename = NULL;

    return test_roundtrip(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}

static char* roundtrip_7_double_plainbuffer() {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 7;
    int64_t shape[] =  {4, 15, 11, 6, 12, 8, 7};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {0};
    int64_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    return test_roundtrip(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}

static char* roundtrip_8_uint8_blosc() {
    uint8_t itemsize = sizeof(uint8_t);
    uint8_t ndim = 8;
    int64_t shape[] = {4, 3, 8, 5, 10, 12, 6, 4};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {3, 2, 3, 3, 4, 5, 4, 2};
    int64_t blockshape[] = {2, 1, 2, 2, 3, 4, 3, 1};
    bool enforceframe = false;
    char *filename = NULL;

    return test_roundtrip(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}


static char* all_tests() {
    MU_RUN_SETUP(roundtrip_setup)

    MU_RUN_TEST(roundtrip_1_uint16_plainbuffer)
    MU_RUN_TEST(roundtrip_2_uint8_plainbuffer)
    MU_RUN_TEST(roundtrip_3_double_blosc)
    MU_RUN_TEST(roundtrip_4_float_blosc)
    MU_RUN_TEST(roundtrip_4_uint8_plainbuffer)
    MU_RUN_TEST(roundtrip_5_double_plainbuffer)
    MU_RUN_TEST(roundtrip_6_uint16_blosc)
    MU_RUN_TEST(roundtrip_7_double_plainbuffer)
    MU_RUN_TEST(roundtrip_8_uint8_blosc)

    MU_RUN_TEARDOWN(roundtrip_teardown)
    return 0;
}

MU_RUN_SUITE("ROUNDTRIP")
