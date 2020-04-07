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

static void test_append(caterva_context_t *ctx, uint8_t itemsize, uint8_t ndim, int64_t *shape,
    caterva_storage_backend_t backend, int64_t *chunkshape, bool enforceframe, char *filename) {

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
            }
            break;
        default:
            CATERVA_TEST_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    caterva_array_t *src;
    CATERVA_TEST_ERROR(caterva_array_empty(ctx, &params, &storage, &src));

    /* Fill empty caterva_array_t with blocks */
    int64_t buffersize = src->chunksize * src->itemsize;
    uint8_t *buffer = malloc(buffersize);
    int ind = 0;
    while (!src->filled) {
        for (int i = 0; i < src->chunksize; ++i) {
            switch (src->itemsize) {
                case 4:
                    ((float *) buffer)[i] = (float) i;
                    break;
                case 8:
                    ((double *) buffer)[i] = (double) i;
                    break;
                default:
                    CATERVA_TEST_ERROR(CATERVA_ERR_INVALID_STORAGE);
            }
        }
        CATERVA_TEST_ERROR(caterva_array_append(ctx, src, buffer, buffersize));
        ind++;
    }
    free(buffer);

    /* Fill dest array with caterva_array_t data */
    buffersize = src->size * src->itemsize;
    buffer = malloc(buffersize);
    CATERVA_TEST_ERROR(caterva_array_to_buffer(ctx, src, buffer, buffersize));
    free(buffer);


    /* Free array  */
    CATERVA_TEST_ERROR(caterva_array_free(ctx, &src));
}

LWTEST_DATA(append) {
    caterva_context_t *ctx;
};

LWTEST_SETUP(append) {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_context_new(&cfg, &data->ctx);
}

LWTEST_TEARDOWN(append) {
    caterva_context_free(&data->ctx);
}

LWTEST_FIXTURE(append, 2_double_plainbuffer) {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 2;
    int64_t shape[] = {4, 3};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;


    test_append(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename);
}

LWTEST_FIXTURE(append, 3_float_blosc_frame) {
    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 3;
    int64_t shape[] = {4, 3, 3};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {2, 2, 2};
    bool enforceframe = true;
    char *filename = NULL;

    test_append(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename);
}

LWTEST_FIXTURE(append, 4_float_plainbuffer) {

    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 4;
    int64_t shape[] = {10, 10, 10, 10};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    test_append(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename);
}

LWTEST_FIXTURE(append, 5_float_blosc) {
    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 5;
    int64_t shape[] = {10, 11, 7, 4, 12};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {3, 2, 2, 3, 5};
    bool enforceframe = false;
    char *filename = NULL;

    test_append(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename);
}
