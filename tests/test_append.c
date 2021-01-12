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
    ));
    CUTEST_PARAMETRIZE(backend, _test_backend, CUTEST_DATA(
            {CATERVA_STORAGE_PLAINBUFFER, false, false},
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

    for (int i = 0; i < 10; ++i) {
        printf("%f - %f\n", shapes.result[i], buffer_dest[i]);
        CUTEST_ASSERT("elements are not equals!", shapes.result[i] == buffer_dest[i]);
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

/*
static char* append_2_dim() {

    bool enforceframe = true;
    char *filename = NULL;

    return test_append(ctx, itemsize, ndim, shape_, backend, chunkshape_, blockshape_, enforceframe, filename, result);
}

static char* append_3_dim_plain() {
    uint8_t itemsize = sizeof(double);
    const uint8_t ndim = 3;

    bool enforceframe = false;
    char *filename = NULL;

    return test_append(ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}

static char* append_4_dim() {
    uint8_t itemsize = sizeof(double);
    const uint8_t ndim = 4;
    int64_t shape_[] = {4, 3, 4, 6};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;

    uint32_t pshape_[] = {2, 3, 2, 3};
    uint32_t spshape_[] = {2, 3, 1, 3};
    double result[80] = {0, 1, 2, 36, 37, 38, 3, 4, 5, 39};
    bool enforceframe = false;
    char *filename = NULL;

    return test_append(ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}

static char* append_5_dim_float() {
    uint8_t itemsize = sizeof(float);
    const uint8_t ndim = 5;
    int64_t shape_[] = {14, 23, 12, 11, 8};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;

    uint32_t pshape_[] = {5, 12, 5, 3, 4};
    uint32_t spshape_[] = {2, 5, 2, 2, 3};
    float result[80] = {0, 1, 2, 3, 3600, 3601, 3602, 3603, 4, 5};
    bool enforceframe = false;
    char *filename = NULL;
    return test_append(ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}

static char* append_6_dim() {
    uint8_t itemsize = sizeof(double);
    const uint8_t ndim = 6;
    int64_t shape_[] = {10, 4, 9, 11, 8, 8};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;

    uint32_t pshape_[] = {7, 3, 5, 6, 5, 3};
    uint32_t spshape_[] = {2, 4, 2, 1, 2, 2};
    double result[80] = {0, 1, 2, 9450, 9451, 9452, 18900, 18901, 3, 4};
    bool enforceframe = false;
    char *filename = NULL;
    return test_append(ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}

static char* append_7_dim_float() {
    uint8_t itemsize = sizeof(float);
    const uint8_t ndim = 7;
    int64_t shape_[] = {5, 3, 2, 4, 3, 2, 7};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;

    uint32_t pshape_[] = {5, 2, 2, 3, 2, 2, 3};
    uint32_t spshape_[] = {4, 2, 1, 2, 1, 2, 2};
    float result[80] = {0, 1, 2, 720, 721, 722, 1440, 3, 4 ,5};
    bool enforceframe = false;
    char *filename = NULL;
    return test_append(ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}

static char* append_8_dim_uint8() {
    uint8_t itemsize = sizeof(uint8_t);
    const uint8_t ndim = 8;
    int64_t shape_[] = {5, 3, 2, 1, 4, 3, 2, 7};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;

    uint32_t pshape_[] = {2, 2, 2, 1, 3, 1, 2, 4};
    uint32_t spshape_[] = {2, 2, 2, 1, 3, 1, 2, 4};
    uint8_t result[80] = {0, 1, 2, 3, 192, 193, 194, 4, 5, 6};
    bool enforceframe = false;
    char *filename = NULL;
    return test_append(ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}

static char* append_8_dim_uint16() {
    uint8_t itemsize = sizeof(uint16_t);
    const uint8_t ndim = 8;
    int64_t shape_[] = {5, 3, 2, 3, 4, 3, 3, 7};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;

    uint32_t pshape_[] = {2, 3, 2, 2, 3, 1, 2, 4};
    uint32_t spshape_[] = {2, 2, 2, 2, 2, 1, 2, 4};
    uint16_t result[80] = {0, 1, 2, 3, 576, 577, 578, 4, 5, 6};
    bool enforceframe = false;
    char *filename = NULL;
    return test_append(ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}

 */