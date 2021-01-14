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


double result0[80] = {0, 1, 2, 3, 16, 17, 18, 19, 4, 5};
double result1[80] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
double result2[80] = {0};
double result3[80] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
double result4[80] = {0};

//TODO: Fix plainbuffer

typedef struct {
    int8_t ndim;
    int64_t shape[CATERVA_MAX_DIM];
    int32_t chunkshape[CATERVA_MAX_DIM];
    int32_t blockshape[CATERVA_MAX_DIM];
    double *result;
} test_append_shapes;

CUTEST_TEST_DATA(append) {
    caterva_context_t *ctx;
};


CUTEST_TEST_SETUP(append) {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    cfg.nthreads = 2;
    cfg.compcodec = BLOSC_BLOSCLZ;
    caterva_context_new(&cfg, &data->ctx);


    // Add parametrizations
    CUTEST_PARAMETRIZE(itemsize, uint8_t, CUTEST_DATA(8));
    CUTEST_PARAMETRIZE(shapes, test_append_shapes, CUTEST_DATA(
            {2, {8, 8}, {4, 4}, {4, 4}, result0},
            {3, {4, 4, 5}, {2, 2, 5}, {1, 2, 2}, result1},
            {0, {0}, {0}, {0}, result2}, // 0-dim
            {1, {10}, {7}, {2}, result3}, // 1-idim
            {2, {20, 0}, {7, 0}, {3, 0}, result4}, // 0-shape

    ));
    CUTEST_PARAMETRIZE(backend, _test_backend, CUTEST_DATA(
            // {CATERVA_STORAGE_PLAINBUFFER, false, false}, TODO: Adapt test to this case
            {CATERVA_STORAGE_BLOSC, false, false},
            {CATERVA_STORAGE_BLOSC, true, false},
            {CATERVA_STORAGE_BLOSC, true, true},
    ));
}


CUTEST_TEST_TEST(append) {
    CUTEST_GET_PARAMETER(backend, _test_backend);
    CUTEST_GET_PARAMETER(shapes, test_append_shapes);
    CUTEST_GET_PARAMETER(itemsize, uint8_t);

    caterva_params_t params;
    params.itemsize = itemsize;
    params.ndim = shapes.ndim;
    for (int i = 0; i < params.ndim; ++i) {
        params.shape[i] = shapes.shape[i];
    }

    caterva_storage_t storage = {0};
    storage.backend = backend.backend;
    switch (backend.backend) {
        case CATERVA_STORAGE_PLAINBUFFER:
            break;
        case CATERVA_STORAGE_BLOSC:
            if (backend.persistent) {
                storage.properties.blosc.filename = "test_append.b2frame";
            } else {
                storage.properties.blosc.filename = NULL;
            }
            storage.properties.blosc.enforceframe = backend.sequential;
            for (int i = 0; i < params.ndim; ++i) {
                storage.properties.blosc.chunkshape[i] = shapes.chunkshape[i];
                storage.properties.blosc.blockshape[i] = shapes.blockshape[i];
            }
            break;
        default:
            CATERVA_TEST_ASSERT(CATERVA_ERR_INVALID_STORAGE);
    }

    caterva_array_t *src;
    CATERVA_TEST_ASSERT(caterva_array_empty(data->ctx, &params, &storage, &src));

    /* Fill empty caterva_array_t with blocks */
    int nextsize = 0;
    if (backend.backend == CATERVA_STORAGE_PLAINBUFFER) {
        nextsize = src->chunknitems;
    }
    size_t buffersize = src->chunknitems * src->itemsize;
    double *buffer = data->ctx->cfg->alloc(buffersize);
    int ind = 0;

    while (!src->filled) {
        memset(buffer, 0, buffersize);
        if (backend.backend == CATERVA_STORAGE_BLOSC) {
            nextsize = (int) src->next_chunknitems;
        }
        for (int i = 0; i < nextsize; ++i) {
            buffer[i] = (double) ind;
            ind ++;
        }
        CATERVA_TEST_ASSERT(caterva_array_append(data->ctx, src, buffer, nextsize * src->itemsize));
    }
    data->ctx->cfg->free(buffer);

    /* Fill dest array with caterva_array_t data */
    buffersize = (size_t) (src->nitems * src->itemsize);
    double *buffer_dest = data->ctx->cfg->alloc(buffersize);
    CATERVA_TEST_ASSERT(caterva_array_to_buffer(data->ctx, src, buffer_dest, buffersize));
    if (src->nitems != 0) {
        int len = src->ndim == 0 ? 1 : 10;
        for (int i = 0; i < len; ++i) {
            CUTEST_ASSERT("elements are not equals!", shapes.result[i] == buffer_dest[i]);
        }
    }

    data->ctx->cfg->free(buffer_dest);

    /* Free array  */
    CATERVA_TEST_ASSERT(caterva_array_free(data->ctx, &src));
    return 0;
}

CUTEST_TEST_TEARDOWN(append) {
    caterva_context_free(&data->ctx);
}

int main() {
    CUTEST_TEST_RUN(append);
}

