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


/** @file caterva.h
 * @brief Caterva header file.
 *
 * This file contains Caterva public API and the structures needed to use it.
 * @author Blosc Development team <blosc@blosc.org>
 */

#ifndef CATERVA_HEADER_FILE
#define CATERVA_HEADER_FILE

#include <stdio.h>
#include <stdlib.h>
#include <blosc.h>

#define CATERVA_MAXDIM 8


/**
 * @brief Formats to store #caterva_array_t data.
 */

typedef enum {
    CATERVA_STORAGE_BLOSC,
    //!< Indicates that data is stored using a Blosc superchunk.
    CATERVA_STORAGE_PLAINBUFFER,
    //!< Indicates that data is stored using a plain buffer.
} caterva_storage_t;


/**
 * @brief The parameters for creating a context for caterva arrays.
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
 * @brief Struct that represents dimensions in Caterva
 */

typedef struct {
    int64_t dims[CATERVA_MAXDIM];
    //!< The size of each dimension
    int8_t ndim;
    //!< The number of dimensions
} caterva_dims_t;


/*
 * @brief Default Caterva dimensions
 */

//static const caterva_dims_t CATERVA_DIMS_DEFAULTS = {
//    .dims = {1, 1, 1, 1, 1, 1, 1, 1},
//    .ndim = 1
//};


/**
 * @brief An *optional* cache for a single block.
 *
 * When a block is needed, it is copied into this cache. In this way, if the same block is needed
 * again afterwards, it is not necessary to recover it because it is already in the cache.
 */

struct part_cache_s {
    uint8_t *data;
    //!< Pointer to the block data.
    int32_t nchunk;
    //!< The block number in cache. If @p nchunk equals to -1, it means that the cache is empty.
};


/**
 * @brief A multidimensional container that allows compressed data.
 */

typedef struct {
    caterva_ctx_t *ctx;
    //!< Caterva context
    caterva_storage_t storage;
    //!< Storage type.
    blosc2_schunk *sc;
    //!< Pointer to a Blosc superchunk.
    //!< Only is used if \p storage equals to \p #CATERVA_STORAGE_BLOSC.
    uint8_t *buf;
    //!< Pointer to a plain buffer where data is stored.
    //!< Only is used if \p storage equals to \p #CATERVA_STORAGE_PLAINBUFFER.
    int64_t shape[CATERVA_MAXDIM];
    //!< Shape of original data.
    int64_t pshape[CATERVA_MAXDIM];
    //!< Shape of each block.
    int64_t eshape[CATERVA_MAXDIM];
    //!< Shape of padded data.
    int64_t size;
    //!< Size of original data.
    int64_t psize;
    //!< Size of each block.
    int64_t esize;
    //!< Size of padded data.
    int8_t ndim;
    //!< Data dimensions.
    struct part_cache_s part_cache;
    //!< A block cache.
} caterva_array_t;


/**
 * @brief Create a context for Caterva functions.
 *
 * @param all The allocation function to use internally. If it is NULL, malloc is used.
 * @param free The free function to use internally. If it is NULL, free is used.
 * @param cparams The compression parameters used when a Caterva array is created.
 * @param dparams The decompression parameters used when data of a Caterva array is decompressed.
 *
 * @return A pointer to the new Caterva context. NULL is returned if this fails.
 */

caterva_ctx_t *caterva_new_ctx(void *(*all)(size_t), void (*free)(void *), blosc2_cparams cparams, blosc2_dparams dparams);


/**
 * @brief Free a Caterva context
 *
 * @param ctx A caterva context
 *
 * @return An error code
 */

int caterva_free_ctx(caterva_ctx_t *ctx);


/**
 * @brief Create a Caterva dimension object
 *
 * @param dims The size of each dimension
 * @param ndim The number of dimensions
 *
 * @return A caterva dimension object
 */

caterva_dims_t caterva_new_dims(int64_t *dims, int8_t ndim);


/**
 * @brief Create a caterva empty array.
 *
 * If \p pshape is \c NULL, the data container will be stored using a plain buffer.
 *
 * However, if \p pshape is not \c NULL, the data container will be stored using a superchunk.
 * In particular, if \p fr is \c NULL it will be stored in the memory and if it is not \c NULL
 * it will be stored on disk.
 *
 * @param ctx The context to be used
 * @param fr A frame to store data on disk.
 * @param pshape The pshape of each block.
 *
 * @return A pointer to a caterva array
 */

caterva_array_t *caterva_empty_array(caterva_ctx_t *ctx, blosc2_frame *fr, caterva_dims_t *pshape);


/**
 * @brief Free a caterva container
 *
 * @param carr A caterva container
 *
 * @return An error code
 */

int caterva_free_array(caterva_array_t *carr);


/**
 * @brief Read a caterva array from disk.
 *
 * @param ctx The caterva context
 * @param filename The filename of the caterva array in disk
 *
 * @return A pointer to a caterva array
 */

caterva_array_t *caterva_from_file(caterva_ctx_t *ctx, const char *filename);


/**
 * @brief Create a caterva array from a plain buffer data
 *
 * @param dest An empty caterva array
 * @param shape The data shape
 * @param src A pointer to data buffer
 *
 * @return An error code
 */

int caterva_from_buffer(caterva_array_t *dest, caterva_dims_t *shape, void *src);


/**
 * @brief Fill a caterva array with a value
 *
 * @param dest An empty caterva array
 * @param shape The data shape
 * @param value The value with which the container is going to be filled
 *
 * @return An error code
 */

int caterva_fill(caterva_array_t *dest, caterva_dims_t *shape, void *value);

/**
 * @brief Extract the data from a container to a buffer
 *
 * @param src The caterva array
 * @param dest The buffer where data will be stored
 *
 * @return An error code
 */

int caterva_to_buffer(caterva_array_t *src, void *dest);

int caterva_get_slice_buffer(void *dest, caterva_array_t *src, caterva_dims_t *start,
                             caterva_dims_t *stop, caterva_dims_t *d_pshape);

int caterva_get_slice_buffer_no_copy(void **dest, caterva_array_t *src, caterva_dims_t *start,
                                     caterva_dims_t *stop, caterva_dims_t *d_pshape);

int caterva_set_slice_buffer(caterva_array_t *dest, void *src, caterva_dims_t *start,
                             caterva_dims_t *stop);

int caterva_get_slice(caterva_array_t *dest, caterva_array_t *src, caterva_dims_t *start, caterva_dims_t *stop);

int caterva_repart(caterva_array_t *dest, caterva_array_t *src);

int caterva_squeeze(caterva_array_t *src);


/**
 * @brief Get the shape of a caterva array
 *
 * @param src A caterva array
 *
 * @return The shape of the caterva array
 */

caterva_dims_t caterva_get_shape(caterva_array_t *src);


/**
 * @brief Get the block shape of a caterva array
 *
 * @param src A caterva array
 *
 * @return The block shape of the caterva array
 */

caterva_dims_t caterva_get_pshape(caterva_array_t *src);

#endif
