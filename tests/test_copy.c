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

typedef struct {
    int8_t ndim;
    int64_t shape[CATERVA_MAX_DIM];
    int32_t chunkshape[CATERVA_MAX_DIM];
    int32_t blockshape[CATERVA_MAX_DIM];
    int32_t chunkshape2[CATERVA_MAX_DIM];
    int32_t blockshape2[CATERVA_MAX_DIM];
} test_squeeze_shapes_t;


CUTEST_TEST_DATA(copy) {
    caterva_context_t *ctx;
};


CUTEST_TEST_SETUP(copy) {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    cfg.nthreads = 2;
    cfg.compcodec = BLOSC_BLOSCLZ;
    caterva_context_new(&cfg, &data->ctx);

    // Add parametrizations
    CUTEST_PARAMETRIZE(itemsize, uint8_t, CUTEST_DATA(1, 2, 4, 8));
    CUTEST_PARAMETRIZE(shapes, test_squeeze_shapes_t, CUTEST_DATA(
            {2, {100, 100}, {20, 20}, {10, 10},
                {20, 20}, {10, 10}},
            {3, {100, 55, 123}, {31, 5, 22}, {4, 4, 4},
                {50, 15, 20}, {10, 4, 4}},
            {3, {100, 0, 12}, {31, 0, 12}, {10, 0, 12},
                {50, 0, 12}, {25, 0, 6}},
            {4, {25, 60, 31, 12}, {12, 20, 20, 10}, {5, 5, 5, 10},
                {25, 20, 20, 10}, {5, 5, 5, 10}},
            {5, {1, 1, 1024, 1, 1}, {1, 1, 500, 1, 1}, {1, 1, 200, 1, 1},
                {1, 1, 300, 1, 1}, {1, 1, 50, 1, 1}},
            {6, {5, 1, 100, 3, 1, 2}, {5, 1, 50, 2, 1, 2}, {2, 1, 20, 2, 1, 2},
                {4, 1, 50, 2, 1, 1}, {2, 1, 20, 2, 1, 1}},
    ));
    CUTEST_PARAMETRIZE(backend, _test_backend, CUTEST_DATA(
            {CATERVA_STORAGE_PLAINBUFFER, false, false},
            {CATERVA_STORAGE_BLOSC, false, false},
            {CATERVA_STORAGE_BLOSC, true, false},
            {CATERVA_STORAGE_BLOSC, true, true},
    ));

    CUTEST_PARAMETRIZE(backend2, _test_backend, CUTEST_DATA(
            {CATERVA_STORAGE_PLAINBUFFER, false, false},
            {CATERVA_STORAGE_BLOSC, false, false},
            {CATERVA_STORAGE_BLOSC, true, false},
            {CATERVA_STORAGE_BLOSC, true, true},
    ));
}

CUTEST_TEST_TEST(copy) {
    CUTEST_GET_PARAMETER(backend, _test_backend);
    CUTEST_GET_PARAMETER(shapes, test_squeeze_shapes_t);
    CUTEST_GET_PARAMETER(backend2, _test_backend);
    CUTEST_GET_PARAMETER(itemsize, uint8_t);

    caterva_params_t params;
    params.itemsize = itemsize;
    params.ndim = shapes.ndim;
    for (int i = 0; i < shapes.ndim; ++i) {
        params.shape[i] = shapes.shape[i];
    }

    double datatoserialize = 8.34;

    caterva_storage_t storage = {0};
    storage.backend = backend.backend;
    switch (storage.backend) {
        case CATERVA_STORAGE_PLAINBUFFER:
            break;
        case CATERVA_STORAGE_BLOSC:
            if (backend.persistent) {
                storage.properties.blosc.urlpath = "test_copy.b2frame";
            } else {
                storage.properties.blosc.urlpath = NULL;
            }
            storage.properties.blosc.enforceframe = backend.sequential;
            for (int i = 0; i < params.ndim; ++i) {
                storage.properties.blosc.chunkshape[i] = shapes.chunkshape[i];
                storage.properties.blosc.blockshape[i] = shapes.blockshape[i];
            }
            storage.properties.blosc.nmetalayers = 1;
            storage.properties.blosc.metalayers[0].name = "random";
            storage.properties.blosc.metalayers[0].sdata = (uint8_t *) &datatoserialize;
            storage.properties.blosc.metalayers[0].size = 8;

            break;
        default:
            CATERVA_TEST_ASSERT(CATERVA_ERR_INVALID_STORAGE);
    }

    /* Create original data */
    size_t buffersize = itemsize;
    for (int i = 0; i < params.ndim; ++i) {
        buffersize *= (size_t) params.shape[i];
    }
    uint8_t *buffer = malloc(buffersize);
    CUTEST_ASSERT("Buffer filled incorrectly", fill_buf(buffer, itemsize, (buffersize / itemsize)));

    /* Create caterva_array_t with original data */
    caterva_array_t *src;
    CATERVA_TEST_ASSERT(caterva_array_from_buffer(data->ctx, buffer, buffersize, &params, &storage,
                                                  &src));

    /* Assert the metalayers creation */
    if (storage.backend == CATERVA_STORAGE_BLOSC) {
        if (blosc2_has_metalayer(src->sc, "random") < 0) {
            CATERVA_TEST_ASSERT(CATERVA_ERR_BLOSC_FAILED);
        }
        double *serializeddata;
        uint32_t len;
        blosc2_get_metalayer(src->sc, "random", (uint8_t **) &serializeddata, &len);
        if (*serializeddata != datatoserialize) {
            CATERVA_TEST_ASSERT(CATERVA_ERR_BLOSC_FAILED);
        }
        free(serializeddata);
    }

    /* Create storage for dest container */
    caterva_storage_t storage2 = {0};
    storage2.backend = backend2.backend;
    switch (storage2.backend) {
        case CATERVA_STORAGE_PLAINBUFFER:
            break;
        case CATERVA_STORAGE_BLOSC:
            if (backend2.persistent) {
                storage2.properties.blosc.urlpath = "test_copy2.b2frame";
            } else {
                storage2.properties.blosc.urlpath = NULL;
            }
            storage2.properties.blosc.enforceframe = backend2.sequential;
            for (int i = 0; i < shapes.ndim; ++i) {
                storage2.properties.blosc.chunkshape[i] = shapes.chunkshape2[i];
                storage2.properties.blosc.blockshape[i] = shapes.blockshape2[i];
            }
            break;
        default:
            CATERVA_TEST_ASSERT(CATERVA_ERR_INVALID_STORAGE);
    }

    caterva_array_t *dest;
    CATERVA_TEST_ASSERT(caterva_array_copy(data->ctx, src, &storage2, &dest));

    uint8_t *buffer_dest = malloc(buffersize);
    CATERVA_TEST_ASSERT(caterva_array_to_buffer(data->ctx, dest, buffer_dest, buffersize));

    /* Testing */
    CATERVA_TEST_ASSERT_BUFFER(buffer, buffer_dest, (int) buffersize);

    /* Free mallocs */
    free(buffer);
    free(buffer_dest);
    CATERVA_TEST_ASSERT(caterva_array_free(data->ctx, &src));
    CATERVA_TEST_ASSERT(caterva_array_free(data->ctx, &dest));

    return 0;
}

CUTEST_TEST_TEARDOWN(copy) {
    caterva_context_free(&data->ctx);
}

int main() {
    CUTEST_TEST_RUN(copy);
}
