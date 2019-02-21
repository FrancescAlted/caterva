/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

/** @file caterva.h
 * @brief Caterva header file.
 *
 * This file contains Caterva public API and the structures needed to use it.
 * @author Francesc Alted <francesc@blosc.org>
 * @author Aleix Alcacer <aleixalcacer@gmail.com>
 */

#ifndef CATERVA_HEADER_FILE
#define CATERVA_HEADER_FILE

#include <stdio.h>
#include <stdlib.h>
#include <blosc.h>

#define CATERVA_MAXDIM 8

/**
 * @brief The parameters for creating a context for caterva containers.
 *
 * In parenthesis it is shown the default value used internally when a NULL value is passed to the
 * constructor.
 */
typedef struct {
    void *(*alloc)(size_t);
    //!< The allocation memory function used internally (malloc)
    void (*free)(void *);
    //!< The free memory function used internally (free)
    blosc2_cparams cparams;
    //!< The blosc compression params used
    blosc2_dparams dparams;
    //!< The blosc decompression params used
} caterva_ctx_t;

/**
 * @brief Represents a shape or a point of a caterva container.
 *
 */
typedef struct {
    uint64_t dims[CATERVA_MAXDIM];
    //!< The dimensions number
    uint8_t ndim;
    //!< The values of each dimension
} caterva_dims_t;

/**
 * @brief Default struct for a caterva shape
 */
static const caterva_dims_t CATERVA_DIMS_DEFAULTS = {
    .dims = {1, 1, 1, 1, 1, 1, 1, 1},
    .ndim = 1
};

/**
 * @brief An optional cache for a single partition
 *
 */
typedef struct part_cache_s {
    uint8_t *data;
    //!< The buffer where a Blosc chunk is decompressed
    int32_t nchunk;
    //!< The ndex of the Blosc chunk stored in @p data
};

typedef struct {
    caterva_ctx_t *ctx;  /* caterva context */
    blosc2_schunk *sc;
    uint64_t shape[CATERVA_MAXDIM];  /* shape of original data */
    uint64_t pshape[CATERVA_MAXDIM];  /* shape of each chunk */
    uint64_t eshape[CATERVA_MAXDIM];  /* shape of schunk */
    uint64_t size;  /* size of original data */
    uint64_t psize;  /* size of each chunnk */
    uint64_t esize;  /* shape of schunk */
    uint8_t ndim;  /* data dimensions */
    struct part_cache_s part_cache;
} caterva_array_t;

caterva_ctx_t *caterva_new_ctx(void *(*all)(size_t), void (*free)(void *), blosc2_cparams cparams, blosc2_dparams dparams);

caterva_dims_t caterva_new_dims(uint64_t *dims, uint8_t ndim);

caterva_array_t *caterva_empty_array(caterva_ctx_t *ctx, blosc2_frame *fr, caterva_dims_t pshape);

caterva_array_t *caterva_from_file(caterva_ctx_t *ctx, const char *filename);

int caterva_free_ctx(caterva_ctx_t *ctx);

int caterva_free_array(caterva_array_t *carr);

int caterva_update_shape(caterva_array_t *carr, caterva_dims_t shape);

int caterva_from_buffer(caterva_array_t *dest, caterva_dims_t shape, void *src);

int caterva_fill(caterva_array_t *dest, caterva_dims_t shape, void *value);

int caterva_to_buffer(caterva_array_t *src, void *dest);

int caterva_get_slice_buffer( void *dest, caterva_array_t *src, caterva_dims_t start, caterva_dims_t stop,
                             caterva_dims_t d_pshape);

int caterva_get_slice(caterva_array_t *dest, caterva_array_t *src, caterva_dims_t start, caterva_dims_t stop);

int caterva_repart(caterva_array_t *dest, caterva_array_t *src);

int caterva_squeeze(caterva_array_t *src);

caterva_dims_t caterva_get_shape(caterva_array_t *src);

caterva_dims_t caterva_get_pshape(caterva_array_t *src);

int caterva_equal_data(caterva_array_t *a, caterva_array_t *b);

#endif
