/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#ifndef CATERVA_CATERVA_PLAINBUFFER_H
#define CATERVA_CATERVA_PLAINBUFFER_H


int caterva_plainbuffer_array_empty(caterva_context_t *ctx, caterva_params_t *params,
                                    caterva_storage_t *storage, caterva_array_t **array);

int caterva_plainbuffer_array_free(caterva_context_t *ctx, caterva_array_t **array);


int caterva_plainbuffer_array_append(caterva_context_t *ctx, caterva_array_t *array, void *chunk, int64_t chunksize);

int caterva_plainbuffer_array_from_buffer(caterva_context_t *ctx, caterva_array_t *array, void *buffer, int64_t buffersize);

int caterva_plainbuffer_array_to_buffer(caterva_context_t *ctx, caterva_array_t *array, void *buffer);

int caterva_plainbuffer_array_get_slice_buffer(caterva_context_t *ctx, caterva_array_t *array, int64_t *start, int64_t *stop,
                                               int64_t *shape, void *buffer);

int caterva_plainbuffer_set_slice_buffer(caterva_array_t *dest, void *src, caterva_dims_t *start,
                                         caterva_dims_t *stop);

int caterva_plainbuffer_get_slice(caterva_array_t *dest, caterva_array_t *src,
                                  caterva_dims_t *start, caterva_dims_t *stop);

int caterva_plainbuffer_squeeze(caterva_array_t *src);

int caterva_plainbuffer_copy(caterva_array_t *dest, caterva_array_t *src);

int caterva_plainbuffer_update_shape(caterva_array_t *carr, caterva_dims_t *shape);

#endif //CATERVA_CATERVA_PLAINBUFFER_H
