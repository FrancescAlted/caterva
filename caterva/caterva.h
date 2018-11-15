/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#ifndef CATERVA_HEADER_FILE
#define CATERVA_HEADER_FILE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <blosc.h>

#define CATERVA_MAXDIM 8

typedef struct {
    void *(*alloc)(size_t);

    void (*free)(void *);
} caterva_ctx;

typedef struct {
    size_t dims[CATERVA_MAXDIM];  /* the shape of each chunk */
    size_t ndim;  /* data dimensions */
} caterva_dims;


static const caterva_dims CATERVA_DIMS_DEFAULTS = {
    .dims = {1, 1, 1, 1, 1, 1, 1, 1},
    .ndim = 1
};

typedef struct {
    caterva_ctx *ctx;  /* caterva context */
    blosc2_schunk *sc;
    size_t shape[CATERVA_MAXDIM];  /* shape of original data */
    size_t pshape[CATERVA_MAXDIM];  /* shape of each chunk */
    size_t eshape[CATERVA_MAXDIM];  /* shape of schunk */
    size_t size;  /* size of original data */
    size_t csize;  /* size of each chunnk */
    size_t esize;  /* shape of schunk */
    size_t ndim;  /* data dimensions */
} caterva_array;

caterva_ctx *caterva_new_ctx(void *(*all)(size_t), void (*free)(void *));

caterva_dims caterva_new_dims(size_t *dims, size_t ndim);

caterva_array *caterva_empty_array(caterva_ctx *ctx, blosc2_cparams cp, blosc2_dparams dp, blosc2_frame *fr, caterva_dims pshape);

int caterva_free_ctxt(caterva_ctx *ctx);

int caterva_free_array(caterva_array *carr);

int _caterva_update_array(caterva_array *dest, caterva_dims shape);

int caterva_from_buffer(caterva_array *dest, caterva_dims shape, void *src);

int caterva_to_buffer(caterva_array *src, void *dest);

int _caterva_get_slice(caterva_array *src, void *dest, size_t *start, size_t *stop);

int caterva_get_slice(caterva_array *dest, caterva_array *src, caterva_dims start, caterva_dims stop);

int caterva_equal_data(caterva_array *a, caterva_array *b);

#endif
