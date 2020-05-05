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

static void test_append_2(caterva_context_t *ctx, uint8_t itemsize, uint8_t ndim, int64_t *shape,
                          caterva_storage_backend_t backend, int64_t *chunkshape, int64_t *blockshape, bool enforceframe,
                          char *filename, double *result) {

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

    caterva_array_t *src;
    CATERVA_TEST_ERROR(caterva_array_empty(ctx, &params, &storage, &src));

    /* Fill empty caterva_array_t with blocks */
    int64_t buffersize = src->chunksize * src->itemsize;
    uint8_t *buffer = ctx->cfg->alloc(buffersize);
    int ind = 0;
    while (!src->filled) {
        memset(buffer, 0, buffersize);
        for (int i = 0; i < src->next_chunksize; ++i) {
            switch (src->itemsize) {
                case 4:
                    ((float *) buffer)[i] = (float) ind;
                    break;
                case 8:
                    ((double *) buffer)[i] = (double) ind;
                    break;
                default:
                    CATERVA_TEST_ERROR(CATERVA_ERR_INVALID_STORAGE);
            }
            ind ++;
        }
        CATERVA_TEST_ERROR(caterva_array_append(ctx, src, buffer, src->next_chunksize * src->itemsize));
    }
    ctx->cfg->free(buffer);

    /* Fill dest array with caterva_array_t data */
    buffersize = src->size * src->itemsize;
    uint8_t *buffer_dest = ctx->cfg->alloc(buffersize);
    CATERVA_TEST_ERROR(caterva_array_to_buffer(ctx, src, buffer_dest, buffersize));

    assert_buf(buffer_dest, result, src->itemsize,(size_t)10, 1e-14);
    ctx->cfg->free(buffer_dest);

    /* Free array  */
    CATERVA_TEST_ERROR(caterva_array_free(ctx, &src));
}


LWTEST_DATA(append_2) {
    caterva_context_t *ctx;
};

LWTEST_SETUP(append_2) {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_context_new(&cfg, &data->ctx);
}

LWTEST_TEARDOWN(append_2) {
    caterva_context_free(&data->ctx);
}

LWTEST_FIXTURE(append_2, 2_dim_pad_sp) {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 2;
    int64_t shape_[] = {8, 8};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;

    int64_t chunkshape_[] = {4, 4};
    int64_t blockshape_[] = {3, 3};
    double result[80] = {0,1,2,3,16,17,18,19,4,5};
    bool enforceframe = true;
    char *filename = NULL;

    test_append_2(data->ctx, itemsize, ndim, shape_, backend, chunkshape_, blockshape_, enforceframe, filename, result);
}

LWTEST_FIXTURE(append_2, 3_dim) {
    uint8_t itemsize = sizeof(double);
    const uint8_t ndim = 3;
    int64_t shape_[] = {4, 4, 5};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t pshape_[] = {3, 3, 4};
    int64_t spshape_[] = {3, 2, 3};
    double result[80] = {0,1,2,3,36,4,5,6,7,37};
    bool enforceframe = false;
    char *filename = NULL;
    test_append_2(data->ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}


LWTEST_FIXTURE(append_2, 4_dim) {
    uint8_t itemsize = sizeof(double);
    const uint8_t ndim = 4;
    int64_t shape_[] = {4, 3, 4, 6};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;

    int64_t pshape_[] = {2, 3, 2, 3};
    int64_t spshape_[] = {2, 3, 1, 3};
    double result[80] = {0,1,2,36,37,38,3,4,5,39};
    bool enforceframe = false;
    char *filename = NULL;
    test_append_2(data->ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}


LWTEST_FIXTURE(append_2, 5_dim) {
    uint8_t itemsize = sizeof(double);
    const uint8_t ndim = 5;
    int64_t shape_[] = {14, 23, 12, 11, 8};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;

    int64_t pshape_[] = {5, 12, 5, 3, 4};
    int64_t spshape_[] = {2, 4, 2, 1, 2};
    double result[80] = {0,1,2,3,3600,3601,3602,3603,4,5};
    bool enforceframe = false;
    char *filename = NULL;
    test_append_2(data->ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}
