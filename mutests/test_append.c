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

static char* test_append(caterva_context_t *ctx,
                        uint8_t itemsize,
                        uint8_t ndim,
                        int64_t *shape,
                        caterva_storage_backend_t backend,
                        int64_t *chunkshape,
                        int64_t *blockshape,
                        bool enforceframe,
                        char *filename,
                        void *result) {

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
            MU_ASSERT_CATERVA(CATERVA_ERR_INVALID_STORAGE);
    }

    caterva_array_t *src;
    MU_ASSERT_CATERVA(caterva_array_empty(ctx, &params, &storage, &src));

    /* Fill empty caterva_array_t with blocks */
    int nextsize = 0;
    if (backend == CATERVA_STORAGE_PLAINBUFFER) {
        nextsize = src->chunksize;
    }
    int64_t buffersize = src->chunksize * src->itemsize;
    uint8_t *buffer = ctx->cfg->alloc(buffersize);
    int ind = 0;

    while (!src->filled) {
        memset(buffer, 0, buffersize);
        if (backend == CATERVA_STORAGE_BLOSC) {
            nextsize = src->next_chunksize;
        }
        for (int i = 0; i < nextsize; ++i) {
            switch (src->itemsize) {
                case 1:
                    ((uint8_t *) buffer)[i] = (uint8_t) ind;
                    break;
                case 2:
                    ((uint16_t *) buffer)[i] = (uint16_t) ind;
                    break;
                case 4:
                    ((float *) buffer)[i] = (float) ind;
                    break;
                case 8:
                    ((double *) buffer)[i] = (double) ind;
                    break;
                default:
                    MU_ASSERT_CATERVA(CATERVA_ERR_INVALID_STORAGE);
            }
            ind ++;
        }

        MU_ASSERT_CATERVA(caterva_array_append(ctx, src, buffer, nextsize * src->itemsize));
    }
    ctx->cfg->free(buffer);

    /* Fill dest array with caterva_array_t data */
    buffersize = src->size * src->itemsize;
    uint8_t *buffer_dest = ctx->cfg->alloc(buffersize);
    MU_ASSERT_CATERVA(caterva_array_to_buffer(ctx, src, buffer_dest, buffersize));

    ctx->cfg->free(buffer_dest);

    /* Free array  */
    MU_ASSERT_CATERVA(caterva_array_free(ctx, &src));
    return 0;
}

caterva_context_t *ctx;

static char* append_setup() {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_context_new(&cfg, &ctx);
    return 0;
}

static char* append_teardown() {
    caterva_context_free(&ctx);
    return 0;
}

static char* append_2_dim() {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 2;
    int64_t shape_[] = {8, 8};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;

    int64_t chunkshape_[] = {4, 4};
    int64_t blockshape_[] = {4, 4};
    double result[80] = {0, 1, 2, 3, 16, 17, 18, 19, 4, 5};
    bool enforceframe = true;
    char *filename = NULL;

    return test_append(ctx, itemsize, ndim, shape_, backend, chunkshape_, blockshape_, enforceframe, filename, result);
}

static char* append_3_dim_plain() {
    uint8_t itemsize = sizeof(double);
    const uint8_t ndim = 3;
    int64_t shape_[] = {4, 4, 5};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t pshape_[] = {0};
    int64_t spshape_[] = {0};
    double result[80] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    bool enforceframe = false;
    char *filename = NULL;

    return test_append(ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}

static char* append_4_dim() {
    uint8_t itemsize = sizeof(double);
    const uint8_t ndim = 4;
    int64_t shape_[] = {4, 3, 4, 6};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    
    int64_t pshape_[] = {2, 3, 2, 3};
    int64_t spshape_[] = {2, 3, 1, 3};
    double result[80] = {0,1,2,36,37,38,3,4,5,39};
    bool enforceframe = false;
    char *filename = NULL;
    
    return test_append(ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}

static char* append_5_dim_float() {
    uint8_t itemsize = sizeof(float);
    const uint8_t ndim = 5;
    int64_t shape_[] = {14, 23, 12, 11, 8};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;

    int64_t pshape_[] = {5, 12, 5, 3, 4};
    int64_t spshape_[] = {2, 5, 2, 2, 3};
    float result[80] = {0,1,2,3,3600,3601,3602,3603,4,5};
    bool enforceframe = false;
    char *filename = NULL;
    return test_append(ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}

static char* append_6_dim() {
    uint8_t itemsize = sizeof(double);
    const uint8_t ndim = 6;
    int64_t shape_[] = {10, 4, 9, 11, 8, 8};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;

    int64_t pshape_[] = {7, 3, 5, 6, 5, 3};
    int64_t spshape_[] = {2, 4, 2, 1, 2, 2};
    double result[80] = {0,1,2,9450,9451,9452,18900,18901,3,4};
    bool enforceframe = false;
    char *filename = NULL;
    return test_append(ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}

static char* append_7_dim_float() {
    uint8_t itemsize = sizeof(float);
    const uint8_t ndim = 7;
    int64_t shape_[] = {5, 3, 2, 4, 3, 2, 7};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;

    int64_t pshape_[] = {5, 2, 2, 3, 2, 2, 3};
    int64_t spshape_[] = {4, 2, 1, 2, 1, 2, 2};
    float result[80] = {0,1,2,720,721,722,1440,3,4,5};
    bool enforceframe = false;
    char *filename = NULL;
    return test_append(ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}

static char* append_8_dim_uint8() {
    uint8_t itemsize = sizeof(uint8_t);
    const uint8_t ndim = 8;
    int64_t shape_[] = {5, 3, 2, 1, 4, 3, 2, 7};
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;

    int64_t pshape_[] = {2, 2, 2, 1, 3, 1, 2, 4};
    int64_t spshape_[] = {2, 2, 2, 1, 3, 1, 2, 4};
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

    int64_t pshape_[] = {2, 3, 2, 2, 3, 1, 2, 4};
    int64_t spshape_[] = {2, 2, 2, 2, 2, 1, 2, 4};
    uint16_t result[80] = {0, 1, 2, 3, 576, 577, 578, 4, 5, 6};
    bool enforceframe = false;
    char *filename = NULL;
    return test_append(ctx, itemsize, ndim, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
}


static char* append_tests(char* filter) {
    if(strncmp("append", filter, strlen(filter)) != 0) {
        return 0;
    }

    MU_RUN_SETUP(append_setup);

    MU_RUN_TEST(append_2_dim);
    MU_RUN_TEST(append_3_dim_plain);
    MU_RUN_TEST(append_4_dim);
    MU_RUN_TEST(append_5_dim_float);
    MU_RUN_TEST(append_6_dim);
    MU_RUN_TEST(append_7_dim_float);
    MU_RUN_TEST(append_8_dim_uint16);
    MU_RUN_TEST(append_8_dim_uint8);

    MU_RUN_TEARDOWN(append_teardown);
    return 0;
}
