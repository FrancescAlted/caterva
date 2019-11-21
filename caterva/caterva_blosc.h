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

caterva_array_t *caterva_blosc_from_frame(caterva_ctx_t *ctx, blosc2_frame *frame, bool copy);

caterva_array_t *caterva_blosc_from_sframe(caterva_ctx_t *ctx, uint8_t *sframe, int64_t len, bool copy);

caterva_array_t *caterva_blosc_from_file(caterva_ctx_t *ctx, const char *filename, bool copy);

int caterva_blosc_free_array(caterva_array_t *carr);

int caterva_blosc_append(caterva_array_t *carr, void *part, int64_t partsize);

int caterva_blosc_from_buffer(caterva_array_t *dest, caterva_dims_t *shape, const void *src);

int caterva_blosc_to_buffer(caterva_array_t *src, void *dest);

int caterva_blosc_get_slice_buffer(void *dest, caterva_array_t *src, caterva_dims_t *start,
                                   caterva_dims_t *stop, caterva_dims_t *d_pshape);

int caterva_blosc_get_slice(caterva_array_t *dest, caterva_array_t *src, caterva_dims_t *start,
                            caterva_dims_t *stop);

#endif //CATERVA_CATERVA_BLOSC_H
