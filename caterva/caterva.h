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
#include <blosc2.h>


/* Version numbers */
#define CATERVA_VERSION_MAJOR    0    /* for major interface/format changes  */
#define CATERVA_VERSION_MINOR    2    /* for minor interface/format changes  */
#define CATERVA_VERSION_RELEASE  3    /* for tweaks, bug-fixes, or development */

#define CATERVA_VERSION_STRING   "0.2.3-dev"  /* string version.  Sync with above! */
#define CATERVA_VERSION_DATE     "2019-10-28"    /* date version */


/* Error handling */
#define CATERVA_SUCCEED 0
#define CATERVA_ERR_INVALID_ARGUMENT 1
#define CATERVA_ERR_BLOSC_FAILED 2
#define CATERVA_ERR_CONTAINER_FILLED 3
#define CATERVA_ERR_INVALID_STORAGE 4
#define CATERVA_ERR_NULL_POINTER 5

#ifdef NDEBUG
#define DEBUG_PRINT(...) do{ } while ( 0 )
#else
#define DEBUG_PRINT(...) do{ fprintf( stderr, "ERROR: %s (%s:%d)\n", __VA_ARGS__, __FILE__, __LINE__ ); } while( 0 )
#endif

#define CATERVA_ERROR(rc) do { if (rc != CATERVA_SUCCEED) { DEBUG_PRINT(print_error(rc)); return rc; }} while( 0 )
#define CATERVA_ERROR_NULL(pointer) do { if (pointer == NULL) { DEBUG_PRINT(print_error(CATERVA_ERR_NULL_POINTER)); return CATERVA_ERR_NULL_POINTER; }} while( 0 )

#define CATERVA_UNUSED_PARAM(x) ((void)(x))

static char *print_error(int rc) {
    switch (rc) {
        case CATERVA_ERR_INVALID_STORAGE: return "Invalid storage";
        case CATERVA_ERR_NULL_POINTER: return "Pointer is null";
        case CATERVA_ERR_BLOSC_FAILED: return "Blosc failed";
        default: return "Unknown error";
    }
}


/* The version for metalayer format; starts from 0 and it must not exceed 127 */
#define CATERVA_METALAYER_VERSION 0


/* The maximum number of dimensions for Caterva arrays */
#define CATERVA_MAXDIM 8


/**
 * @brief Configuration parameters used to create a Caterva context.
 */
struct caterva_config_s {
    void *(*alloc)(size_t);
    //!< The memory allocation function used internally.
    void (*free)(void *);
    //!< The memory release function used internally.
    int compcodec;
    //!< Defines the codec used in compression.
    int complevel;
    //!< Determines the compression level used in Blosc.
    int usedict;
    //!< Indicates whether a dict is used to compress data or not.
    int nthreads;
    //!< Determines the maximum number of threads that can be used.
    uint8_t filters[BLOSC2_MAX_FILTERS];
    //!< Defines the filters used in compression.
    uint8_t filtersmeta[BLOSC2_MAX_FILTERS];
    //!< Indicates the meta filters used in Blosc.
    blosc2_prefilter_fn prefilter;
    //!< Defines the function that is applied to the data before compressing it.
    blosc2_prefilter_params *pparams;
    //!< Indicates the parameters of the prefilter function.
};


/**
 * @brief Type definition for the #caterva_config_s structure.
 */
typedef struct caterva_config_s caterva_config_t;


/**
 * @brief The default configuration parameters used in Caterva.
 */
static const caterva_config_t CATERVA_CONFIG_DEFAULTS = {
    .alloc = malloc,
    .free = free,
    .compcodec = BLOSC_ZSTD,
    .complevel = 5,
    .usedict = 0,
    .nthreads = 1,
    .filters = {0, 0, 0, 0, 0, BLOSC_SHUFFLE},
    .filtersmeta = {0, 0, 0, 0, 0, 0},
    .prefilter = NULL,
    .pparams = NULL
};


/**
 * @brief Context for Caterva containers that specifies the functions used to manage memory and
 * the compression/decompression parameters used in Blosc.
 */
typedef struct caterva_context_s {
    caterva_config_t *cfg;
    //!< The configuration paramters.
} caterva_context_t;


/**
 * @brief The backends available to store the #caterva_array_t data.
 */
typedef enum caterva_storage_backend_e {
    CATERVA_STORAGE_BLOSC,
    //!< Indicates that the data is stored using a Blosc superchunk.
    CATERVA_STORAGE_PLAINBUFFER,
    //!< Indicates that the data is stored using a plain buffer.
} caterva_storage_backend_t;


/**
 * @brief The storage properties that have a #caterva_array_t backed by a Blosc superchunk.
 */
typedef struct caterva_storage_properties_blosc_s {
    int32_t chunkshape[CATERVA_MAXDIM];
    //!< The shape of each Blosc chunk;
    bool enforceframe;
    //!< Flag to indicate if the superchunk is stored as a frame.
    char* filename;
    //!< The superchunk/frame name. If @p filename is not @p NULL, the superchunk will be stored on disk.
} caterva_storage_properties_blosc_t;


/**
 * @brief The storage properties that have a #caterva_array_t backed by a plain buffer.
 */
typedef struct caterva_storage_properties_plainbuffer_s {
} caterva_storage_properties_plainbuffer_t;


/**
 * @brief The storage properties for a specific #caterva_array_t.
 */
typedef union caterva_storage_properties_u {
    caterva_storage_properties_blosc_t blosc;
    //!< The storage properties when the container is backed by a Blosc superchunk.
    caterva_storage_properties_plainbuffer_t plainbuffer;
    //!< The storage properties when the container is backed by a plain buffer.
} caterva_storage_properties_t;


/**
 * @brief Storage parameters needed for the creation of a #caterva_array_t container.
 */
struct caterva_storage_s {
    caterva_storage_backend_t backend;
    //!< The backend storage.
    caterva_storage_properties_t properties;
    //!< The specific properties for the selected @p backend.
};


/**
 * @brief Type definition for the #caterva_storage_s structure.
 */
typedef struct caterva_storage_s caterva_storage_t;


/**
 * @brief General parameters needed for the creation of a #caterva_array_t container.
 */
struct caterva_params_s {
    int64_t shape[CATERVA_MAXDIM];
    //!< The container shape.
    uint8_t ndim;
    //!< The container dimensions.
    uint8_t itemsize;
    //!< The size of each item of the container.
};


/**
 * @brief Type definition for the #caterva_params_s structure.
 */
typedef struct caterva_params_s caterva_params_t;


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
typedef struct caterva_array_s {
    caterva_storage_backend_t storage;
    //!< Storage type
    blosc2_schunk *sc;
    //!< Pointer to a Blosc superchunk
    //!< Only is used if \p storage equals to \p #CATERVA_STORAGE_BLOSC
    uint8_t *buf;
    //!< Pointer to a plain buffer where data is stored.
    //!< Only is used if \p storage equals to \p #CATERVA_STORAGE_PLAINBUFFER.
    int64_t shape[CATERVA_MAXDIM];
    //!< Shape of original data.
    int32_t chunkshape[CATERVA_MAXDIM];
    //!< Shape of each chunk. If @p storage equals to \p #CATERVA_STORAGE_PLAINBUFFER, it is equal to @shape.
    int64_t extendedshape[CATERVA_MAXDIM];
    //!< Shape of padded data.
    int64_t size;
    //!< Size of original data.
    int32_t chunksize;
    //!< Size of each chunk.
    int64_t extendedesize;
    //!< Size of padded data.
    int8_t ndim;
    //!< Data dimensions.
    int8_t itemsize;
    //!< Size of each item.
    bool empty;
    //!< Indicate if an array is empty or is filled with data.
    bool filled;
    //!< Indicate if an array is completely filled or not.
    int64_t nparts;
    //!< Number of partitions in the array.
    struct part_cache_s part_cache;
    //!< A partition cache.
} caterva_array_t;


/**
 * @brief Create a context for Caterva.
 *
 * @param cfg The configuration parameters needed for the context creation.
 * @param ctx The memory pointer where the context will be created.
 *
 * @return An error code.
 */
int caterva_context_new(caterva_config_t *cfg, caterva_context_t **ctx);


/**
 * @brief Free a Caterva context.
 *
 * @param ctx Pointer to the context to be freed.
 *
 * @return An error code.
 */
int caterva_context_free(caterva_context_t **ctx);


/**
 * @brief Create a caterva empty container
 *
 * When a container is created, only the partition shape, the storage type and the context are defined.
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
 * @param pshape The shape of each partition
 *
 * @return A pointer to the empty caterva container created
 */
int caterva_array_empty(caterva_context_t *ctx, caterva_params_t *params, caterva_storage_t *storage,
                        caterva_array_t **array);


/**
 * @brief Free a caterva container
 *
 * @param carr Pointer to the container to be freed
 *
 * @return An error code
 */
int caterva_array_free(caterva_context_t *ctx, caterva_array_t **array);


/**
 * Append a partition to a caterva container (until it is completely filled)
 *
 * @param array Pointer to the container where data will be appended
 * @param part A pointer to the buffer where data is stored
 * @param partsize Size (in bytes) of the buffer
 *
 * @return An error code
 */
int caterva_array_append(caterva_context_t *ctx, caterva_array_t *array, void *chunk, int64_t chunksize);


/**
 * @brief Create a caterva container from a frame
 *
 * @param ctx Pointer to the caterva context to be used.
 *   The context should be the same as the one used to create the array.
 * @param frame The frame for the caterva container.
 * @param copy If true, a new, sparse in-memory super-chunk is created.
 *   Else, a frame-backed one is created (i.e. no copies are made).
 *
 * @return A pointer to the new caterva container
 */
int caterva_array_from_frame(caterva_context_t *ctx, blosc2_frame *frame, bool copy, caterva_array_t **array);


/**
 * @brief Create a caterva container from a serialized frame
 *
 * @param ctx Pointer to the caterva context to be used.
 *   The context should be the same as the one used to create the array.
 * @param sframe The serialized frame for the caterva container.
 * @param len The length (in bytes) of tge serialized frame.
 * @param copy If true, a new, sparse in-memory super-chunk is created.
 *   Else, a frame-backed one is created (i.e. no copies are made).
 *
 * @return A pointer to the new caterva container
 */
int caterva_array_from_sframe(caterva_context_t *ctx, uint8_t *sframe, int64_t len, bool copy, caterva_array_t **array);


/**
 * @brief Read a caterva container from disk
 *
 * @param ctx Pointer to the caterva context to be used.
 *   The context should be the same as the one used to create the array.
 * @param filename The filename of the caterva container on disk.
 * @param copy If true, a new, sparse in-memory super-chunk is created.
 *   Else, a frame-backed one is created (i.e. no copies are made).
 *
 * @return A pointer to the new caterva container
 */
int caterva_array_from_file(caterva_context_t *ctx, const char *filename, bool copy, caterva_array_t **array);


/**
 * @brief Create a caterva container from the data obtained in a C buffer
 *
 * @param dest Pointer to the container that will be created with the buffer data
 * @param shape The shape of the buffer data
 * @param src A pointer to the C buffer where data is stored
 *
 * @return An error code
 */
int caterva_array_from_buffer(caterva_context_t *ctx, caterva_params_t *params, caterva_storage_t *storage,
                              void *buffer, int64_t buffersize, caterva_array_t **array);


/**
 * @brief Extract the data into a C buffer from a caterva container
 *
 * @param src Pointer to the container from which the data will be obtained
 * @param dest Pointer to the buffer where data will be stored
 *
 * @return An error code
 */
int caterva_array_to_buffer(caterva_context_t *ctx, caterva_array_t *array, void *buffer, int64_t buffersize);


/**
 * @brief Get a slice into an empty caterva container from another caterva container
 *
 * @param dest Pointer to the empty container where the obtained slice will be stored
 * @param src Pointer to the container from which the slice will be obtained
 * @param start The coordinates where the slice will begin
 * @param stop The coordinates where the slice will end
 *
 * @return An error code
 */
int caterva_array_get_slice(caterva_context_t *ctx, caterva_storage_t *storage, caterva_array_t *src,
    int64_t *start, int64_t *stop, caterva_array_t **array);


/**
 * @brief Squeeze a caterva container
 *
 * This function remove single-dimensional entries from the shape of a caterva container.
 *
 * @param src Pointer to the container to be squeezed
 *
 * @return An error code
 */
int caterva_array_squeeze(caterva_context_t *ctx, caterva_array_t *array);


/**
 * @brief Get a slice into a C buffer from a caterva container
 *
 * @param dest Pointer to the buffer where data will be stored
 * @param src Pointer to the container from which the slice will be extracted
 * @param start The coordinates where the slice will begin
 * @param stop The coordinates where the slice will end
 * @param d_pshape The partition shape of the buffer
 * @return An error code
 */
int caterva_array_get_slice_buffer(caterva_context_t *ctx, caterva_array_t *array, int64_t *start, int64_t *stop,
                                   int64_t *shape, void *buffer, int64_t buffersize);


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
int caterva_array_set_slice_buffer(caterva_context_t *ctx, void *buffer, int64_t buffersize, int64_t *start,
                                   int64_t *stop, caterva_array_t *array);


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
 * @param d_pshape The partition shape of the buffer
 *
 * @return An error code
 */
int caterva_array_get_slice_buffer_no_copy(caterva_context_t *ctx, caterva_array_t *src, int64_t *start,
                                           int64_t *stop, void **dest);



/**
 * @brief Make a copy of the container data.
 *
 * The copy is done into \p dest container
 *
 * @param dest Pointer to the container where data is copied
 * @param src Pointer to the container from which data is copied
 *
 * @return An error code
 */
int caterva_array_copy(caterva_context_t *ctx, caterva_storage_t *storage, caterva_array_t *src, caterva_array_t **dest);


#endif
