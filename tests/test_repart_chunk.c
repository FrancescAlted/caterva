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
#include "caterva_blosc.h"

static void test_repart_chunk(caterva_context_t *ctx, uint8_t itemsize, caterva_storage_backend_t backend, uint8_t ndim,
                              int64_t *shape_, int64_t *chunkshape, int64_t *blockshape, bool enforceframe,
                              char *filename, double result[]) {


    caterva_params_t params;
    params.itemsize = itemsize;
    params.ndim = ndim;
    for (int i = 0; i < ndim; ++i) {
        params.shape[i] = shape_[i];
    }
    caterva_storage_t storage = {0};
    storage.properties.blosc.filename = filename;
    storage.properties.blosc.enforceframe = enforceframe;
    storage.backend = backend;
    switch (backend) {
        case CATERVA_STORAGE_PLAINBUFFER:
            break;
        case CATERVA_STORAGE_BLOSC:
            for (int i = 0; i < ndim; ++i) {
                storage.properties.blosc.chunkshape[i] = chunkshape[i];
                storage.properties.blosc.blockshape[i] = blockshape[i];
            }
            break;
        default:
            CATERVA_TEST_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }
    caterva_array_t *carr;
    caterva_array_empty(ctx, &params, &storage, &carr);

    /* Fill empty caterva_array_t with blocks */
    int size_src = carr->chunksize * itemsize;
    int size_dest = carr->extchunksize * itemsize;
    double *buffer_src = (double *) ctx->cfg->alloc(size_src);
    double *buffer_dest = (double *) ctx->cfg->alloc(size_dest);
    for (int i = 0; i < carr->chunksize; ++i) {
        buffer_src[i] =  (double) i;
    }
    int res = caterva_blosc_array_repart_chunk((int8_t *) buffer_dest, size_dest, buffer_src, size_src, carr);
    if (res != 0) {
        printf("Error code : %d\n", res);
    }

    assert_buf((uint8_t*) buffer_dest, (uint8_t*) result, itemsize, (size_t)carr->extchunksize, 1e-8);   // tam epsize*typesize????????????????
    ctx->cfg->free(buffer_src);
    ctx->cfg->free(buffer_dest);
    CATERVA_TEST_ERROR(caterva_array_free(ctx, &carr));
}

LWTEST_DATA(repart_chunk) {
    caterva_context_t *ctx;
};

LWTEST_SETUP(repart_chunk) {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_context_new(&cfg, &data->ctx);
}

LWTEST_TEARDOWN(repart_chunk) {
    caterva_context_free(&data->ctx);
}


LWTEST_FIXTURE(repart_chunk, 2_dim) {
    const uint8_t ndim = 2;
    uint8_t itemsize = sizeof(double);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = true;
    char *filename = NULL;
    int64_t shape_[] = {8, 16};
    int64_t pshape_[] = {4, 8};
    int64_t spshape_[] = {2, 4};
    double result[1024] = {0, 1, 2, 3, 8, 9, 10, 11, 4, 5, 6, 7, 12, 13,
                           14, 15, 16, 17, 18, 19, 24, 25, 26, 27, 20, 21, 22, 23, 28, 29, 30, 31};

    test_repart_chunk(data->ctx, itemsize, backend, ndim, shape_, pshape_, spshape_, enforceframe, filename, result);
}

LWTEST_FIXTURE(repart_chunk, 2_dim_pad) {
    const uint8_t ndim = 2;
    uint8_t itemsize = sizeof(double);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = true;
    char *filename = NULL;
    int64_t shape__[] = {8, 16};
    int64_t pshape__[] = {4, 8};
    int64_t spshape__[] = {2, 3};
    double result[1024] = {0, 1, 2, 8, 9, 10, 3, 4, 5, 11, 12, 13, 6, 7, 0, 14, 15, 0,
                           16, 17, 18, 24, 25, 26, 19, 20, 21, 27, 28, 29, 22, 23, 0, 30, 31, 0};
    test_repart_chunk(data->ctx, itemsize, backend, ndim, shape__, pshape__, spshape__, enforceframe, filename, result);
}


LWTEST_FIXTURE(repart_chunk, 3_dim) {
    const uint8_t ndim = 3;
    uint8_t itemsize = sizeof(double);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = true;
    char *filename = NULL;
    int64_t shape_[] = {4, 3, 2};
    int64_t pshape_[] = {2, 2, 2};
    int64_t spshape_[] = {1, 2, 1};
    double result[1024] = {0, 2, 1, 3, 4, 6, 5, 7};
    test_repart_chunk(data->ctx, itemsize, backend, ndim, shape_, pshape_, spshape_, enforceframe, filename, result);
}

LWTEST_FIXTURE(repart_chunk, 3_dim_pad) {
    const uint8_t ndim = 3;
    uint8_t itemsize = sizeof(double);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = true;
    char *filename = NULL;
    int64_t shape_[] = {4, 3, 2};
    int64_t pshape_[] = {2, 3, 2};
    int64_t spshape_[] = {1, 2, 1};
    double result[1024] = {0, 2, 1, 3, 4, 0, 5, 0, 6, 8, 7, 9, 10, 0, 11, 0};
    test_repart_chunk(data->ctx, itemsize, backend, ndim, shape_, pshape_, spshape_, enforceframe, filename, result);
}

LWTEST_FIXTURE(repart_chunk, 3_dim_2) {
    const uint8_t ndim = 3;
    uint8_t itemsize = sizeof(double);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = true;
    char *filename = NULL;
    int64_t shape_[] = {5, 6, 3};
    int64_t pshape_[] = {4, 3, 3};
    int64_t spshape_[] = {3, 3, 2};
    double result[1024] = {0.000000, 1.000000, 3.000000, 4.000000, 6.000000, 7.000000, 9.000000, 10.000000, 12.000000,
                           13.000000, 15.000000, 16.000000, 18.000000, 19.000000, 21.000000, 22.000000, 24.000000, 25.000000,
                           2.000000, 0.000000, 5.000000, 0.000000, 8.000000, 0.000000, 11.000000, 0.000000, 14.000000,
                           0.000000, 17.000000, 0.000000, 20.000000, 0.000000, 23.000000, 0.000000, 26.000000, 0.000000,
                           27, 28, 30, 31, 33, 34, 0,0,0,0,0,0,0,0,0,0,0,0, 29, 0, 32, 0, 35, 0, 0,0,0,0,0,0,0,0,0,0,0,0};
    test_repart_chunk(data->ctx, itemsize, backend, ndim, shape_, pshape_, spshape_, enforceframe, filename, result);
}

LWTEST_FIXTURE(repart_chunk, 4_dim) {
    const uint8_t ndim = 4;
    uint8_t itemsize = sizeof(double);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = true;
    char *filename = NULL;
    int64_t shape_[] = {4, 3, 5, 4};
    int64_t pshape_[] = {2, 2, 2, 2};
    int64_t spshape_[] = {1, 2, 1, 1};
    double result[1024] = {0, 4, 1, 5, 2, 6, 3, 7, 8, 12, 9, 13, 10, 14, 11, 15};
    test_repart_chunk(data->ctx, itemsize, backend, ndim, shape_, pshape_, spshape_, enforceframe, filename, result);
}

LWTEST_FIXTURE(repart_chunk, 4_dim_pad) {
    const uint8_t ndim = 4;
    uint8_t itemsize = sizeof(double);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = true;
    char *filename = NULL;
    int64_t shape_[] = {4, 3, 2, 6};
    int64_t pshape_[] = {2, 3, 2, 2};
    int64_t spshape_[] = {1, 2, 1, 1};
    double result[1024] = {0, 4, 1, 5, 2, 6, 3, 7, 8, 0, 9, 0, 10, 0, 11, 0,
                           12, 16, 13, 17, 14, 18, 15, 19, 20, 0, 21, 0, 22, 0, 23, 0};
    test_repart_chunk(data->ctx, itemsize, backend, ndim, shape_, pshape_, spshape_, enforceframe, filename, result);
}


LWTEST_FIXTURE(repart_chunk, 5_dim_blosc) {
    const uint8_t ndim = 5;
    uint8_t itemsize = sizeof(double);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = true;
    char *filename = NULL;
    int64_t shape_[] = {14, 23, 12, 11, 8};
    int64_t pshape_[] = {2, 2, 2, 2, 2};
    int64_t spshape_[] = {1, 1, 1, 1, 1};
    double result[1024];
    for(double i = 0.0; i < 32; i++){
        result[(int) i] = i;
    }
    test_repart_chunk(data->ctx, itemsize, backend, ndim, shape_, pshape_, spshape_, enforceframe, filename, result);
}

LWTEST_FIXTURE(repart_chunk, 6_dim_pad) {
    const uint8_t ndim = 6;
    uint8_t itemsize = sizeof(double );
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = true;
    char *filename = NULL;
    int64_t shape_[] = {2, 3, 2, 3, 3, 6};
    int64_t pshape_[] = {1, 2, 1, 2, 3, 5};
    int64_t spshape_[] = {1, 1, 1, 2, 2, 5};
    double result[1024] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 10,
                            11, 12, 13, 14, 0, 0, 0, 0, 0, 25, 26, 27, 28, 29, 0, 0, 0, 0, 0, 30, 31,
                            32, 33, 34, 35, 36, 37, 38, 39, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
                            40, 41, 42, 43, 44, 0, 0, 0, 0, 0, 55, 56, 57, 58, 59, 0, 0, 0, 0, 0};
    test_repart_chunk(data->ctx, itemsize, backend, ndim, shape_, pshape_, spshape_, enforceframe, filename, result);
}

LWTEST_FIXTURE(repart_chunk, 7_dim_pad) {
    const uint8_t ndim = 7;
    uint8_t itemsize = sizeof(double );
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = true;
    char *filename = NULL;
    int64_t shape_[] = {2, 3, 2, 3, 3, 2, 5};
    int64_t pshape_[] = {1, 2, 1, 2, 3, 2, 4};
    int64_t spshape_[] = {1, 2, 1, 2, 2, 1, 3};
    double result[1024] = { 0, 1, 2, 8, 9, 10, 24, 25, 26, 32, 33, 34, 48, 49, 50, 56, 57, 58, 72, 73, 74, 80,
                            81, 82, 3, 0, 0, 11, 0, 0, 27, 0, 0, 35, 0, 0, 51, 0, 0, 59, 0, 0, 75, 0, 0, 83, 0,
                            0, 4, 5, 6, 12, 13, 14, 28, 29, 30, 36, 37, 38, 52, 53, 54, 60, 61, 62, 76, 77, 78,
                            84, 85, 86, 7, 0, 0, 15, 0, 0, 31, 0, 0, 39, 0, 0, 55, 0, 0, 63, 0, 0, 79, 0, 0, 87,
                            0, 0, 16, 17, 18, 0, 0, 0, 40, 41, 42, 0, 0, 0, 64, 65, 66, 0, 0, 0, 88, 89, 90, 0,
                            0, 0, 19, 0, 0, 0, 0, 0, 43, 0, 0, 0, 0, 0, 67, 0, 0, 0, 0, 0, 91, 0, 0, 0, 0, 0, 20,
                            21, 22, 0, 0, 0, 44, 45, 46, 0, 0, 0, 68, 69, 70, 0, 0, 0, 92, 93, 94, 0, 0, 0, 23,
                            0, 0, 0, 0, 0, 47, 0, 0, 0, 0, 0, 71, 0, 0, 0, 0, 0, 95, 0, 0, 0, 0, 0};
    test_repart_chunk(data->ctx, itemsize, backend, ndim, shape_, pshape_, spshape_, enforceframe, filename, result);
}

LWTEST_FIXTURE(repart_chunk, 8_dim_float) {
    const uint8_t ndim = 8;
    uint8_t itemsize = sizeof(double );
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = false;
    char *filename = NULL;
    int64_t shape_[] = {1, 2, 1, 3, 3, 2, 2, 3};
    int64_t pshape_[] = {1, 2, 1, 2, 3, 2, 2, 3};
    int64_t spshape_[] = {1, 2, 1, 2, 2, 1, 2, 3};
    double result[1024] = {0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17, 36, 37, 38, 39, 40, 41, 48, 49, 50, 51, 52, 53, 72, 73, 74, 75, 76, 77, 84, 85, 86, 87, 88, 89, 108, 109, 110, 111, 112, 113, 120, 121, 122, 123, 124, 125, 6, 7, 8, 9, 10, 11, 18, 19, 20, 21, 22, 23, 42, 43, 44, 45, 46, 47, 54, 55, 56, 57, 58, 59, 78, 79, 80, 81, 82, 83, 90, 91, 92, 93, 94, 95, 114, 115, 116, 117, 118, 119, 126, 127, 128, 129, 130, 131, 24, 25, 26, 27, 28, 29, 0, 0, 0, 0, 0, 0, 60, 61, 62, 63, 64, 65, 0, 0, 0, 0, 0, 0, 96, 97, 98, 99, 100, 101, 0, 0, 0, 0, 0, 0, 132, 133, 134, 135, 136, 137, 0, 0, 0, 0, 0, 0, 30, 31, 32, 33, 34, 35, 0, 0, 0, 0, 0, 0, 66, 67, 68, 69, 70, 71, 0, 0, 0, 0, 0, 0, 102, 103, 104, 105, 106, 107, 0, 0, 0, 0, 0, 0, 138, 139, 140, 141, 142, 143, 0, 0, 0, 0, 0, 0};
    test_repart_chunk(data->ctx, itemsize, backend, ndim, shape_, pshape_, spshape_, enforceframe, filename, result);
}