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

#ifndef CATERVA_CATERVA_PLAINBUFFER_H_
#define CATERVA_CATERVA_PLAINBUFFER_H_

int caterva_plainbuffer_array_empty(caterva_ctx_t *ctx, caterva_params_t *params,
                                    caterva_storage_t *storage, caterva_array_t **array);

int caterva_plainbuffer_array_zeros(caterva_ctx_t *ctx, caterva_params_t *params,
                                    caterva_storage_t *storage, caterva_array_t **array);

int caterva_plainbuffer_array_full(caterva_ctx_t *ctx, caterva_params_t *params,
                                   caterva_storage_t *storage, void *fill_value,
                                   caterva_array_t **array);

int caterva_plainbuffer_array_free(caterva_ctx_t *ctx, caterva_array_t **array);

int caterva_plainbuffer_array_set_slice_buffer(caterva_ctx_t *ctx,
                                               void *buffer, int64_t buffersize,
                                               int64_t *start, int64_t *stop, int64_t *shape,
                                               caterva_array_t *array);

int caterva_plainbuffer_array_get_slice_buffer(caterva_ctx_t *ctx, caterva_array_t *array,
                                               int64_t *start, int64_t *stop, int64_t *shape,
                                               void *buffer, int64_t buffersize);

int caterva_plainbuffer_array_to_buffer(caterva_ctx_t *ctx, caterva_array_t *array, void *buffer,
                                        int64_t buffersize);

int caterva_plainbuffer_array_get_slice(caterva_ctx_t *ctx, caterva_array_t *src,
                                        int64_t *start, int64_t *stop,
                                        caterva_storage_t *storage, caterva_array_t **array);

int caterva_plainbuffer_array_squeeze_index(caterva_ctx_t *ctx, caterva_array_t *array,
                                            bool *index);

int caterva_plainbuffer_array_squeeze(caterva_ctx_t *ctx, caterva_array_t *array);

int caterva_plainbuffer_array_copy(caterva_ctx_t *ctx, caterva_params_t *params,
                                   caterva_storage_t *storage, caterva_array_t *src,
                                   caterva_array_t **dest);

#endif  // CATERVA_CATERVA_PLAINBUFFER_H_
