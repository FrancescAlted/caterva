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

# include <caterva.h>

int frame_generator(int8_t *data, int8_t ndim, int64_t shape[8], int32_t chunkshape[8],
                    int32_t blockshape[8], int8_t itemsize, int64_t size, char *urlpath) {

    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_ctx_t *ctx;
    caterva_ctx_new(&cfg, &ctx);

    caterva_params_t params = {0};
    params.ndim = ndim;
    params.itemsize = itemsize;
    for (int i = 0; i < ndim; ++i) {
        params.shape[i] = shape[i];
    }

    caterva_storage_t storage = {0};
    storage.urlpath = urlpath;
    storage.sequencial = true;
    for (int i = 0; i < ndim; ++i) {
        storage.chunkshape[i] = chunkshape[i];
        storage.blockshape[i] = blockshape[i];
    }

    caterva_array_t *arr;
    CATERVA_ERROR(caterva_from_buffer(ctx, data, size, &params, &storage, &arr));

    return 0;
}

int all_eq() {
    int8_t ndim = 3;
    int64_t shape[] = {100, 50, 100};
    int32_t chunkshape[] = {40, 20, 60};
    int32_t blockshape[] = {20, 10, 30};
    int8_t itemsize = 8;
    int64_t nelem = 1;
    for (int i = 0; i < ndim; ++i) {
        nelem *= shape[i];
    }
    int64_t size = nelem * itemsize;

    int8_t *data = malloc(size);
    for (int i= 0; i < nelem; i++) {
        data[i] = (int8_t) 22;
    }
    char *urlpath = "all_eq.caterva";
    CATERVA_ERROR(frame_generator(data, ndim, shape, chunkshape, blockshape, itemsize, size, urlpath));

    return 0;
}

int cyclic() {
    int8_t ndim = 3;
    int64_t shape[] = {100, 50, 100};
    int32_t chunkshape[] = {40, 20, 60};
    int32_t blockshape[] = {20, 10, 30};
    int8_t itemsize = 8;
    int64_t nelem = 1;
    for (int i = 0; i < ndim; ++i) {
        nelem *= shape[i];
    }
    int64_t size = nelem * itemsize;

    int8_t *data = malloc(size);
    for (int i= 0; i < nelem; i++) {
        data[i] = (int8_t) i;
    }
    char *urlpath = "cyclic.caterva";
    CATERVA_ERROR(frame_generator(data, ndim, shape, chunkshape, blockshape, itemsize, size, urlpath));

    return 0;
}

int many_matches() {
    int8_t ndim = 3;
    int64_t shape[] = {80, 120, 111};
    int32_t chunkshape[] = {40, 30, 50};
    int32_t blockshape[] = {11, 14, 24};
    int8_t itemsize = 8;
    int64_t nelem = 1;
    for (int i = 0; i < ndim; ++i) {
        nelem *= shape[i];
    }
    int64_t size = nelem * itemsize;

    int8_t *data = malloc(size);
    for (int i = 0; i < nelem; i += 2) {
        data[i] = (int8_t) i;
        data[i + 1] = (int8_t) 2;
    }
    char *urlpath = "many_matches.caterva";
    CATERVA_ERROR(frame_generator(data, ndim, shape, chunkshape, blockshape, itemsize, size, urlpath));

    return 0;
}

int main() {
    int err;
    err = all_eq();
    if (err != CATERVA_SUCCEED) {
        printf("\n All_eq error: %d", err);
    }
    err = cyclic();
    if (err != CATERVA_SUCCEED) {
        printf("\n Cyclic error: %d", err);
    }
    err = many_matches();
    if (err != CATERVA_SUCCEED) {
        printf("\n Many_matches error: %d", err);
    }
    return err;
}