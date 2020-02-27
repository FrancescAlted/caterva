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

static void test_copy(caterva_context_t *ctx, uint8_t itemsize, uint8_t ndim, int64_t *shape,
                      caterva_storage_backend_t backend, int64_t *chunkshape, bool enforceframe, char *filename,
                      caterva_storage_backend_t backend2, int64_t *chunkshape2, bool enforceframe2, char *filename2) {

    caterva_params_t params;
    params.itemsize = itemsize;
    params.ndim = ndim;
    for (int i = 0; i < ndim; ++i) {
        params.shape[i] = shape[i];
    }

    caterva_storage_t storage;
    storage.backend = backend;
    switch (backend) {
        case CATERVA_STORAGE_PLAINBUFFER:
            break;
        case CATERVA_STORAGE_BLOSC:
            storage.properties.blosc.filename = filename;
            storage.properties.blosc.enforceframe = enforceframe;
            for (int i = 0; i < ndim; ++i) {
                storage.properties.blosc.chunkshape[i] = chunkshape[i];
            }
            break;
        default:
            CATERVA_TEST_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    /* Create original data */
    int64_t buffersize = itemsize;
    for (int i = 0; i < ndim; ++i) {
        buffersize *= shape[i];
    }
    uint8_t *buffer = malloc(buffersize);
    fill_buf(buffer, itemsize, buffersize / itemsize);

    /* Create caterva_array_t with original data */
    caterva_array_t *src;
    CATERVA_TEST_ERROR(caterva_array_from_buffer(ctx, &params, &storage, buffer, buffersize, &src));


    /* Create storage for dest container */

    caterva_storage_t storage2;
    storage2.backend = backend2;
    switch (backend2) {
        case CATERVA_STORAGE_PLAINBUFFER:
            break;
        case CATERVA_STORAGE_BLOSC:
            storage2.properties.blosc.filename = filename2;
            storage2.properties.blosc.enforceframe = enforceframe2;
            for (int i = 0; i < ndim; ++i) {
                storage2.properties.blosc.chunkshape[i] = chunkshape2[i];
            }
            break;
        default:
            CATERVA_TEST_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    caterva_array_t *dest;
    CATERVA_TEST_ERROR(caterva_array_copy(ctx, &storage2, src, &dest));

    uint8_t *buffer_dest = malloc(buffersize);
    CATERVA_TEST_ERROR(caterva_array_to_buffer(ctx, dest, buffer_dest, buffersize));

    /* Testing */
    double tol = (itemsize == 4) ? 1e-6 : 1e-15;
    assert_buf(buffer, buffer_dest, itemsize, src->size, tol);

    /* Free mallocs */
    free(buffer);
    free(buffer_dest);
    CATERVA_TEST_ERROR(caterva_array_free(ctx, &src));
    CATERVA_TEST_ERROR(caterva_array_free(ctx, &dest));
}

LWTEST_DATA(copy) {
    caterva_context_t *ctx;
};

LWTEST_SETUP(copy) {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_context_new(&cfg, &data->ctx);
}

LWTEST_TEARDOWN(copy) {
    caterva_context_free(&data->ctx);
}

LWTEST_FIXTURE(copy, 2_double_blosc_blosc) {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 2;
    int64_t shape[] = {40, 40};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {26, 17};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape2[] = {14, 24};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    test_copy(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename, backend2,
              chunkshape2, enforceframe2, filename2);
}


LWTEST_FIXTURE(copy, 3_float_blosc_plainbuffer) {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 3;
    int64_t shape[] = {134, 56, 204};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {26, 17, 34};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape2[] = {};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    test_copy(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename, backend2,
              chunkshape2, enforceframe2, filename2);
}


LWTEST_FIXTURE(copy, 4_float_plainbuffer_blosc_frame) {
    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 4;
    int64_t shape[] = {4, 3, 8, 5};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape2[] = {2, 2, 3, 2};
    bool enforceframe2 = true;
    char *filename2 = NULL;

    test_copy(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename, backend2,
              chunkshape2, enforceframe2, filename2);
}


LWTEST_FIXTURE(copy, 5_float_plainbuffer_plainbuffer) {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 5;
    int64_t shape[] = {4, 3, 8, 5, 10};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape2[] = {};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    test_copy(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename, backend2,
              chunkshape2, enforceframe2, filename2);
}

