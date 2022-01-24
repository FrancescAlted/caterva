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

typedef struct {
    int8_t ndim;
    int64_t shape[CATERVA_MAX_DIM];
    int32_t chunkshape[CATERVA_MAX_DIM];
    int32_t blockshape[CATERVA_MAX_DIM];
    int64_t newshape[CATERVA_MAX_DIM];
} test_shapes_t;


CUTEST_TEST_DATA(extend_shape) {
    caterva_ctx_t *ctx;
};


CUTEST_TEST_SETUP(extend_shape) {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    cfg.nthreads = 2;
    cfg.compcodec = BLOSC_ZSTD;
    caterva_ctx_new(&cfg, &data->ctx);

    // Add parametrizations
    CUTEST_PARAMETRIZE(itemsize, uint8_t, CUTEST_DATA(
          1,
           2,
           4,
                                              8,
    ));

    CUTEST_PARAMETRIZE(backend, _test_backend, CUTEST_DATA(
            {false, false},
           {true, false},
            {true, true},
            {false, true},
    ));


    CUTEST_PARAMETRIZE(shapes, test_shapes_t, CUTEST_DATA(
            {1, {5}, {3}, {2}, {10}},
            {2, {20, 5}, {7, 5}, {3, 3}, {20, 20}},
            {2, {20, 10}, {7, 5}, {3, 5}, {30, 10}},
            {2, {14, 10}, {8, 5}, {2, 2}, {20, 15}},
            {3, {12, 10, 14}, {3, 5, 9}, {3, 4, 4}, {15, 18, 14}},
            {3, {10, 21, 30}, {8, 7, 15}, {5, 5, 10}, {10, 40, 30}},
            {2, {50, 50}, {25, 13}, {8, 8}, {50, 51}},
            {2, {143, 41}, {18, 13}, {7, 7}, {145, 50}},
            {4, {10, 10, 5, 5}, {5, 7, 3, 3}, {2, 2, 1, 1}, {10, 20, 5, 10}},

    ));
}

CUTEST_TEST_TEST(extend_shape) {
    CUTEST_GET_PARAMETER(backend, _test_backend);
    CUTEST_GET_PARAMETER(shapes, test_shapes_t);
    CUTEST_GET_PARAMETER(itemsize, uint8_t);

    char *urlpath = "test_extend_shape.b2frame";
    caterva_remove(data->ctx, urlpath);

    caterva_params_t params;
    params.itemsize = itemsize;
    params.ndim = shapes.ndim;
    for (int i = 0; i < params.ndim; ++i) {
        params.shape[i] = shapes.shape[i];
    }

    caterva_storage_t storage = {0};
    if (backend.persistent) {
        storage.urlpath = urlpath;
    }
    storage.sequencial = backend.sequential;
    for (int i = 0; i < params.ndim; ++i) {
        storage.chunkshape[i] = shapes.chunkshape[i];
        storage.blockshape[i] = shapes.blockshape[i];
    }

    /* Create dest buffer */
    int64_t buffersize = itemsize;
    for (int i = 0; i < params.ndim; ++i) {
        buffersize *= shapes.shape[i];
    }
    uint8_t *buffer = data->ctx->cfg->alloc(buffersize);
    CUTEST_ASSERT("Buffer filled incorrectly", fill_buf(buffer, itemsize, buffersize / itemsize));
    /* Create caterva_array_t with original data */
    caterva_array_t *src;
    CATERVA_ERROR(caterva_from_buffer(data->ctx, buffer, buffersize, &params, &storage, &src));

    CATERVA_ERROR(caterva_resize(src, shapes.newshape));

    int64_t shape[CATERVA_MAX_DIM] = {0};
    int64_t destsize = itemsize;
    for (int i = 0; i < params.ndim; ++i) {
        shape[i] = (shapes.newshape[i]-shapes.shape[i]);
        destsize *= shape[i];
    }
    uint8_t *destbuffer = data->ctx->cfg->alloc((size_t) destsize);
    /* Fill dest buffer with a slice from the new chunks*/
   CATERVA_ERROR(caterva_get_slice_buffer(data->ctx, src, shapes.shape, shapes.newshape,
                                                 destbuffer, shape,
                                                 destsize));

    for (uint64_t i = 0; i < (uint64_t) destsize / itemsize; ++i) {
        switch (itemsize) {
            case 8:
                CUTEST_ASSERT("Elements are not equal!",
                           (uint64_t) 0 == ((uint64_t *) destbuffer)[i]);
                break;
            case 4:
                CUTEST_ASSERT("Elements are not equal!",
                             (uint32_t) 0 == ((uint32_t *) destbuffer)[i]);
                break;
            case 2:
                CUTEST_ASSERT("Elements are not equal!",
                              (uint16_t) 0 == ((uint16_t *) destbuffer)[i]);
                break;
            case 1:
                CUTEST_ASSERT("Elements are not equal!",
                              (uint8_t) 0 == ((uint8_t *) destbuffer)[i]);
                break;
            default:
                CATERVA_TEST_ASSERT(CATERVA_ERR_INVALID_ARGUMENT);
        }
    }

    /* Free mallocs */
    data->ctx->cfg->free(buffer);
    data->ctx->cfg->free(destbuffer);
    CATERVA_TEST_ASSERT(caterva_free(data->ctx, &src));
    caterva_remove(data->ctx, urlpath);

    return 0;
}

CUTEST_TEST_TEARDOWN(extend_shape) {
    caterva_ctx_free(&data->ctx);
}

int main() {
    CUTEST_TEST_RUN(extend_shape);
}
