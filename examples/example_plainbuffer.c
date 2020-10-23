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

#include <caterva.h>

int main() {

    int8_t ndim = 3;
    int64_t shape[] = {500, 500, 500};
    int8_t itemsize = 8;

    int64_t slice_start[] = {50, 122, 1};
    int64_t slice_stop[] = {466, 256, 221};

    int64_t nelem = 1;
    for (int i = 0; i < ndim; ++i) {
        nelem *= shape[i];
    }
    int64_t size = nelem * itemsize;
    int8_t *data = malloc(size);

    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;

    caterva_context_t *ctx;
    caterva_context_new(&cfg, &ctx);

    caterva_params_t params = {0};
    params.ndim = ndim;
    params.itemsize = itemsize;
    for (int i = 0; i < ndim; ++i) {
        params.shape[i] = shape[i];
    }

    caterva_storage_t storage = {0};
    storage.backend = CATERVA_STORAGE_PLAINBUFFER;

    caterva_array_t *arr;
    caterva_array_from_buffer(ctx, data, size, &params, &storage, &arr);


    int64_t slice_shape[CATERVA_MAX_DIM];
    int64_t slice_nelem = 1;
    for (int i = 0; i < ndim; ++i) {
        slice_shape[i] = slice_stop[i] - slice_start[i];
        slice_nelem *= slice_shape[i];
    }
    int64_t slice_size = slice_nelem * itemsize;
    int8_t *slice = malloc(slice_size);

    // blosc_timestamp_t t0, t1;
    // blosc_set_timestamp(&t0);
    caterva_array_get_slice_buffer(ctx, arr, slice_start, slice_stop, slice_shape, slice,
                                   slice_size);
    // blosc_set_timestamp(&t1);


    // printf("Elapsed seconds: %.5f\n", blosc_elapsed_secs(t0, t1));

    int8_t *a = malloc(1000);
    slice_nelem += a[50];
    a = 0;
    return 0;
}