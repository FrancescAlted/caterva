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

static char* test_copy(caterva_context_t *ctx, uint8_t itemsize, uint8_t ndim, int64_t *shape,
                      caterva_storage_backend_t backend, uint32_t *chunkshape, uint32_t *blockshape,
                      bool enforceframe, char *filename, caterva_storage_backend_t backend2,
                       uint32_t *chunkshape2, uint32_t *blockshape2, bool enforceframe2, char *filename2) {

    caterva_params_t params;
    params.itemsize = itemsize;
    params.ndim = ndim;
    for (int i = 0; i < ndim; ++i) {
        params.shape[i] = shape[i];
    }

    double datatoserialize = 8.34;

    caterva_storage_t storage = {0};
    storage.backend = backend;
    switch (backend) {
        case CATERVA_STORAGE_PLAINBUFFER:
            break;
        case CATERVA_STORAGE_BLOSC:
            storage.properties.blosc.filename = filename;
            storage.properties.blosc.enforceframe = enforceframe;
            for (int i = 0; i < ndim; ++i) {
                storage.properties.blosc.chunkshape[i] = chunkshape[i];
                storage.properties.blosc.blockshape[i] = blockshape[i];
            }
            storage.properties.blosc.nmetalayers = 1;
            storage.properties.blosc.metalayers[0].name = "random";
            storage.properties.blosc.metalayers[0].sdata = (uint8_t *) &datatoserialize;
            storage.properties.blosc.metalayers[0].size = 8;

            break;
        default:
            MU_ASSERT_CATERVA(CATERVA_ERR_INVALID_STORAGE);
    }

    /* Create original data */
    size_t buffersize = itemsize;
    for (int i = 0; i < ndim; ++i) {
        buffersize *= (size_t) shape[i];
    }
    uint8_t *buffer = malloc(buffersize);
    MU_ASSERT("Buffer filled incorrectly", fill_buf(buffer, itemsize, (buffersize / itemsize)));

    /* Create caterva_array_t with original data */
    caterva_array_t *src;
    MU_ASSERT_CATERVA(caterva_array_from_buffer(ctx, buffer, buffersize, &params, &storage, &src));

    /* Assert the metalayers creation */
    if (storage.backend == CATERVA_STORAGE_BLOSC) {
        if (blosc2_has_metalayer(src->sc, "random") < 0) {
            MU_ASSERT_CATERVA(CATERVA_ERR_BLOSC_FAILED);
        }
        double *serializeddata;
        uint32_t len;
        blosc2_get_metalayer(src->sc, "random", (uint8_t **) &serializeddata, &len);
        if (*serializeddata != datatoserialize) {
            MU_ASSERT_CATERVA(CATERVA_ERR_BLOSC_FAILED);
        }
        free(serializeddata);
    }

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
    MU_ASSERT_CATERVA(caterva_array_copy(ctx, src, &storage2, &dest));

    uint8_t *buffer_dest = malloc(buffersize);
    MU_ASSERT_CATERVA(caterva_array_to_buffer(ctx, dest, buffer_dest, buffersize));

    /* Testing */
    MU_ASSERT_BUFFER(buffer, buffer_dest, buffersize);

    /* Free mallocs */
    free(buffer);
    free(buffer_dest);
    MU_ASSERT_CATERVA(caterva_array_free(ctx, &src));
    MU_ASSERT_CATERVA(caterva_array_free(ctx, &dest));

    return 0;
}


caterva_context_t *ctx;

static char* copy_setup() {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_context_new(&cfg, &ctx);
    return 0;
}

static char* copy_teardown() {
    caterva_context_free(&ctx);
    return 0;
}

static char* copy_1_double_plain_blosc() {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 1;
    int64_t shape[] = {17};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    uint32_t chunkshape[] = {0};
    uint32_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_BLOSC;
    uint32_t chunkshape2[] = {14};
    uint32_t blockshape2[] = {13};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    return test_copy(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe,
              filename, backend2, chunkshape2, blockshape2, enforceframe2, filename2);
}

static char* copy_2_double_blosc_blosc() {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 2;
    int64_t shape[] = {40, 40};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    uint32_t chunkshape[] = {26, 17};
    uint32_t blockshape[] = {15, 12};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_BLOSC;
    uint32_t chunkshape2[] = {14, 24};
    uint32_t blockshape2[] = {7, 11};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    return test_copy(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe,
            filename, backend2, chunkshape2, blockshape2, enforceframe2, filename2);
}


static char* copy_3_double_blosc_plainbuffer() {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 3;
    int64_t shape[] = {134, 56, 204};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    uint32_t chunkshape[] = {26, 17, 34};
    uint32_t blockshape[] = {11, 8, 12};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_PLAINBUFFER;
    uint32_t chunkshape2[] = {0};
    uint32_t blockshape2[] = {0};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    return test_copy(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe,
            filename, backend2, chunkshape2, blockshape2, enforceframe2, filename2);
}


static char* copy_4_float_plainbuffer_blosc_frame() {
    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 4;
    int64_t shape[] = {8, 10, 8, 9};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    uint32_t chunkshape[] = {0};
    uint32_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_BLOSC;
    uint32_t chunkshape2[] = {3, 7, 3, 3};
    uint32_t blockshape2[] = {3, 3, 3, 3};
    bool enforceframe2 = true;
    char *filename2 = NULL;

    return test_copy(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe,
            filename, backend2, chunkshape2, blockshape2, enforceframe2, filename2);
}


static char* copy_5_double_plainbuffer_plainbuffer() {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 5;
    int64_t shape[] = {4, 3, 8, 5, 10};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    uint32_t chunkshape[] = {0};
    uint32_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_PLAINBUFFER;
    uint32_t chunkshape2[] = {0};
    uint32_t blockshape2[] = {0};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    return test_copy(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe,
            filename, backend2, chunkshape2, blockshape2, enforceframe2, filename2);
}

static char* copy_6_uint16_blosc_plainbuffer() {
    uint8_t itemsize = sizeof(uint16_t);
    uint8_t ndim = 6;
    int64_t shape[] = {4, 3, 8, 5, 6, 5};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    uint32_t chunkshape[] = {3, 2, 5, 3, 5, 4};
    uint32_t blockshape[] = {2, 1, 3, 2, 2, 3};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_PLAINBUFFER;
    uint32_t chunkshape2[] = {0};
    uint32_t blockshape2[] = {0};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    return test_copy(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe,
              filename, backend2, chunkshape2, blockshape2, enforceframe2, filename2);
}

static char* copy_7_float_blosc_blosc() {
    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 7;
    int64_t shape[] = {2, 3, 5, 3, 4, 1, 4};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    uint32_t chunkshape[] = {1, 2, 4, 2, 3, 1, 3};
    uint32_t blockshape[] = {1, 1, 3, 2, 2, 1, 3};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_BLOSC;
    uint32_t chunkshape2[] = {2, 1, 5, 3, 3, 1, 3};
    uint32_t blockshape2[] = {2, 1, 4, 1, 2, 1, 2};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    return test_copy(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe,
              filename, backend2, chunkshape2, blockshape2, enforceframe2, filename2);
}

static char* copy_8_uint8_plainbuffer_plainbuffer() {
    uint8_t itemsize = sizeof(uint8_t);
    uint8_t ndim = 8;
    int64_t shape[] = {4, 3, 6, 8, 2, 5, 9, 3};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    uint32_t chunkshape[] = {0};
    uint32_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_PLAINBUFFER;
    uint32_t chunkshape2[] = {0};
    uint32_t blockshape2[] = {0};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    return test_copy(ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe,
              filename, backend2, chunkshape2, blockshape2, enforceframe2, filename2);
}

static char* all_tests() {
    MU_RUN_SETUP(copy_setup)

    MU_RUN_TEST(copy_1_double_plain_blosc)
    MU_RUN_TEST(copy_2_double_blosc_blosc)
    MU_RUN_TEST(copy_3_double_blosc_plainbuffer)
    MU_RUN_TEST(copy_4_float_plainbuffer_blosc_frame)
    MU_RUN_TEST(copy_5_double_plainbuffer_plainbuffer)
    MU_RUN_TEST(copy_6_uint16_blosc_plainbuffer)
    MU_RUN_TEST(copy_7_float_blosc_blosc)
    MU_RUN_TEST(copy_8_uint8_plainbuffer_plainbuffer)

    MU_RUN_TEARDOWN(copy_teardown)
    return 0;
}

MU_RUN_SUITE("COPY");
