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

#ifndef CATERVA_CATERVA_BLOSC_H_
#define CATERVA_CATERVA_BLOSC_H_

int caterva_blosc_array_empty(caterva_context_t *ctx, caterva_params_t *params,
                              caterva_storage_t *storage, caterva_array_t **array);

int caterva_blosc_array_free(caterva_context_t *ctx, caterva_array_t **array);

int caterva_blosc_from_frame(caterva_context_t *ctx, blosc2_frame *frame, bool copy,
                             caterva_array_t **array);

int caterva_blosc_from_sframe(caterva_context_t *ctx, uint8_t *sframe, int64_t len, bool copy,
                              caterva_array_t **array);

int caterva_blosc_from_file(caterva_context_t *ctx, const char *urlpath, bool copy,
                            caterva_array_t **array);

int caterva_blosc_array_repart_chunk(int8_t *rchunk, int64_t rchunksize, void *chunk,
                                     int64_t chunksize, caterva_array_t *array);

int caterva_blosc_array_append(caterva_context_t *ctx, caterva_array_t *array, void *chunk,
                               int64_t chunksize);

int caterva_blosc_array_from_buffer(caterva_context_t *ctx, caterva_array_t *array, void *buffer,
                                    int64_t buffersize);

int caterva_blosc_array_get_slice_buffer(caterva_context_t *ctx, caterva_array_t *array,
                                         int64_t *start, int64_t *stop, int64_t *shape,
                                         void *buffer);

int caterva_blosc_array_to_buffer(caterva_context_t *ctx, caterva_array_t *array, void *buffer);

int caterva_blosc_array_get_slice(caterva_context_t *ctx, caterva_array_t *src, int64_t *start,
                                  int64_t *stop, caterva_array_t *array);

int caterva_blosc_array_squeeze_index(caterva_context_t *ctx, caterva_array_t *src, bool *index);

int caterva_blosc_array_squeeze(caterva_context_t *ctx, caterva_array_t *src);

int caterva_blosc_array_copy(caterva_context_t *ctx, caterva_params_t *params,
                             caterva_storage_t *storage, caterva_array_t *src,
                             caterva_array_t **dest);

#endif  // CATERVA_CATERVA_BLOSC_H_
