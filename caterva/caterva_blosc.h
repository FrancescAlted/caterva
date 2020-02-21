/*
 * Copyright (C) 2018-present Francesc Alted, Aleix Alcacer.
 * Copyright (C) 2019-present Blosc Development team <blosc@blosc.org>
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */


#ifndef CATERVA_CATERVA_BLOSC_H
#define CATERVA_CATERVA_BLOSC_H

int caterva_blosc_array_empty(caterva_context_t *ctx, caterva_params_t *params, caterva_storage_t *storage,
                              caterva_array_t **array);

int caterva_blosc_array_free(caterva_context_t *ctx, caterva_array_t **array);


int caterva_blosc_from_frame(caterva_context_t *ctx, blosc2_frame *frame, bool copy, caterva_array_t **array);

int caterva_blosc_from_sframe(caterva_context_t *ctx, uint8_t *sframe, int64_t len, bool copy, caterva_array_t **array);

int caterva_blosc_from_file(caterva_context_t *ctx, const char *filename, bool copy, caterva_array_t **array);


int caterva_blosc_array_append(caterva_context_t *ctx, caterva_array_t *array, void *chunk, int64_t chunksize);

int caterva_blosc_from_buffer(caterva_array_t *dest, caterva_dims_t *shape, const void *src);

int caterva_blosc_to_buffer(caterva_array_t *src, void *dest);

int caterva_blosc_get_slice_buffer(void *dest, caterva_array_t *src, caterva_dims_t *start,
                                   caterva_dims_t *stop, caterva_dims_t *d_pshape);

int caterva_blosc_get_slice(caterva_array_t *dest, caterva_array_t *src, caterva_dims_t *start,
                            caterva_dims_t *stop);

int caterva_blosc_squeeze(caterva_array_t *src);

int caterva_blosc_copy(caterva_array_t *dest, caterva_array_t *src);

int caterva_blosc_update_shape(caterva_array_t *carr, caterva_dims_t *shape);

#endif //CATERVA_CATERVA_BLOSC_H
