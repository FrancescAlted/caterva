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

static void test_get_slice(caterva_context_t *ctx, uint8_t itemsize, uint8_t ndim, int64_t *shape,
                           caterva_storage_backend_t backend, int64_t *chunkshape, bool enforceframe, char *filename,
                           caterva_storage_backend_t backend2, int64_t *chunkshape2, bool enforceframe2, char *filename2,
                           int64_t *start, int64_t *stop, void *result) {

    caterva_params_t params;
    params.itemsize = itemsize;
    params.ndim = ndim;
    for (int i = 0; i < ndim; ++i) {
        params.shape[i] = shape[i];
    }

    caterva_storage_t storage;
    storage.backend = backend;
    switch (backend) {
        case CATERVA_STORAGE_PLAINBUFFER:
            break;
        case CATERVA_STORAGE_BLOSC:
            storage.properties.blosc.filename = filename;
            storage.properties.blosc.enforceframe = enforceframe;
            for (int i = 0; i < ndim; ++i) {
                storage.properties.blosc.chunkshape[i] = chunkshape[i];
            }
            break;
        default:
            CATERVA_TEST_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    /* Create original data */
    int64_t buffersize = itemsize;
    for (int i = 0; i < ndim; ++i) {
        buffersize *= shape[i];
    }
    uint8_t *buffer = malloc(buffersize);
    fill_buf(buffer, itemsize, buffersize / itemsize);

    /* Create caterva_array_t with original data */
    caterva_array_t *src;
    CATERVA_TEST_ERROR(caterva_array_from_buffer(ctx, buffer, buffersize, &params, &storage, &src));


    /* Create storage for dest container */

    caterva_storage_t storage2;
    storage2.backend = backend2;
    switch (backend2) {
        case CATERVA_STORAGE_PLAINBUFFER:
            break;
        case CATERVA_STORAGE_BLOSC:
            storage2.properties.blosc.filename = filename2;
            storage2.properties.blosc.enforceframe = enforceframe2;
            for (int i = 0; i < ndim; ++i) {
                storage2.properties.blosc.chunkshape[i] = chunkshape2[i];
            }
            break;
        default:
            CATERVA_TEST_ERROR(CATERVA_ERR_INVALID_STORAGE);
    }

    caterva_array_t *dest;
    CATERVA_TEST_ERROR(caterva_array_get_slice(ctx, src, start, stop, &storage2, &dest));

    uint8_t *buffer_dest = malloc(buffersize);
    CATERVA_TEST_ERROR(caterva_array_to_buffer(ctx, dest, buffer_dest, buffersize));

    /* Testing */
    double tol = (itemsize == 4) ? 1e-6 : 1e-15;
    assert_buf((uint8_t *) result, buffer_dest, itemsize, dest->size, tol);

    /* Free mallocs */
    free(buffer);
    free(buffer_dest);
    CATERVA_TEST_ERROR(caterva_array_free(ctx, &src));
    CATERVA_TEST_ERROR(caterva_array_free(ctx, &dest));

}

LWTEST_DATA(get_slice) {
    caterva_context_t *ctx;
};

LWTEST_SETUP(get_slice) {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_context_new(&cfg, &data->ctx);
}

LWTEST_TEARDOWN(get_slice) {
    caterva_context_free(&data->ctx);
}

LWTEST_FIXTURE(get_slice, 1_double_plainbuffer_plainbuffer) {
    int64_t start[] = {2};
    int64_t stop[] = {9};

    double result[1024] = {2, 3, 4, 5, 6, 7, 8};

    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 1;
    int64_t shape[] = {10};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape2[] = {0};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    test_get_slice(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename,
                   backend2, chunkshape2, enforceframe2, filename2, start, stop, result);
}

LWTEST_FIXTURE(get_slice, 2_double_plainbuffer_blosc) {
    int64_t start[] = {5, 3};
    int64_t stop[] = {9, 10};

    double result[1024] = {53, 54, 55, 56, 57, 58, 59, 63, 64, 65, 66, 67, 68, 69, 73, 74, 75, 76,
                           77, 78, 79, 83, 84, 85, 86, 87, 88, 89};

    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 2;
    int64_t shape[] = {10, 10};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape2[] = {3, 2};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    test_get_slice(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename,
        backend2, chunkshape2, enforceframe2, filename2, start, stop, result);
}


LWTEST_FIXTURE(get_slice, 3_float_blosc_blosc) {
    int64_t start[] = {3, 0, 3};
    int64_t stop[] = {6, 7, 10};

    float result[1024] = {303, 304, 305, 306, 307, 308, 309, 313, 314, 315, 316, 317, 318, 319,
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

    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 3;
    int64_t shape[] = {10, 10, 10};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {3, 5, 2};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape2[] = {2, 4, 3};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    test_get_slice(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename,
                   backend2, chunkshape2, enforceframe2, filename2, start, stop, result);
}

LWTEST_FIXTURE(get_slice, 4_double_blosc_plainbuffer) {
    int64_t start[] = {5, 3, 9, 2};
    int64_t stop[] = {9, 6, 10, 7};

    double result[1024] = {5392, 5393, 5394, 5395, 5396, 5492, 5493, 5494, 5495, 5496, 5592, 5593,
                           5594, 5595, 5596, 6392, 6393, 6394, 6395, 6396, 6492, 6493, 6494, 6495,
                           6496, 6592, 6593, 6594, 6595, 6596, 7392, 7393, 7394, 7395, 7396, 7492,
                           7493, 7494, 7495, 7496, 7592, 7593, 7594, 7595, 7596, 8392, 8393, 8394,
                           8395, 8396, 8492, 8493, 8494, 8495, 8496, 8592, 8593, 8594, 8595, 8596};

    uint8_t itemsize = sizeof(double);
    uint8_t ndim = 4;
    int64_t shape[] = {10, 10, 10, 10};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {2, 2, 4, 2};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape2[] = {0};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    test_get_slice(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename,
                   backend2, chunkshape2, enforceframe2, filename2, start, stop, result);
}

LWTEST_FIXTURE(get_slice, 5_float_plainbuffer_plainbuffer) {
    int64_t start[] = {6, 0, 5, 5, 7};
    int64_t stop[] = {8, 9, 6, 6, 10};

    float result[1024] = {60557, 60558, 60559, 61557, 61558, 61559, 62557, 62558, 62559, 63557,
                          63558, 63559, 64557, 64558, 64559, 65557, 65558, 65559, 66557, 66558,
                          66559, 67557, 67558, 67559, 68557, 68558, 68559, 70557, 70558, 70559,
                          71557, 71558, 71559, 72557, 72558, 72559, 73557, 73558, 73559, 74557,
                          74558, 74559, 75557, 75558, 75559, 76557, 76558, 76559, 77557, 77558,
                          77559, 78557, 78558, 78559};

    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 5;
    int64_t shape[] = {10, 10, 10, 10, 10};

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape2[] = {0};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    test_get_slice(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename,
                   backend2, chunkshape2, enforceframe2, filename2, start, stop, result);
}


LWTEST_FIXTURE(get_slice, 6_double_plainbuffer_blosc_frame) {
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

    caterva_storage_backend_t backend = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape[] = {0};
    bool enforceframe = false;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape2[] = {1, 3, 2, 1, 2, 2};
    bool enforceframe2 = true;
    char *filename2 = NULL;

    test_get_slice(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename,
                   backend2, chunkshape2, enforceframe2, filename2, start, stop, result);
}

LWTEST_FIXTURE(get_slice, 7_float_blosc_frame_plainbuffer) {
    int64_t start[] = {1, 4, 2, 2, 2, 2, 1};
    int64_t stop[] = {4, 5, 3, 4, 3, 5, 2};

    float result[1024] = {4745, 4747, 4749, 4785, 4787, 4789, 7145, 7147, 7149, 7185, 7187,
                          7189, 9545, 9547, 9549, 9585, 9587, 9589};


    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 7;
    int64_t shape[] = {4, 5, 3, 4, 4, 5, 2};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {3, 4, 2, 2, 2, 2, 1};
    bool enforceframe = true;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_PLAINBUFFER;
    int64_t chunkshape2[] = {0};
    bool enforceframe2 = false;
    char *filename2 = NULL;

    test_get_slice(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename,
                   backend2, chunkshape2, enforceframe2, filename2, start, stop, result);
}

LWTEST_FIXTURE(get_slice, 8_float_blosc_frame_blosc_frame) {
    int64_t start[] = {3, 2, 2, 2, 2, 1, 1, 1};
    int64_t stop[] = {4, 3, 3, 3, 3, 3, 2, 3};

    float result[1024] = {16750, 16751, 16756, 16757};

    uint8_t itemsize = sizeof(float);
    uint8_t ndim = 8;
    int64_t shape[] = {5, 3, 4, 5, 4, 3, 2, 3};

    caterva_storage_backend_t backend = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape[] = {2, 2, 1, 1, 2, 2, 1, 1};
    bool enforceframe = true;
    char *filename = NULL;

    caterva_storage_backend_t backend2 = CATERVA_STORAGE_BLOSC;
    int64_t chunkshape2[] = {1, 1, 1, 1, 1, 1, 1, 2};
    bool enforceframe2 = true;
    char *filename2 = NULL;

    test_get_slice(data->ctx, itemsize, ndim, shape, backend, chunkshape, enforceframe, filename,
                   backend2, chunkshape2, enforceframe2, filename2, start, stop, result);}
