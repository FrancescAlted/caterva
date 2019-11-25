/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#ifndef CATERVA_CATERVA_PLAINBUFFER_H
#define CATERVA_CATERVA_PLAINBUFFER_H


int caterva_plainbuffer_free_array(caterva_array_t *carr);

int caterva_plainbuffer_append(caterva_array_t *carr, void *part, int64_t partsize);

int caterva_plainbuffer_from_buffer(caterva_array_t *dest, caterva_dims_t *shape, const void *src);

int caterva_plainbuffer_to_buffer(caterva_array_t *src, void *dest);

int caterva_plainbuffer_get_slice_buffer(void *dest, caterva_array_t *src, caterva_dims_t *start,
                                         caterva_dims_t *stop, caterva_dims_t *d_pshape);

int caterva_plainbuffer_set_slice_buffer(caterva_array_t *dest, void *src, caterva_dims_t *start,
                                         caterva_dims_t *stop);

int caterva_plainbuffer_get_slice(caterva_array_t *dest, caterva_array_t *src,
                                  caterva_dims_t *start, caterva_dims_t *stop);

int caterva_plainbuffer_squeeze(caterva_array_t *src);

int caterva_plainbuffer_copy(caterva_array_t *dest, caterva_array_t *src);

#endif //CATERVA_CATERVA_PLAINBUFFER_H
