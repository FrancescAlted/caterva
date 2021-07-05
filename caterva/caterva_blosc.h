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

int caterva_blosc_array_empty(caterva_ctx_t *ctx, caterva_params_t *params,
                              caterva_storage_t *storage, caterva_array_t **array);
int caterva_blosc_array_zeros(caterva_ctx_t *ctx, caterva_params_t *params,
                              caterva_storage_t *storage, caterva_array_t **array);

int caterva_blosc_array_full(caterva_ctx_t *ctx, caterva_params_t *params,
                             caterva_storage_t *storage, void *fill_value,
                             caterva_array_t **array);

int caterva_blosc_array_free(caterva_ctx_t *ctx, caterva_array_t **array);

int caterva_blosc_from_schunk(caterva_ctx_t *ctx, blosc2_schunk *schunk, caterva_array_t **array);

int caterva_blosc_from_serial_schunk(caterva_ctx_t *ctx, uint8_t *serial_schunk, int64_t len,
                                     caterva_array_t **array);

int caterva_blosc_open(caterva_ctx_t *ctx, const char *urlpath, caterva_array_t **array);

int caterva_blosc_save(caterva_ctx_t *ctx, caterva_array_t *array, char *urlpath);

int caterva_blosc_array_set_slice_buffer(caterva_ctx_t *ctx,
                                         void *buffer, int64_t buffersize,
                                         int64_t *start, int64_t *stop, int64_t *shape,
                                         caterva_array_t *array);

int caterva_blosc_array_get_slice_buffer(caterva_ctx_t *ctx, caterva_array_t *array,
                                         int64_t *start, int64_t *stop, int64_t *shape,
                                         void *buffer, int64_t buffersize);

int caterva_blosc_array_to_buffer(caterva_ctx_t *ctx, caterva_array_t *array, void *buffer,
                                  int64_t buffersize);

int caterva_blosc_array_get_slice(caterva_ctx_t *ctx, caterva_array_t *src,
                                  int64_t *start, int64_t *stop,
                                  caterva_storage_t *storage, caterva_array_t **array);

int caterva_blosc_array_squeeze_index(caterva_ctx_t *ctx, caterva_array_t *src,
                                      bool *index);

int caterva_blosc_array_squeeze(caterva_ctx_t *ctx, caterva_array_t *src);

int caterva_blosc_array_copy(caterva_ctx_t *ctx, caterva_params_t *params,
                             caterva_storage_t *storage, caterva_array_t *src,
                             caterva_array_t **dest);

int caterva_blosc_remove(caterva_ctx_t *ctx, char *urlpath);

int caterva_blosc_vlmeta_add(caterva_ctx_t *ctx, caterva_array_t *array,
                             const char *name, uint8_t *content, int32_t content_len);

int caterva_blosc_vlmeta_get(caterva_ctx_t *ctx, caterva_array_t *array,
                             const char *name, uint8_t **content, int32_t *content_len);

int caterva_blosc_vlmeta_exists(caterva_ctx_t *ctx, caterva_array_t *array,
                                const char *name, bool *exists);

int caterva_blosc_vlmeta_update(caterva_ctx_t *ctx, caterva_array_t *array,
                                const char *name, uint8_t *content, int32_t content_len);

int caterva_blosc_meta_get(caterva_ctx_t *ctx, caterva_array_t *array,
                             const char *name, uint8_t **content, int32_t *content_len);

int caterva_blosc_meta_exists(caterva_ctx_t *ctx, caterva_array_t *array,
                                const char *name, bool *exists);

int caterva_blosc_meta_update(caterva_ctx_t *ctx, caterva_array_t *array,
                                const char *name, uint8_t *content, int32_t content_len);

#endif  // CATERVA_CATERVA_BLOSC_H_
