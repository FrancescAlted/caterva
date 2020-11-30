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


static char* test_get_slice(caterva_context_t *ctx, int8_t ndim, int8_t itemsize, int64_t *shape,
                           caterva_storage_backend_t backend, int32_t *chunkshape, int32_t *blockshape, bool enforceframe,
                           char* filename, int64_t *start, int64_t *stop, int64_t *destshape, void *result) {

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

    /* Create original data */
    size_t buffersize = itemsize;
    for (int i = 0; i < ndim; ++i) {
        buffersize *= (size_t) shape[i];
    }
    double *buffer = ctx->cfg->alloc(buffersize);
    MU_ASSERT("Buffer filled incorrectly", fill_buf(buffer, itemsize, buffersize / itemsize));

    /* Create caterva_array_t with original data */
    caterva_array_t *src;
    MU_ASSERT_CATERVA(caterva_array_from_buffer(ctx, buffer, buffersize, &params, &storage, &src));

    /* Create dest buffer */
    int64_t destbuffersize = itemsize;
    for (int i = 0; i < ndim; ++i) {
        destbuffersize *= destshape[i];
    }
    uint8_t *destbuffer = ctx->cfg->alloc((size_t) destbuffersize);

    /* Fill dest buffer with a slice*/
    MU_ASSERT_CATERVA(caterva_array_get_slice_buffer(ctx, src, start, stop, destshape, destbuffer, destbuffersize));

    /* Assert results */
    MU_ASSERT_BUFFER(destbuffer, result, destbuffersize);

    ctx->cfg->free(buffer);
    ctx->cfg->free(destbuffer);
    MU_ASSERT_CATERVA(caterva_array_free(ctx, &src));
    
    return 0;
}


caterva_context_t *ctx;

static char* get_slice_buffer_setup() {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    cfg.complevel = 9;
    caterva_context_new(&cfg, &ctx);
    return 0;
}

static char* get_slice_buffer_teardown() {
    caterva_context_free(&ctx);
    return 0;
}

static char* get_slice_buffer_1_acceleration_path() {
    int64_t start[] = {0};
    int64_t stop[] = {30};

    double result[1024] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                           17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29};

    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 1;
    int64_t shape[] = {30};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int32_t chunkshape[] = {30};
    int32_t blockshape[] = {30};
    bool enforceframe = false;
    char *filename = NULL;

    int64_t destshape[] = {0};
    for (int i = 0; i < ndim; ++i) {
        destshape[i] = stop[i] - start[i];
    }
    return test_get_slice(ctx, ndim, itemsize, shape, backend, chunkshape, blockshape, enforceframe, filename,
                   start, stop, destshape, result);
}

static char* get_slice_buffer_1_double_blosc() {
    int64_t start[] = {2};
    int64_t stop[] = {9};

    double result[1024] = {2, 3, 4, 5, 6, 7, 8};

    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 1;
    int64_t shape[] = {30};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int32_t chunkshape[] = {30};
    int32_t blockshape[] = {20};
    bool enforceframe = false;
    char *filename = NULL;

    int64_t destshape[] = {0};
    for (int i = 0; i < ndim; ++i) {
        destshape[i] = stop[i] - start[i];
    }
    return test_get_slice(ctx, ndim, itemsize, shape, backend, chunkshape, blockshape, enforceframe, filename,
                   start, stop, destshape, result);
}

static char* get_slice_buffer_2_uint16_blosc() {
    int64_t start[] = {5, 3};
    int64_t stop[] = {9, 10};

    uint16_t result[1024] = {53, 54, 55, 56, 57, 58, 59, 63, 64, 65, 66, 67, 68, 69, 73, 74, 75, 76,
                           77, 78, 79, 83, 84, 85, 86, 87, 88, 89};

    uint8_t itemsize = sizeof(uint16_t);
    uint8_t ndim = 2;
    int64_t shape[] = {14, 10};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int32_t chunkshape[] = {11, 9};
    int32_t blockshaoe[] = {9, 8};
    bool enforceframe = false;
    char *filename = NULL;

    int64_t destshape[] = {0, 0};
    for (int i = 0; i < ndim; ++i) {
        destshape[i] = stop[i] - start[i];
    }
    return test_get_slice(ctx, ndim, itemsize, shape, backend, chunkshape, blockshaoe, enforceframe, filename,
                   start, stop, destshape, result);
}

static char* get_slice_buffer_2_uint8_plainbuffer() {
    int64_t start[] = {2, 2};
    int64_t stop[] = {4, 4};

    uint8_t result[1024] = {14, 15, 20, 21};

    uint8_t itemsize = sizeof(uint8_t);
    uint8_t ndim = 2;
    int64_t shape[] = {5, 6};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int32_t chunkshape[] = {0};
    int32_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    int64_t destshape[] = {0, 0, 0};
    for (int i = 0; i < ndim; ++i) {
        destshape[i] = stop[i] - start[i];
    }
    return test_get_slice(ctx, ndim, itemsize, shape, backend, chunkshape, blockshape, enforceframe, filename,
                   start, stop, destshape, result);
}

static char* get_slice_buffer_3_float_plainbuffer() {
    int64_t start[] = {2, 2, 0};
    int64_t stop[] = {4, 4, 2};

    float result[1024] = {42, 43, 45, 46, 60, 61, 63, 64};

    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 3;
    int64_t shape[] = {5, 6, 3};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int32_t chunkshape[] = {0};
    int32_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    int64_t destshape[] = {0, 0, 0};
    for (int i = 0; i < ndim; ++i) {
        destshape[i] = stop[i] - start[i];
    }

    return test_get_slice(ctx, ndim, itemsize, shape, backend, chunkshape, blockshape, enforceframe, filename,
                   start, stop, destshape, result);
}


static char* get_slice_buffer_3_float_blosc() {
    int64_t start[] = {3, 0, 3};
    int64_t stop[] = {6, 7, 10};

    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 3;
    int64_t shape_[] = {10, 10, 10};
    int32_t pshape_[] = {3, 5, 2};
    int32_t spshape_[] = {3, 3, 2};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = false;
    char *filename = NULL;

    int64_t destshape[] = {0, 0, 0};
    for (int i = 0; i < ndim; ++i) {
        destshape[i] = stop[i] - start[i];
    }

    double result[1024] = {303, 304, 305, 306, 307, 308, 309, 313, 314, 315, 316, 317, 318, 319,
                           323, 324, 325, 326, 327, 328, 329, 333, 334, 335, 336, 337, 338, 339,
                           343, 344, 345, 346, 347, 348, 349, 353, 354, 355, 356, 357, 358, 359,
                           363, 364, 365, 366, 367, 368, 369, 403, 404, 405, 406, 407, 408, 409,
                           413, 414, 415, 416, 417, 418, 419, 423, 424, 425, 426, 427, 428, 429,
                           433, 434, 435, 436, 437, 438, 439, 443, 444, 445, 446, 447, 448, 449,
                           453, 454, 455, 456, 457, 458, 459, 463, 464, 465, 466, 467, 468, 469,
                           503, 504, 505, 506, 507, 508, 509, 513, 514, 515, 516, 517, 518, 519,
                           523, 524, 525, 526, 527, 528, 529, 533, 534, 535, 536, 537, 538, 539,
                           543, 544, 545, 546, 547, 548, 549, 553, 554, 555, 556, 557, 558, 559,
                           563, 564, 565, 566, 567, 568, 569};

    return test_get_slice(ctx, ndim, itemsize, shape_, backend, pshape_, spshape_, enforceframe, filename,
                   start, stop, destshape, result);
}


static char* get_slice_buffer_3_double_blosc() {
    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 3;
    int64_t shape_[] = {10, 10, 10};
    int32_t pshape_[] = {3, 5, 2};
    int32_t spshape_[] = {3, 5, 2};
    int64_t start[] = {3, 0, 3};
    int64_t stop[] = {6, 7, 10};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    bool enforceframe = false;
    char *filename = NULL;

    int64_t destshape[] = {0, 0, 0};
    for (int i = 0; i < ndim; ++i) {
        destshape[i] = stop[i] - start[i];
    }

    double result[1024] = {303, 304, 305, 306, 307, 308, 309, 313, 314, 315, 316, 317, 318, 319,
                           323, 324, 325, 326, 327, 328, 329, 333, 334, 335, 336, 337, 338, 339,
                           343, 344, 345, 346, 347, 348, 349, 353, 354, 355, 356, 357, 358, 359,
                           363, 364, 365, 366, 367, 368, 369, 403, 404, 405, 406, 407, 408, 409,
                           413, 414, 415, 416, 417, 418, 419, 423, 424, 425, 426, 427, 428, 429,
                           433, 434, 435, 436, 437, 438, 439, 443, 444, 445, 446, 447, 448, 449,
                           453, 454, 455, 456, 457, 458, 459, 463, 464, 465, 466, 467, 468, 469,
                           503, 504, 505, 506, 507, 508, 509, 513, 514, 515, 516, 517, 518, 519,
                           523, 524, 525, 526, 527, 528, 529, 533, 534, 535, 536, 537, 538, 539,
                           543, 544, 545, 546, 547, 548, 549, 553, 554, 555, 556, 557, 558, 559,
                           563, 564, 565, 566, 567, 568, 569};

    return test_get_slice(ctx, ndim, itemsize, shape_, backend, pshape_, spshape_, enforceframe, filename,
                   start, stop, destshape, result);
}


static char* get_slice_buffer_4_float_blosc() {
    int64_t start[] = {5, 3, 9, 2};
    int64_t stop[] = {9, 6, 10, 7};

    float result[1024] = {5392, 5393, 5394, 5395, 5396, 5492, 5493, 5494, 5495, 5496, 5592, 5593,
                          5594, 5595, 5596, 6392, 6393, 6394, 6395, 6396, 6492, 6493, 6494, 6495,
                          6496, 6592, 6593, 6594, 6595, 6596, 7392, 7393, 7394, 7395, 7396, 7492,
                          7493, 7494, 7495, 7496, 7592, 7593, 7594, 7595, 7596, 8392, 8393, 8394,
                          8395, 8396, 8492, 8493, 8494, 8495, 8496, 8592, 8593, 8594, 8595, 8596};

    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 4;
    int64_t shape[] = {10, 10, 10, 10};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int32_t chunkshape[] = {3, 2, 3, 2};
    int32_t blockshape[] = {3, 2, 2, 2};
    bool enforceframe = true;
    char *filename = NULL;

    int64_t destshape[] = {0, 0, 0, 0};
    for (int i = 0; i < ndim; ++i) {
        destshape[i] = stop[i] - start[i];
    }

    return test_get_slice(ctx, ndim, itemsize, shape, backend, chunkshape, blockshape, enforceframe, filename,
                   start, stop, destshape, result);
}

static char* get_slice_buffer_5_double_plainbuffer() {
    int64_t start[] = {6, 0, 5, 5, 7};
    int64_t stop[] = {8, 9, 6, 6, 10};

    double result[1024] = {60557, 60558, 60559, 61557, 61558, 61559, 62557, 62558, 62559, 63557,
                           63558, 63559, 64557, 64558, 64559, 65557, 65558, 65559, 66557, 66558,
                           66559, 67557, 67558, 67559, 68557, 68558, 68559, 70557, 70558, 70559,
                           71557, 71558, 71559, 72557, 72558, 72559, 73557, 73558, 73559, 74557,
                           74558, 74559, 75557, 75558, 75559, 76557, 76558, 76559, 77557, 77558,
                           77559, 78557, 78558, 78559};

    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 5;
    int64_t shape[] = {10, 10, 10, 10, 10};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int32_t chunkshape[] = {0};
    int32_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    int64_t destshape[] = {0, 0, 0, 0, 0};
    for (int i = 0; i < ndim; ++i) {
        destshape[i] = stop[i] - start[i];
    }

    return test_get_slice(ctx, ndim, itemsize, shape, backend, chunkshape, blockshape, enforceframe, filename,
                   start, stop, destshape, result);
}


static char* get_slice_buffer_6_double_blosc() {
    int64_t start[] = {0, 4, 2, 4, 5, 1};
    int64_t stop[] = {1, 7, 4, 6, 8, 3};

    double result[1024] = {42451, 42452, 42461, 42462, 42471, 42472, 42551, 42552, 42561, 42562,
                           42571, 42572, 43451, 43452, 43461, 43462, 43471, 43472, 43551, 43552,
                           43561, 43562, 43571, 43572, 52451, 52452, 52461, 52462, 52471, 52472,
                           52551, 52552, 52561, 52562, 52571, 52572, 53451, 53452, 53461, 53462,
                           53471, 53472, 53551, 53552, 53561, 53562, 53571, 53572, 62451, 62452,
                           62461, 62462, 62471, 62472, 62551, 62552, 62561, 62562, 62571, 62572,
                           63451, 63452, 63461, 63462, 63471, 63472, 63551, 63552, 63561, 63562,
                           63571, 63572};

    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 6;
    int64_t shape[] = {10, 10, 10, 10, 10, 10};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int32_t chunkshape[] = {6, 5, 3, 5, 4, 2};
    int32_t blockshape[] = {3, 2, 3, 5, 4, 2};
    bool enforceframe = false;
    char *filename = NULL;

    int64_t destshape[] = {0, 0, 0, 0, 0, 0};
    for (int i = 0; i < ndim; ++i) {
        destshape[i] = stop[i] - start[i];
    }

    return test_get_slice(ctx, ndim, itemsize, shape, backend, chunkshape, blockshape, enforceframe, filename,
                   start, stop, destshape, result);
}

static char* get_slice_buffer_7_float_plainbuffer() {
    int64_t start[] = {5, 4, 3, 8, 4, 5, 1};
    int64_t stop[] = {8, 6, 5, 9, 7, 7, 3};

    float result[1024] = {5438451, 5438452, 5438461, 5438462, 5438551, 5438552, 5438561, 5438562,
                          5438651, 5438652, 5438661, 5438662, 5448451, 5448452, 5448461, 5448462,
                          5448551, 5448552, 5448561, 5448562, 5448651, 5448652, 5448661, 5448662,
                          5538451, 5538452, 5538461, 5538462, 5538551, 5538552, 5538561, 5538562,
                          5538651, 5538652, 5538661, 5538662, 5548451, 5548452, 5548461, 5548462,
                          5548551, 5548552, 5548561, 5548562, 5548651, 5548652, 5548661, 5548662,
                          6438451, 6438452, 6438461, 6438462, 6438551, 6438552, 6438561, 6438562,
                          6438651, 6438652, 6438661, 6438662, 6448451, 6448452, 6448461, 6448462,
                          6448551, 6448552, 6448561, 6448562, 6448651, 6448652, 6448661, 6448662,
                          6538451, 6538452, 6538461, 6538462, 6538551, 6538552, 6538561, 6538562,
                          6538651, 6538652, 6538661, 6538662, 6548451, 6548452, 6548461, 6548462,
                          6548551, 6548552, 6548561, 6548562, 6548651, 6548652, 6548661, 6548662,
                          7438451, 7438452, 7438461, 7438462, 7438551, 7438552, 7438561, 7438562,
                          7438651, 7438652, 7438661, 7438662, 7448451, 7448452, 7448461, 7448462,
                          7448551, 7448552, 7448561, 7448562, 7448651, 7448652, 7448661, 7448662,
                          7538451, 7538452, 7538461, 7538462, 7538551, 7538552, 7538561, 7538562,
                          7538651, 7538652, 7538661, 7538662, 7548451, 7548452, 7548461, 7548462,
                          7548551, 7548552, 7548561, 7548562, 7548651, 7548652, 7548661, 7548662};

    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 7;
    int64_t shape[] = {10, 10, 10, 10, 10, 10, 10};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int32_t chunkshape[] = {0};
    int32_t blockshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    int64_t destshape[] = {0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < ndim; ++i) {
        destshape[i] = stop[i] - start[i];
    }

    return test_get_slice(ctx, ndim, itemsize, shape, backend, chunkshape, blockshape, enforceframe, filename,
                   start, stop, destshape, result);
}

static char* get_slice_buffer_8_float_blosc() {
    int64_t start[] = {3, 2, 2, 2, 2, 1, 1, 1};
    int64_t stop[] = {4, 3, 3, 3, 3, 3, 2, 3};

    double result[1024] = {16750, 16751, 16756, 16757};

    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 8;
    int64_t shape[] = {5, 3, 4, 5, 4, 3, 2, 3};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int32_t chunkshape[] = {2, 2, 1, 1, 2, 2, 1, 1};
    int32_t blockshape[] = {2, 2, 1, 1, 2, 2, 1, 1};
    bool enforceframe = false;
    char *filename = NULL;

    int64_t destshape[] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < ndim; ++i) {
        destshape[i] = stop[i] - start[i];
    }

    return test_get_slice(ctx, ndim, itemsize, shape, backend, chunkshape, blockshape, enforceframe, filename,
                   start, stop, destshape, result);
}

static char* all_tests() {
    MU_RUN_SETUP(get_slice_buffer_setup)

    MU_RUN_TEST(get_slice_buffer_1_acceleration_path)
    MU_RUN_TEST(get_slice_buffer_1_double_blosc)
    MU_RUN_TEST(get_slice_buffer_2_uint16_blosc)
    MU_RUN_TEST(get_slice_buffer_2_uint8_plainbuffer)
    MU_RUN_TEST(get_slice_buffer_3_double_blosc)
    MU_RUN_TEST(get_slice_buffer_3_float_blosc)
    MU_RUN_TEST(get_slice_buffer_3_float_plainbuffer)
    MU_RUN_TEST(get_slice_buffer_4_float_blosc)
    MU_RUN_TEST(get_slice_buffer_5_double_plainbuffer)
    MU_RUN_TEST(get_slice_buffer_6_double_blosc)
    MU_RUN_TEST(get_slice_buffer_7_float_plainbuffer)
    MU_RUN_TEST(get_slice_buffer_8_float_blosc)

    MU_RUN_TEARDOWN(get_slice_buffer_teardown)
    return 0;
}

MU_RUN_SUITE()
