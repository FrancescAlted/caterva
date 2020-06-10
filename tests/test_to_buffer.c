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

static char* test_to_buffer(caterva_context_t *ctx, int8_t ndim, int8_t itemsize, int64_t *shape,
                           caterva_storage_backend_t backend, int64_t *chunkshape, int64_t *blockshape, bool enforceframe,
                           char* filename, void *result) {
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
                storage.properties.blosc.chunkshape[i] = (int32_t) chunkshape[i];
                storage.properties.blosc.blockshape[i] = (int32_t) blockshape[i];
            }
            break;
        default:
            MU_ASSERT_CATERVA(CATERVA_ERR_INVALID_STORAGE);
    }

    /* Create original data */
    int64_t buffersize = itemsize;
    for (int i = 0; i < ndim; ++i) {
        buffersize *= shape[i];
    }
    /* Create caterva_array_t with original data */
    caterva_array_t *src;
    MU_ASSERT_CATERVA(caterva_array_from_buffer(ctx, result, buffersize, &params, &storage, &src))

    /* Create dest buffer */
    uint8_t *destbuffer = ctx->cfg->alloc((size_t) buffersize);

    /* Fill dest buffer with a slice*/
    MU_ASSERT_CATERVA(caterva_array_to_buffer(ctx, src, destbuffer, buffersize))

    /* Assert results */
    MU_ASSERT_BUFFER(destbuffer, result, buffersize)

    ctx->cfg->free(destbuffer);
    MU_ASSERT_CATERVA(caterva_array_free(ctx, &src))
    
    return 0;
}

caterva_context_t *ctx;

static char* to_buffer_setup() {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_context_new(&cfg, &ctx);
    return 0;
}

static char* to_buffer_teardown() {
    caterva_context_free(&ctx);
    return 0;
}


static char* to_buffer_ndim_1() {
    const int8_t ndim = 1;
    int64_t shape_[] = {30};
    int64_t pshape_[] = {30};
    int64_t spshape_[] = {30};

    uint8_t itemsize = sizeof(double);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = false;
    char *filename = NULL;

    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    float *result = (float *) ctx->cfg->alloc((size_t)buf_size * itemsize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (float) i;
    }

    char* message = test_to_buffer(ctx, ndim, itemsize, shape_, backend, pshape_, spshape_,  enforceframe, filename, result);
    ctx->cfg->free(result);
    return message;
}

static char* to_buffer_ndim_2_plain_uint16() {
    const int8_t ndim = 2;
    int64_t shape_[] = {10, 10};
    int64_t pshape_[] = {0};
    int64_t spshape_[] = {0};

    uint8_t itemsize = sizeof(uint16_t);
    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    bool enforceframe = false;
    char *filename = NULL;

    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    uint16_t *result = (uint16_t *) ctx->cfg->alloc((size_t)buf_size * itemsize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (uint16_t) i;
    }

    char* message = test_to_buffer(ctx, ndim, itemsize, shape_, backend, pshape_, spshape_,  enforceframe, filename, result);
    ctx->cfg->free(result);
    return message;
}

static char* to_buffer_ndim_2_blosc() {
    const int8_t ndim = 2;
    uint8_t itemsize = sizeof(double);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = false;
    char *filename = NULL;

    int64_t shape_[] = {7, 10};
    int64_t pshape_[] = {5, 6};
    int64_t spshape_[] = {4, 5};
    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    double *result = (double *) ctx->cfg->alloc((size_t)buf_size *itemsize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (double) i;
    }

    char* message = test_to_buffer(ctx, ndim, itemsize, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
    ctx->cfg->free(result);
    return message;
 }


static char* to_buffer_ndim_3_float() {
    const int8_t ndim = 3;
    uint8_t itemsize = sizeof(float);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = false;
    char *filename = NULL;

    int64_t shape_[] = {10, 10, 10};
    int64_t pshape_[] = {7, 5, 4};
    int64_t spshape_[] = {4, 4, 3};
    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    float *result = (float *) ctx->cfg->alloc((size_t)buf_size * itemsize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (float) i;
    }

    char* message = test_to_buffer(ctx, ndim, itemsize, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
    ctx->cfg->free(result);
    return message;
 }

static char* to_buffer_ndim_3_double() {
    const int8_t ndim = 3;
    uint8_t itemsize = sizeof(double);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = false;
    char *filename = NULL;

    int64_t shape_[] = {5, 6, 3};
    int64_t pshape_[] = {4, 3, 3};
    int64_t spshape_[] = {3, 3, 2};

    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    double *result = (double *) ctx->cfg->alloc((size_t)buf_size * itemsize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (double) i;
    }

    char* message = test_to_buffer(ctx, ndim, itemsize, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
    ctx->cfg->free(result);
    return message;
}

static char* to_buffer_ndim_4_plain_uint8() {
    const int8_t ndim = 4;
    uint8_t itemsize = sizeof(uint8_t );
    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    bool enforceframe = false;
    char *filename = NULL;

    int64_t shape_[] = {10, 10, 10, 10};
    int64_t pshape_[] = {0};
    int64_t spshape_[] = {0};
    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    uint8_t *result = (uint8_t *) ctx->cfg->alloc((size_t)buf_size * itemsize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (uint8_t ) i;
    }

    char* message = test_to_buffer(ctx, ndim, itemsize, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
    ctx->cfg->free(result);
    return message;
}

static char* to_buffer_ndim_5_float() {
    const int8_t ndim = 5;
    uint8_t itemsize = sizeof(float);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = false;
    char *filename = NULL;

    int64_t shape_[] = {10, 10, 10, 10, 10};
    int64_t pshape_[] = {3, 5, 2, 4, 5};
    int64_t spshape_[] = {3, 3, 2, 3, 5};
    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    float *result = (float *) ctx->cfg->alloc((size_t)buf_size * itemsize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (float) i;
    }

    char* message = test_to_buffer(ctx, ndim, itemsize, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
    ctx->cfg->free(result);
    return message;
}

static char* to_buffer_ndim_6() {
    const int8_t ndim = 6;
    uint8_t itemsize = sizeof(double);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = false;
    char *filename = NULL;

    int64_t shape_[] = {10, 10, 10, 10, 10, 10};
    int64_t pshape_[] = {3, 4, 2, 2, 2, 3};
    int64_t spshape_[] = {2, 3, 1, 2, 1, 2};
    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    double *result = (double *) ctx->cfg->alloc((size_t)buf_size * itemsize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (double) i;
    }

    char* message = test_to_buffer(ctx, ndim, itemsize, shape_, backend, pshape_, spshape_,  enforceframe, filename, result);
    ctx->cfg->free(result);
    return message;
 }

static char* to_buffer_ndim_7_uint16() {
    const int8_t ndim = 7;
    uint8_t itemsize = sizeof(uint16_t);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = false;
    char *filename = NULL;

    int64_t shape_[] = {4, 5, 3, 4, 4, 5, 2};
    int64_t pshape_[] = {3, 4, 2, 2, 2, 3, 1};
    int64_t spshape_[] = {2, 3, 2, 2, 2, 3, 1};
    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    uint16_t *result = (uint16_t *) ctx->cfg->alloc((size_t)buf_size * itemsize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (uint16_t) i;
    }
    char* message = test_to_buffer(ctx, ndim, itemsize, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
    ctx->cfg->free(result);
    return message;
}

static char* to_buffer_ndim_8_float_plain() {
    const int8_t ndim = 8;
    uint8_t itemsize = sizeof(float);
    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    bool enforceframe = false;
    char *filename = NULL;

    int64_t shape_[] = {4, 5, 3, 4, 4, 5, 2, 8};
    int64_t pshape_[] = {0};
    int64_t spshape_[] = {0};
    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    float *result = (float *) ctx->cfg->alloc((size_t)buf_size * itemsize);
    for (int i = 0; i < buf_size; ++i) {
        result[i] = (float) i;
    }
    char* message = test_to_buffer(ctx, ndim, itemsize, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
    ctx->cfg->free(result);
    return message;
}

static char* to_buffer_ndim_3_hard() {
    const int8_t ndim = 3;
    uint8_t itemsize = sizeof(double);
    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = false;
    char *filename = NULL;

    int64_t shape_[] = {252, 252, 252};
    int64_t pshape_[] = {64, 64, 64};
    int64_t spshape_[] = {16, 16, 16};

    int64_t buf_size = 1;
    for (int i = 0; i < ndim; ++i) {
        buf_size *= (shape_[i]);
    }
    double *result = (double *) ctx->cfg->alloc((size_t)buf_size * itemsize);
    for (int64_t i = 0; i < buf_size; ++i) {
        result[i] = (double) i;
    }

    char* message = test_to_buffer(ctx, ndim, itemsize, shape_, backend, pshape_, spshape_, enforceframe, filename, result);
    ctx->cfg->free(result);
    return message;
 }


static char* all_tests() {
    MU_RUN_SETUP(to_buffer_setup)

    MU_RUN_TEST(to_buffer_ndim_1)
    MU_RUN_TEST(to_buffer_ndim_2_blosc)
    MU_RUN_TEST(to_buffer_ndim_2_plain_uint16)
    MU_RUN_TEST(to_buffer_ndim_3_double)
    MU_RUN_TEST(to_buffer_ndim_3_float)
    MU_RUN_TEST(to_buffer_ndim_3_hard)
    MU_RUN_TEST(to_buffer_ndim_4_plain_uint8)
    MU_RUN_TEST(to_buffer_ndim_5_float)
    MU_RUN_TEST(to_buffer_ndim_6)
    MU_RUN_TEST(to_buffer_ndim_7_uint16)
    MU_RUN_TEST(to_buffer_ndim_8_float_plain)

    MU_RUN_TEARDOWN(to_buffer_teardown)
    return 0;
}

MU_RUN_SUITE("TO BUFFER")
