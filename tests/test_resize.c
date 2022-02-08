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


CUTEST_TEST_DATA(resize_shape) {
    caterva_ctx_t *ctx;
};


CUTEST_TEST_SETUP(resize_shape) {
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
            {1, {5}, {3}, {2}, {10}}, // extend only
            {2, {20, 5}, {7, 5}, {3, 3}, {20, 20}}, // extend only
            {2, {20, 10}, {7, 5}, {3, 5}, {10, 10}}, // shrink only
            {2, {14, 10}, {8, 5}, {2, 2}, {10, 5}}, // shrink only
            {3, {12, 10, 14}, {3, 5, 9}, {3, 4, 4}, {10, 15, 14}}, // shrink and extend
            {3, {10, 21, 30}, {8, 7, 15}, {5, 5, 10}, {10, 13, 10}}, // shrink and extend
            {2, {50, 50}, {25, 13}, {8, 8}, {49, 51}}, // shrink and extend
            {2, {143, 41}, {18, 13}, {7, 7}, {50, 50}}, // shrink and extend
            {4, {10, 10, 5, 5}, {5, 7, 3, 3}, {2, 2, 1, 1}, {11, 20, 2, 2}}, // shrink and extend

    ));
}

CUTEST_TEST_TEST(resize_shape) {
    CUTEST_GET_PARAMETER(backend, _test_backend);
    CUTEST_GET_PARAMETER(shapes, test_shapes_t);
    CUTEST_GET_PARAMETER(itemsize, uint8_t);

    char *urlpath = "test_resize_shape.b2frame";
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

    // Create dest buffer, get shrinked shape  and extended shape
    int64_t buffersize = itemsize;
    int64_t shrink_shape[CATERVA_MAX_DIM] = {0};
    int64_t shrink_size = itemsize;
    int64_t extend_shape[CATERVA_MAX_DIM] = {0};
    int64_t start_extend_slice[CATERVA_MAX_DIM] = {0};
    int64_t extend_size = itemsize;
    bool only_shrink = true;
    for (int i = 0; i < params.ndim; ++i) {
        if (shapes.newshape[i] <= shapes.shape[i]) {
            shrink_shape[i] = shapes.newshape[i];
            extend_shape[i] = shapes.newshape[i];
            start_extend_slice[i] = 0;
        } else {
            shrink_shape[i] = shapes.shape[i];
            extend_shape[i] = (shapes.newshape[i]-shapes.shape[i]);
            start_extend_slice[i] = shapes.shape[i];
            only_shrink = false;
        }
        buffersize *= shapes.shape[i];
        shrink_size *= shrink_shape[i];
        extend_size *= extend_shape[i];
    }
    uint8_t *buffer = data->ctx->cfg->alloc(buffersize);
    CUTEST_ASSERT("Buffer filled incorrectly", fill_buf(buffer, itemsize, buffersize / itemsize));
    /* Create caterva_array_t with original data */
    caterva_array_t *src;
    CATERVA_ERROR(caterva_from_buffer(data->ctx, buffer, buffersize, &params, &storage, &src));

    // Get shrink slice previous to resize
    uint8_t *original_buffer = data->ctx->cfg->alloc(shrink_size);
    int64_t start_shape[CATERVA_MAX_DIM] = {0};

    CATERVA_ERROR(caterva_get_slice_buffer(data->ctx, src, start_shape, shrink_shape,
                                           original_buffer, shrink_shape,
                                           shrink_size));

    CATERVA_ERROR(caterva_resize(data->ctx, src, shapes.newshape));


    uint8_t *shrink_buffer = data->ctx->cfg->alloc((size_t) shrink_size);
    /* Fill extend buffer with a slice from the new chunks*/
    CATERVA_ERROR(caterva_get_slice_buffer(data->ctx, src, start_shape, shrink_shape,
                                           shrink_buffer, shrink_shape,
                                           shrink_size));
    for (uint64_t i = 0; i < (uint64_t) shrink_size / itemsize; ++i) {
        switch (itemsize) {
            case 8:
                CUTEST_ASSERT("Elements are not equal!",
                              ((uint64_t *) original_buffer)[i] == ((uint64_t *) shrink_buffer)[i]);
                break;
            case 4:
                CUTEST_ASSERT("Elements are not equal!",
                              ((uint32_t *) original_buffer)[i] == ((uint32_t *) shrink_buffer)[i]);
                break;
            case 2:
                CUTEST_ASSERT("Elements are not equal!",
                              ((uint16_t *) original_buffer)[i] == ((uint16_t *) shrink_buffer)[i]);
                break;
            case 1:
                CUTEST_ASSERT("Elements are not equal!",
                              ((uint8_t *) original_buffer)[i] == ((uint8_t *) shrink_buffer)[i]);
                break;
            default:
                CATERVA_TEST_ASSERT(CATERVA_ERR_INVALID_ARGUMENT);
        }
    }

    if (!only_shrink) {
        uint8_t *extend_buffer = data->ctx->cfg->alloc((size_t) extend_size);
        /* Fill extend buffer with a slice from the new chunks*/
        CATERVA_ERROR(caterva_get_slice_buffer(data->ctx, src, start_extend_slice, shapes.newshape,
                                               extend_buffer, extend_shape,
                                               extend_size));
        for (uint64_t i = 0; i < (uint64_t) extend_size / itemsize; ++i) {
            switch (itemsize) {
                case 8:
                    CUTEST_ASSERT("Elements are not equal!",
                                  (uint64_t) 0 == ((uint64_t *) extend_buffer)[i]);
                    break;
                case 4:
                    CUTEST_ASSERT("Elements are not equal!",
                                  (uint32_t) 0 == ((uint32_t *) extend_buffer)[i]);
                    break;
                case 2:
                    CUTEST_ASSERT("Elements are not equal!",
                                  (uint16_t) 0 == ((uint16_t *) extend_buffer)[i]);
                    break;
                case 1:
                    CUTEST_ASSERT("Elements are not equal!",
                                  (uint8_t) 0 == ((uint8_t *) extend_buffer)[i]);
                    break;
                default:
                    CATERVA_TEST_ASSERT(CATERVA_ERR_INVALID_ARGUMENT);
            }
        }
        data->ctx->cfg->free(extend_buffer);
    }

    /* Free mallocs */
    data->ctx->cfg->free(buffer);
    data->ctx->cfg->free(original_buffer);
    data->ctx->cfg->free(shrink_buffer);

    CATERVA_TEST_ASSERT(caterva_free(data->ctx, &src));
    caterva_remove(data->ctx, urlpath);

    return 0;
}

CUTEST_TEST_TEARDOWN(resize_shape) {
    caterva_ctx_free(&data->ctx);
}

int main() {
    CUTEST_TEST_RUN(resize_shape);
}
