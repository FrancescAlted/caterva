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

#define CATERVA_UNUSED_PARAM(x) ((void)(x))

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
 * @brief A context for caterva containers.
 *
 * In parenthesis it is shown the default value used internally when a \c NULL value is passed to the
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
 * @brief A dimensions vector that can represent shapes or points
 */

typedef struct {
    int64_t dims[CATERVA_MAXDIM];
    //!< The size of each dimension
    int8_t ndim;
    //!< The number of dimensions
} caterva_dims_t;


/**
 * @brief Default caterva dimensions vector
 */

static const caterva_dims_t CATERVA_DIMS_DEFAULTS = {
    .dims = {1, 1, 1, 1, 1, 1, 1, 1},
    .ndim = 1
};


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
    //!< Storage type
    blosc2_schunk *sc;
    //!< Pointer to a Blosc superchunk
    //!< Only is used if \p storage equals to \p #CATERVA_STORAGE_BLOSC
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
    bool empty;
    //!< Indicate if an array is empty or is filled with data.
    bool filled;
    //!< Indicate if an array is filled completely or not.
    int64_t nblocks;
    //!< Number of blocks append to the array.
    struct part_cache_s part_cache;
    //!< A block cache.

} caterva_array_t;


/**
 * @brief Create a context for Caterva functions.
 *
 * @param all The allocation function to use internally. If it is \c NULL, malloc is used.
 * @param free The free function to use internally. If it is \c NULL, free is used.
 * @param cparams The compression parameters used when a caterva container is created.
 * @param dparams The decompression parameters used when data of a caterva container is decompressed.
 *
 * @return A pointer to the new caterva context. \p NULL is returned if this fails.
 */

caterva_ctx_t *caterva_new_ctx(void *(*all)(size_t), void (*free)(void *), blosc2_cparams cparams, blosc2_dparams dparams);


/**
 * @brief Free a caterva context
 *
 * @param ctx Pointer to the context to be freed
 *
 * @return An error code
 */

int caterva_free_ctx(caterva_ctx_t *ctx);


/**
 * @brief Create a caterva dimensions vector
 *
 * A #caterva_dims_t can be used to represent shapes or points of the container.
 *
 * @param dims The size of each dimension
 * @param ndim The number of dimensions
 *
 * @return The caterva dimensions vector created
 */

caterva_dims_t caterva_new_dims(int64_t *dims, int8_t ndim);


/**
 * @brief Create a caterva empty container
 *
 * When a container is created, only the block shape, the storage type and the context are defined.
 * It should be noted that the shape is defined when a container is filled and not when it is created.
 *
 * If \p pshape is \c NULL, the data container will be stored using a plain buffer.
 *
 * However, if \p pshape is not \c NULL, the data container will be stored using a superchunk.
 * In particular, if \p fr is \c NULL it will be stored in the memory and if it is not \c NULL
 * it will be stored on disk.
 *
 * @param ctx Pointer to the caterva context to be used
 * @param fr Pointer to the blosc frame used to store data on disk
 * @param pshape The shape of each block
 *
 * @return A pointer to the empty caterva container created
 */

caterva_array_t *caterva_empty_array(caterva_ctx_t *ctx, blosc2_frame *fr, caterva_dims_t *pshape);


/**
 * @brief Free a caterva container
 *
 * @param carr Pointer to the container to be freed
 *
 * @return An error code
 */

int caterva_free_array(caterva_array_t *carr);


/**
 * Append a block to a caterva container
 * @param carr
 * @param part
 * @param partsize
 * @return
 */

int caterva_append(caterva_array_t *carr, void *part, int64_t partsize);


/**
 * @brief Read a caterva container from disk
 *
 * @param ctx Pointer to the caterva context to be used. The context should be the same as the one used to create the array.

 * @param filename The filename of the caterva container in disk
 *
 * @return A pointer to the caterva container read from disk
 */

caterva_array_t *caterva_from_file(caterva_ctx_t *ctx, const char *filename);


/**
 * @brief Create a caterva container from the data obtained in a C buffer
 *
 * @param dest Pointer to the container that will be created with the buffer data
 * @param shape The shape of the buffer data
 * @param src A pointer to the C buffer where data is stored
 *
 * @return An error code
 */

int caterva_from_buffer(caterva_array_t *dest, caterva_dims_t *shape, void *src);


/**
 * @brief Fill a caterva container with a value
 *
 * @param dest Pointer to the container that is filled
 * @param shape The shape of the container
 * @param value The value with which the container is going to be filled
 *
 * @return An error code
 */

int caterva_fill(caterva_array_t *dest, caterva_dims_t *shape, void *value);


/**
 * @brief Extract the data into a C buffer from a caterva container
 *
 * @param src Pointer to the container from which the data will be obtained
 * @param dest Pointer to the buffer where data will be stored
 *
 * @return An error code
 */

int caterva_to_buffer(caterva_array_t *src, void *dest);


/**
 * @brief Get a slice into an empty caterva container from another caterva container
 *
 * @param dest Pointer to the empty container where the obtained slice will be stored
 * @param src Pointer to the container from which the slice will be obtained
 * @param start The coordinates where the slice will begin
 * @param stop The coordinates where the slice will end
 * @return
 */

int caterva_get_slice(caterva_array_t *dest, caterva_array_t *src, caterva_dims_t *start, caterva_dims_t *stop);


/**
 * @brief Change the block of a caterva container
 *
 * It can only be used if the container is based on a blosc superchunk since it is the only one
 * that has the concept of block.
 *
 * @param dest Pointer to the empty container with the new block shape.
 * @param src Pointer to the container to be reblocked.
 *
 * @return An error code
 */

int caterva_repart(caterva_array_t *dest, caterva_array_t *src);


/**
 * @brief Squeeze a caterva container
 *
 * This function remove single-dimensional entries from the shape of a caterva container.
 *
 * @param src Pointer to the container to be squeezed
 *
 * @return An error code
 */

int caterva_squeeze(caterva_array_t *src);


/**
 * @brief Get a slice into a C buffer from a caterva container
 *
 * @param dest Pointer to the buffer where data will be stored
 * @param src Pointer to the container from which the slice will be extracted
 * @param start The coordinates where the slice will begin
 * @param stop The coordinates where the slice will end
 * @param d_pshape ?
 * @return An error code
 */

int caterva_get_slice_buffer(void *dest, caterva_array_t *src, caterva_dims_t *start,
                             caterva_dims_t *stop, caterva_dims_t *d_pshape);


/**
 * @brief Get a slice (without copy) into a C pointer from a caterva container
 *
 * With this function the data is not copied, thus allowing a higher speed.
 *
 * It can only be used if the container is based on a buffer. Also, the slice obtained should
 * not be modified since it is a reference to the container data. If the slice is modified,
 * the container will also be modified.
 *
 * @param dest Pointer to the C pointer where pointer data will be referenced
 * @param src Pointer to the container from which the slice will be extracted
 * @param start The coordinates where the slice will begin
 * @param stop The coordinates where the slice will end
 * @param d_pshape ?
 *
 * @return An error code
 */

int caterva_get_slice_buffer_no_copy(void **dest, caterva_array_t *src, caterva_dims_t *start,
                                     caterva_dims_t *stop, caterva_dims_t *d_pshape);


/**
 * @brief Set a slice into a caterva container from a C buffer
 *
 * It can only be used if the container is based on a buffer.
 *
 * @param dest Pointer to the caterva container where the partition will be set
 * @param src Pointer to the buffer where the slice data is
 * @param start The coordinates where the slice will begin
 * @param stop The coordinates where the slice will end
 *
 * @return An error code
 */

int caterva_set_slice_buffer(caterva_array_t *dest, void *src, caterva_dims_t *start,
                             caterva_dims_t *stop);


/**
 * @brief Update the shape of a caterva container.
 *
 * This is used when data is added to the container to update the shape
 *
 * @param src Pointer to the container from which the shape will be updated
 * @param shape The new shape of the container
 *
 * @return An error code
 */

int caterva_update_shape(caterva_array_t *src, caterva_dims_t *shape);

/**
 * @brief Get the shape of a caterva array
 *
 * @param src pointer to the container from which the block shape will be obtained
 *
 * @return The block shape of the caterva array
 */

caterva_dims_t caterva_get_shape(caterva_array_t *src);


/**
 * @brief Get the block shape of a caterva array
 *
 * @param src pointer to the container from which the block shape will be obtained
 *
 * @return The block shape of the caterva array
 */

caterva_dims_t caterva_get_pshape(caterva_array_t *src);

/**
 * @brief Make a copy of the container data.
 *
 * The copy is done into \p dest container
 *
 * @param dest Pointer to the container where data is copied
 * @param src Pointer to the container from which data is copied
 * @return
 */

int caterva_copy(caterva_array_t *dest, caterva_array_t *src);


#endif
