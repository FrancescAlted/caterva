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
#define CATERVA_VERSION_MINOR    3    /* for minor interface/format changes  */
#define CATERVA_VERSION_RELEASE  3    /* for tweaks, bug-fixes, or development */

#define CATERVA_VERSION_STRING   "0.3.3"  /* string version.  Sync with above! */
#define CATERVA_VERSION_DATE     "2020-04-27"    /* date version */


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
#ifdef __GNUC__
#define CATERVA_ATTRIBUTE_UNUSED  __attribute__((unused))
#else
#define CATERVA_ATTRIBUTE_UNUSED
#endif

static char *print_error(int rc) CATERVA_ATTRIBUTE_UNUSED;
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


/* The maximum number of dimensions for caterva arrays */
#define CATERVA_MAX_DIM 8

/* The maximum number of metalayers for caterva arrays */
#define CATERVA_MAX_METALAYERS BLOSC2_MAX_METALAYERS - 1


/**
 * @brief Configuration parameters used to create a caterva context.
 */
typedef struct {
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
} caterva_config_t;


/**
 * @brief The default configuration parameters used in caterva.
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
 * @brief Context for caterva arrays that specifies the functions used to manage memory and
 * the compression/decompression parameters used in Blosc.
 */
typedef struct {
    caterva_config_t *cfg;
    //!< The configuration paramters.
} caterva_context_t;


/**
 * @brief The backends available to store the data of the caterva array.
 */
typedef enum {
    CATERVA_STORAGE_BLOSC,
    //!< Indicates that the data is stored using a Blosc superchunk.
    CATERVA_STORAGE_PLAINBUFFER,
    //!< Indicates that the data is stored using a plain buffer.
} caterva_storage_backend_t;

/**
 * @brief The metalayer data needed to store it on an array
 */
typedef struct {
    char *name;
    //!< The name of the metalater
    uint8_t *sdata;
    //!< The serialized data to store
    int32_t size;
    //!< The size of the serialized data
}caterva_metalayer_t;

/**
 * @brief The storage properties for an array backed by a Blosc superchunk.
 */
typedef struct {
    int32_t chunkshape[CATERVA_MAX_DIM];
    //!< The shape of each chunk of Blosc.
    int32_t blockshape[CATERVA_MAX_DIM];
    //!< The shape of each block of Blosc.
    bool enforceframe;
    //!< Flag to indicate if the superchunk is stored as a frame.
    char* filename;
    //!< The superchunk/frame name. If @p filename is not @p NULL, the superchunk will be stored on disk.
    caterva_metalayer_t metalayers[CATERVA_MAX_METALAYERS];
    //!< List with the metalayers desired.
    int32_t nmetalayers;
    //!< The number of metalayers.
} caterva_storage_properties_blosc_t;


/**
 * @brief The storage properties that have a caterva array backed by a plain buffer.
 */
typedef struct {
    char* filename;
    //!< The plain buffer name. If @p filename is not @p NULL, the plain buffer will be stored on disk. (Not implemented yet).
} caterva_storage_properties_plainbuffer_t;


/**
 * @brief The storage properties for an array.
 */
typedef union {
    caterva_storage_properties_blosc_t blosc;
    //!< The storage properties when the array is backed by a Blosc superchunk.
    caterva_storage_properties_plainbuffer_t plainbuffer;
    //!< The storage properties when the array is backed by a plain buffer.
} caterva_storage_properties_t;


/**
 * @brief Storage parameters needed for the creation of a caterva array.
 */
typedef struct {
    caterva_storage_backend_t backend;
    //!< The backend storage.
    caterva_storage_properties_t properties;
    //!< The specific properties for the selected @p backend.
} caterva_storage_t;


/**
 * @brief General parameters needed for the creation of a caterva array.
 */
typedef struct {
    uint8_t itemsize;
    //!< The size of each item of the array.
    int64_t shape[CATERVA_MAX_DIM];
    //!< The array shape.
    uint8_t ndim;
    //!< The array dimensions.
} caterva_params_t;


/**
 * @brief An *optional* cache for a single block.
 *
 * When a chunk is needed, it is copied into this cache. In this way, if the same chunk is needed
 * again afterwards, it is not necessary to recover it because it is already in the cache.
 */
struct part_cache_s {
    uint8_t *data;
    //!< Pointer to the chunk data.
    int32_t nchunk;
    //!< The chunk number in cache. If @p nchunk equals to -1, it means that the cache is empty.
};


/**
 * @brief A multidimensional array of data that can be compressed data.
 */
typedef struct {
    caterva_storage_backend_t storage;
    //!< Storage type.
    blosc2_schunk *sc;
    //!< Pointer to a Blosc superchunk
    //!< Only is used if \p storage equals to @p CATERVA_STORAGE_BLOSC.
    uint8_t *buf;
    //!< Pointer to a plain buffer where data is stored.
    //!< Only is used if \p storage equals to @p CATERVA_STORAGE_PLAINBUFFER.
    int64_t shape[CATERVA_MAX_DIM];
    //!< Shape of original data.
    int32_t chunkshape[CATERVA_MAX_DIM];
    //!< Shape of each chunk. If @p storage equals to @p CATERVA_STORAGE_PLAINBUFFER, it is equal to @p shape.
    int64_t extendedshape[CATERVA_MAX_DIM];
    //!< Shape of padded data.
    int32_t blockshape[CATERVA_MAX_DIM];
    //!< Shape of each subpartition.
    int64_t extendedchunkshape[CATERVA_MAX_DIM];
    //!< Shape of padded partition.
    int32_t next_chunkshape[CATERVA_MAX_DIM];
    //!< Shape of next partition to be appended.
    int64_t size;
    //!< Size of original data.
    int32_t chunksize;
    //!< Size of each chunk.
    int64_t extendedsize;
    //!< Size of padded data.
    int32_t blocksize;
    //!< Size of each subpartition.
    int64_t extendedchunksize;
    //!< Size of padded partition.
    int64_t next_chunksize;
    //!< Size of next partiton to be appended.
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
 * @brief Create a context for caterva.
 *
 * @param cfg The configuration parameters needed for the context creation.
 * @param ctx Pointer to the memory pointer where the context will be created.
 *
 * @return An error code.
 */
int caterva_context_new(caterva_config_t *cfg, caterva_context_t **ctx);


/**
 * @brief Free a context.
 *
 * @param ctx Pointer to the pointer to the context to be freed.
 *
 * @return An error code.
 */
int caterva_context_free(caterva_context_t **ctx);


/**
 * @brief Create an empty array.
 *
 * @param ctx Pointer to the caterva context to be used.
 * @param params Pointer to the general params of the array desired.
 * @param storage Pointer to the storage params of the array desired.
 * @param array Pointer to the memory pointer where the array will be created.
 *
 * @return An error code.
 */
int caterva_array_empty(caterva_context_t *ctx, caterva_params_t *params, caterva_storage_t *storage,
                        caterva_array_t **array);


/**
 * @brief Free an array.
 *
 * @param ctx Pointer to the caterva context to be used.
 * @param array Pointer to the memory pointer where the array is placed.
 *
 * @return An error code.
 */
int caterva_array_free(caterva_context_t *ctx, caterva_array_t **array);


/**
 * Append a chunk to a caterva array (until it is completely filled).
 *
 * @param ctx Pointer to the caterva context to be used.
 * @param array Pointer to the caterva array.
 * @param chunk Pointer to the buffer where the chunk data is stored.
 * @param chunksize Size (in bytes) of the buffer.
 *
 * @return An error code.
 */
int caterva_array_append(caterva_context_t *ctx, caterva_array_t *array, void *chunk, int64_t chunksize);


/**
 * @brief Create a caterva array from a frame. It can only be used if the array
 * is backed by a blosc super-chunk.
 *
 * @param ctx Pointer to the caterva context to be used.
 * @param frame The blosc frame where the caterva array is stored.
 * @param copy If true, a new, sparse in-memory super-chunk is created. Else, a frame-backed one is
 * created (i.e. no copies are made).
 * @param array Pointer to the memory pointer where the array will be created.
 *
 * @return An error code.
 */
int caterva_array_from_frame(caterva_context_t *ctx, blosc2_frame *frame, bool copy, caterva_array_t **array);


/**
 * @brief Create a caterva array from a serialized frame. It can only be used if the array
 * is backed by a blosc super-chunk.
 *
 * @param ctx Pointer to the caterva context to be used.
 * @param sframe The serialized frame where the caterva array is stored.
 * @param len The size (in bytes) of the serialized frame.
 * @param copy If true, a new, sparse in-memory super-chunk is created. Else, a frame-backed one is
 * created (i.e. no copies are made).
 * @param array Pointer to the memory pointer where the array will be created.
 *
 * @return An error code.
 */
int caterva_array_from_sframe(caterva_context_t *ctx, uint8_t *sframe, int64_t len, bool copy, caterva_array_t **array);


/**
 * @brief Read a caterva array from disk.
 *
 * @param ctx Pointer to the caterva context to be used.
 * @param filename The filename of the caterva array on disk.
 * @param copy If true, a new, sparse in-memory super-chunk is created. Else, a frame-backed one is
 * created (i.e. no copies are made).
 * @param array Pointer to the memory pointer where the array will be created.
 *
 * @return An error code.
 */
int caterva_array_from_file(caterva_context_t *ctx, const char *filename, bool copy, caterva_array_t **array);


/**
 * @brief Create a caterva array from the data stored in a buffer.
 *
 * @param ctx Pointer to the caterva context to be used.
 * @param buffer Pointer to the buffer where source data is stored.
 * @param buffersize The size (in bytes) of the serialized frame.
 * @param params Pointer to the general params of the array desired.
 * @param storage Pointer to the storage params of the array desired.
 * @param array Pointer to the memory pointer where the array will be created.
 *
 * @return An error code.
 */
int caterva_array_from_buffer(caterva_context_t *ctx, void *buffer, int64_t buffersize, caterva_params_t *params,
                              caterva_storage_t *storage, caterva_array_t **array);


/**
 * @brief Extract the data into a C buffer from a caterva array.
 *
 * @param ctx Pointer to the caterva context to be used.
 * @param array Pointer to the caterva array.
 * @param buffer Pointer to the buffer where the data will be stored.
 * @param buffersize Size (in bytes) of the buffer.
 *
 * @return An error code.
 */
int caterva_array_to_buffer(caterva_context_t *ctx, caterva_array_t *array, void *buffer, int64_t buffersize);


/**
 * @brief Get a slice from an array and store it into a new array.
 *
 * @param ctx Pointer to the caterva context to be used.
 * @param src Pointer to the array from which the slice will be extracted
 * @param start The coordinates where the slice will begin.
 * @param stop The coordinates where the slice will end.
 * @param storage Pointer to the storage params of the array desired.
 * @param array Pointer to the memory pointer where the array will be created.
 *
 * @return An error code.
 */
int caterva_array_get_slice(caterva_context_t *ctx, caterva_array_t *src, int64_t *start, int64_t *stop,
                            caterva_storage_t *storage, caterva_array_t **array);


/**
 * @brief Squeeze a caterva array
 *
 * This function remove single-dimensional entries from the shape of a caterva array.
 *
 * @param ctx Pointer to the caterva context to be used.
 * @param array Pointer to the caterva array.
 *
 * @return An error code
 */
int caterva_array_squeeze(caterva_context_t *ctx, caterva_array_t *array);


/**
 * @brief Get a slice from an array and store it into a C buffer.
 *
 * @param ctx Pointer to the caterva context to be used.
 * @param src Pointer to the array from which the slice will be extracted.
 * @param start The coordinates where the slice will begin.
 * @param stop The coordinates where the slice will end.
 * @param shape The shape of the buffer.
 * @param buffer Pointer to the buffer where the data will be stored.
 * @param buffersize The size (in bytes) of the buffer.
 *
 * @return An error code.
 */
int caterva_array_get_slice_buffer(caterva_context_t *ctx, caterva_array_t *src, int64_t *start, int64_t *stop,
                                   int64_t *shape, void *buffer, int64_t buffersize);


/**
 * @brief Set a slice into a caterva array from a C buffer. It can only be used if the array
 * is backed by a plainbuffer.
 *
 * @param ctx Pointer to the caterva context to be used.
 * @param buffer Pointer to the buffer where the slice data is.
 * @param buffersize The size (in bytes) of the buffer.
 * @param start The coordinates where the slice will begin.
 * @param stop The coordinates where the slice will end.
 * @param array Pointer to the caterva array where the slice will be set
 *
 * @return An error code.
 */
int caterva_array_set_slice_buffer(caterva_context_t *ctx, void *buffer, int64_t buffersize, int64_t *start,
                                   int64_t *stop, caterva_array_t *array);


/**
 * @brief Make a copy of the array data. The copy is done into a new caterva array.
 *
 * @param ctx Pointer to the caterva context to be used.
 * @param src Pointer to the array from which data is copied.
 * @param storage Pointer to the storage params of the array desired.
 * @param array Pointer to the memory pointer where the array will be created.
 *
 * @return An error code
 */
int caterva_array_copy(caterva_context_t *ctx, caterva_array_t *src, caterva_storage_t *storage, caterva_array_t **array);


#endif
