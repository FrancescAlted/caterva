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
#include <unistd.h>

static void test_persistency(caterva_context_t *ctx, uint8_t itemsize, uint8_t ndim, int64_t *shape,
                             caterva_storage_backend_t backend, int64_t *chunkshape, int64_t *blockshape,
                             bool enforceframe, char *filename) {

    if (access(filename, F_OK) != -1) {
        remove(filename);
    }
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
                storage.properties.blosc.chunkshape[i] = chunkshape[i];
                storage.properties.blosc.blockshape[i] = blockshape[i];
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
    CATERVA_TEST_ERROR(caterva_array_from_buffer(ctx, buffer, buffersize, &params, &storage, &src));

    caterva_array_t *dest;
    caterva_array_from_file(ctx, filename, true, &dest);

    /* Fill dest array with caterva_array_t data */
    uint8_t *buffer_dest = malloc(buffersize);
    CATERVA_TEST_ERROR(caterva_array_to_buffer(ctx, dest, buffer_dest, buffersize));

    /* Testing */
    double tol = (itemsize == 4) ? 1e-6 : 1e-15;
    assert_buf(buffer, buffer_dest, itemsize, dest->size, tol);

    /* Free mallocs */
    free(buffer);
    free(buffer_dest);
    CATERVA_TEST_ERROR(caterva_array_free(ctx, &src));
    CATERVA_TEST_ERROR(caterva_array_free(ctx, &dest));
    if (access(filename, F_OK) != -1) {
        remove(filename);
    }
}


LWTEST_DATA(persistency) {
    caterva_context_t *ctx;
    char *filename;
};

LWTEST_SETUP(persistency) {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_context_new(&cfg, &data->ctx);
    data->filename = "persistency.caterva";
}

LWTEST_TEARDOWN(persistency) {
    caterva_context_free(&data->ctx);
}


LWTEST_FIXTURE(persistency, 1_uint16) {
    uint8_t itemsize = sizeof(uint16_t);
    uint8_t ndim = 1;
    int64_t shape[] = {500};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {200};
    int64_t blockshape[] = {80};
    bool enforceframe = true;
    char *filename = data->filename;

    test_persistency(data->ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}

LWTEST_FIXTURE(persistency, 2_uint8) {
    uint8_t itemsize = sizeof(uint8_t);
    uint8_t ndim = 2;
    int64_t shape[] = {400, 300};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {55, 67};
    int64_t blockshape[] = {40, 40};
    bool enforceframe = true;
    char *filename = data->filename;

    test_persistency(data->ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}


LWTEST_FIXTURE(persistency, 3_double) {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 3;
    int64_t shape[] = {134, 56, 204};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {26, 17, 34};
    int64_t blockshape[] = {11, 8, 13};
    bool enforceframe = true;
    char *filename = data->filename;

    test_persistency(data->ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);

}


LWTEST_FIXTURE(persistency, 4_float) {
    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 4;
    int64_t shape[] = {10, 13, 18, 25};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {9, 5, 11, 10};
    int64_t blockshape[] = {2, 2, 5, 3};
    bool enforceframe = true;
    char *filename = data->filename;

    test_persistency(data->ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}

LWTEST_FIXTURE(persistency, 4_uint8) {
    uint8_t itemsize = sizeof(uint8_t);
    uint8_t ndim = 4;
    int64_t shape[] = {78, 85, 34, 56};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {23, 12, 24, 50};
    int64_t blockshape[] = {11, 5, 7, 13};
    bool enforceframe = true;
    char *filename = data->filename;

    test_persistency(data->ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}


LWTEST_FIXTURE(persistency, 5_double) {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 5;
    int64_t shape[] = {25, 27, 24, 25, 12};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {10, 15, 8, 11, 11};
    int64_t blockshape[] = {3, 3, 3, 3, 3};
    bool enforceframe = true;
    char *filename = data->filename;

    test_persistency(data->ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}

LWTEST_FIXTURE(persistency, 6_uint16) {
    uint8_t itemsize = sizeof(uint16_t);
    uint8_t ndim = 6;
    int64_t shape[] = {4, 3, 8, 5, 10, 12};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {2, 2, 3, 3, 4, 5};
    int64_t blockshape[] = {2, 1, 2, 2, 3, 4};
    bool enforceframe = true;
    char *filename = data->filename;

    test_persistency(data->ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}

LWTEST_FIXTURE(persistency, 7_uint64) {
    uint8_t itemsize = sizeof(uint64_t);
    uint8_t ndim = 7;
    int64_t shape[] =  {4, 15, 11, 6, 12, 8, 7};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {4, 3, 4, 3, 3, 5, 3};
    int64_t blockshape[] = {1, 3, 2, 2, 1, 2, 2};
    bool enforceframe = true;
    char *filename = data->filename;

    test_persistency(data->ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}

LWTEST_FIXTURE(persistency, 8_uint8) {
    uint8_t itemsize = sizeof(uint8_t);
    uint8_t ndim = 8;
    int64_t shape[] = {4, 3, 8, 5, 10, 12, 6, 4};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {3, 2, 3, 3, 4, 5, 4, 2};
    int64_t blockshape[] = {2, 1, 2, 2, 3, 4, 3, 1};
    bool enforceframe = true;
    char *filename = data->filename;

    test_persistency(data->ctx, itemsize, ndim, shape, backend, chunkshape, blockshape, enforceframe, filename);
}
