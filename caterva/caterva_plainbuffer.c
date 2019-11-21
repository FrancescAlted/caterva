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


int caterva_plainbuffer_free_array(caterva_array_t *carr) {

    if (carr->buf != NULL) {
        carr->ctx->free(carr->buf);
    }
    return 0;
}


int caterva_plainbuffer_append(caterva_array_t *carr, void *part, int64_t partsize) {
    if (carr->nparts == 0) {
        carr->buf = malloc(carr->size * (size_t) carr->ctx->cparams.typesize);
    } else {
        carr->nparts = 0;
    }
    int64_t start_[CATERVA_MAXDIM], stop_[CATERVA_MAXDIM];
    for (int i = 0; i < carr->ndim; ++i) {
        start_[i] = 0;
        stop_[i] = start_[i] + carr->pshape[i];
    }
    caterva_dims_t start = caterva_new_dims(start_, carr->ndim);
    caterva_dims_t stop = caterva_new_dims(stop_, carr->ndim);
    caterva_set_slice_buffer(carr, part, &start, &stop);

    return 0;
}
