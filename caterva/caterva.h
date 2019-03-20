/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#ifndef CATERVA_HEADER_FILE
#define CATERVA_HEADER_FILE

#include <stdio.h>
#include <stdlib.h>
#include <blosc.h>

#define CATERVA_MAXDIM 8

typedef enum {
    CATERVA_TYPE_BLOSC,
    CATERVA_TYPE_PLAINBUFFER,
} caterva_type_t;

typedef struct {
    void *(*alloc)(size_t);

    void (*free)(void *);

    blosc2_cparams cparams;
    blosc2_dparams dparams;
} caterva_ctx_t;

typedef struct {
    int64_t dims[CATERVA_MAXDIM];  /* the shape of each chunk */
    int8_t ndim;  /* data dimensions */
} caterva_dims_t;

static const caterva_dims_t CATERVA_DIMS_DEFAULTS = {
    .dims = {1, 1, 1, 1, 1, 1, 1, 1},
    .ndim = 1
};

/* An *optional* cache for a single partition */
struct part_cache_s {
    uint8_t *data;
    int32_t nchunk;
};

typedef struct {
    caterva_ctx_t *ctx;  /* caterva context */
    caterva_type_t type;
    blosc2_schunk *sc;
    uint8_t *buf;
    int64_t shape[CATERVA_MAXDIM];  /* shape of original data */
    int64_t pshape[CATERVA_MAXDIM];  /* shape of each chunk */
    int64_t eshape[CATERVA_MAXDIM];  /* shape of schunk */
    int64_t size;  /* size of original data */
    int64_t psize;  /* size of each chunnk */
    int64_t esize;  /* shape of schunk */
    int8_t ndim;  /* data dimensions */
    struct part_cache_s part_cache;
} caterva_array_t;

caterva_ctx_t *caterva_new_ctx(void *(*all)(size_t), void (*free)(void *), blosc2_cparams cparams, blosc2_dparams dparams);

caterva_dims_t caterva_new_dims(int64_t *dims, int8_t ndim);

caterva_array_t *caterva_empty_array(caterva_ctx_t *ctx, blosc2_frame *fr, caterva_dims_t *pshape);

caterva_array_t *caterva_from_file(caterva_ctx_t *ctx, const char *filename);

int caterva_free_ctx(caterva_ctx_t *ctx);

int caterva_free_array(caterva_array_t *carr);

int caterva_update_shape(caterva_array_t *carr, caterva_dims_t *shape);

int caterva_from_buffer(caterva_array_t *dest, caterva_dims_t *shape, void *src);

int caterva_fill(caterva_array_t *dest, caterva_dims_t *shape, void *value);

int caterva_to_buffer(caterva_array_t *src, void *dest);

int caterva_get_slice_buffer( void *dest, caterva_array_t *src, caterva_dims_t *start, caterva_dims_t *stop,
                             caterva_dims_t *d_pshape);

int caterva_get_slice(caterva_array_t *dest, caterva_array_t *src, caterva_dims_t *start, caterva_dims_t *stop);

int caterva_repart(caterva_array_t *dest, caterva_array_t *src);

int caterva_squeeze(caterva_array_t *src);

caterva_dims_t caterva_get_shape(caterva_array_t *src);

caterva_dims_t caterva_get_pshape(caterva_array_t *src);

#endif
