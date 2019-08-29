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

int main(){

    // Create a context
    caterva_ctx_t *ctx = caterva_new_ctx(NULL, NULL, BLOSC2_CPARAMS_DEFAULTS, BLOSC2_DPARAMS_DEFAULTS);
    ctx->cparams.typesize = sizeof(double);

    // Define the pshape for the first array
    int8_t ndim = 3;
    int64_t pshape_[] = {3, 2, 4};
    caterva_dims_t pshape = caterva_new_dims(pshape_, ndim);

    // Create an on-disk frame
    blosc2_frame* frame = &(blosc2_frame) {
        .fname = "persistency.caterva",
    };

    // Create the first array (empty)
    caterva_array_t *cat1 = caterva_empty_array(ctx, frame, &pshape);

    // Define a buffer shape to fill cat1
    int64_t shape_[] = {10, 10, 10};
    caterva_dims_t shape = caterva_new_dims(shape_, ndim);

    // Create a buffer to fill cat1 and empty it with an arange
    int64_t buf1size = 1;
    for (int i = 0; i < shape.ndim; ++i) {
        buf1size *= shape.dims[i];
    }
    double *buf1 = (double *) malloc(buf1size * sizeof(double));

    for (int64_t k = 0; k < buf1size; ++k) {
        buf1[k] = (double) k;
    }

    // Fill cat1 with the above buffer
    caterva_from_buffer(cat1, &shape, buf1);

    // Close cat1 and reopen the caterva frame persisted on-disk on cat3 and operate with it
    caterva_free_array(cat1);
    caterva_array_t* cat3 = caterva_from_file(ctx, "persistency.caterva");

    // Apply a `get_slice` to cat3 and store it into cat2
    int64_t start_[] = {3, 6, 4};
    caterva_dims_t start = caterva_new_dims(start_, ndim);
    int64_t stop_[] = {4, 9, 8};
    caterva_dims_t stop = caterva_new_dims(stop_, ndim);

    int64_t pshape2_[]  = {1, 2, 3};
    caterva_dims_t pshape2 = caterva_new_dims(pshape2_, ndim);
    caterva_array_t *cat2 = caterva_empty_array(ctx, NULL, &pshape2);

    caterva_get_slice(cat2, cat3, &start, &stop);
    caterva_squeeze(cat2);

    // Assert that the `squeeze` works well
    if (cat3->ndim == cat2->ndim) {
        return -1;
    }

    // Create a buffer to store the cat2 elements
    int64_t buf2size = 1;
    caterva_dims_t shape2 = caterva_get_shape(cat2);
    for (int j = 0; j < shape2.ndim; ++j) {
        buf2size *= shape2.dims[j];
    }
    double *buf2 = (double *) malloc(buf2size * sizeof(double));

    // Fill buffer with the cat2 content
    caterva_to_buffer(cat2, buf2);

    // Print results
    printf("The resulting hyperplane is:\n");

    for (int64_t i = 0; i < shape2.dims[0]; ++i) {
        for (int64_t j = 0; j < shape2.dims[1]; ++j) {
            printf("%6.f", buf2[i * cat2->shape[1] + j]);
        }
        printf("\n");
    }

    free(buf1);
    free(buf2);
    caterva_free_array(cat2);
    caterva_free_array(cat3);

    return 0;
}
