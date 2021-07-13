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

#ifndef CATERVA_CATERVA_H_
#define CATERVA_CATERVA_H_

#include <blosc2.h>
#include <stdio.h>
#include <stdlib.h>

/* Version numbers */
#define CATERVA_VERSION_MAJOR 0         /* for major interface/format changes  */
#define CATERVA_VERSION_MINOR 5         /* for minor interface/format changes  */
#define CATERVA_VERSION_RELEASE 1       /* for tweaks, bug-fixes, or development */

#define CATERVA_VERSION_STRING "0.5.1.dev0" /* string version. Sync with above! */
#define CATERVA_VERSION_DATE "2021-07-13"  /* date version */

/* Error handling */
#define CATERVA_SUCCEED 0
#define CATERVA_ERR_INVALID_ARGUMENT 1
#define CATERVA_ERR_BLOSC_FAILED 2
#define CATERVA_ERR_CONTAINER_FILLED 3
#define CATERVA_ERR_INVALID_STORAGE 4
#define CATERVA_ERR_NULL_POINTER 5
#define CATERVA_ERR_INVALID_INDEX  5

#ifdef NDEBUG
#define DEBUG_PRINT(...) \
    do {                 \
    } while (0)
#else
#define DEBUG_PRINT(...)                                                         \
    do {                                                                         \
        fprintf(stderr, "ERROR: %s (%s:%d)\n", __VA_ARGS__, __FILE__, __LINE__); \
    } while (0)
#endif

#define CATERVA_ERROR(rc)                 \
    do {                                  \
        int rc_ = rc;\
        if (rc_ != CATERVA_SUCCEED) {      \
            DEBUG_PRINT(print_error(rc_)); \
            return rc_;                    \
        }                                 \
    } while (0)
#define CATERVA_ERROR_NULL(pointer)                             \
    do {                                                        \
        if (pointer == NULL) {                                  \
            DEBUG_PRINT(print_error(CATERVA_ERR_NULL_POINTER)); \
            return CATERVA_ERR_NULL_POINTER;                    \
        }                                                       \
    } while (0)

#define CATERVA_UNUSED_PARAM(x) ((void) (x))
#ifdef __GNUC__
#define CATERVA_ATTRIBUTE_UNUSED __attribute__((unused))
#else
#define CATERVA_ATTRIBUTE_UNUSED
#endif

static char *print_error(int rc) CATERVA_ATTRIBUTE_UNUSED;
static char *print_error(int rc) {
    switch (rc) {
        case CATERVA_ERR_INVALID_STORAGE:
            return "Invalid storage";
        case CATERVA_ERR_NULL_POINTER:
            return "Pointer is null";
        case CATERVA_ERR_BLOSC_FAILED:
            return "Blosc failed";
        default:
            return "Unknown error";
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
    uint8_t compcodec;
    //!< Defines the codec used in compression.
    uint8_t compmeta;
    //!< The metadata for the compressor codec.
    uint8_t complevel;
    //!< Determines the compression level used in Blosc.
    int32_t splitmode;
    //!< Whether the blocks should be split or not.
    int usedict;
    //!< Indicates whether a dictionary is used to compress data or not.
    int16_t nthreads;
    //!< Determines the maximum number of threads that can be used.
    uint8_t filters[BLOSC2_MAX_FILTERS];
    //!< Defines the filters used in compression.
    uint8_t filtersmeta[BLOSC2_MAX_FILTERS];
    //!< Indicates the meta filters used in Blosc.
    blosc2_prefilter_fn prefilter;
    //!< Defines the function that is applied to the data before compressing it.
    blosc2_prefilter_params *pparams;
    //!< Indicates the parameters of the prefilter function.
    blosc2_btune *udbtune;
    //!< Indicates user-defined parameters for btune.
} caterva_config_t;

/**
 * @brief The default configuration parameters used in caterva.
 */
static const caterva_config_t CATERVA_CONFIG_DEFAULTS = {.alloc = malloc,
                                                         .free = free,
                                                         .compcodec = BLOSC_ZSTD,
                                                         .compmeta = 0,
                                                         .complevel = 5,
                                                         .splitmode = BLOSC_AUTO_SPLIT,
                                                         .usedict = 0,
                                                         .nthreads = 1,
                                                         .filters = {0, 0, 0, 0, 0, BLOSC_SHUFFLE},
                                                         .filtersmeta = {0, 0, 0, 0, 0, 0},
                                                         .prefilter = NULL,
                                                         .pparams = NULL,
                                                         .udbtune = NULL,
                                                         };

/**
 * @brief Context for caterva arrays that specifies the functions used to manage memory and
 * the compression/decompression parameters used in Blosc.
 */
typedef struct {
    caterva_config_t *cfg;
    //!< The configuration paramters.
} caterva_ctx_t;

/**
 * @brief The backends available to store the data of the caterva array.
 */
typedef enum {
    CATERVA_STORAGE_BLOSC,
    //!< Indicates that the data is stored using a Blosc super-chunk.
    CATERVA_STORAGE_PLAINBUFFER,
    //!< Indicates that the data is stored using a plain buffer.
} caterva_storage_backend_t;

/**
 * @brief The metalayer data needed to store it on an array
 */
typedef struct {
    char *name;
    //!< The name of the metalayer
    uint8_t *sdata;
    //!< The serialized data to store
    int32_t size;
    //!< The size of the serialized data
} caterva_metalayer_t;

/**
 * @brief The storage properties for an array backed by a Blosc super-chunk.
 */
typedef struct {
    int32_t chunkshape[CATERVA_MAX_DIM];
    //!< The shape of each chunk of Blosc.
    int32_t blockshape[CATERVA_MAX_DIM];
    //!< The shape of each block of Blosc.
    bool sequencial;
    //!< Flag to indicate if the super-chunk is stored sequentially or sparsely.
    char *urlpath;
    //!< The super-chunk name. If @p urlpath is not @p NULL, the super-chunk will be stored on
    //!< disk.
    caterva_metalayer_t metalayers[CATERVA_MAX_METALAYERS];
    //!< List with the metalayers desired.
    int32_t nmetalayers;
    //!< The number of metalayers.
} caterva_storage_properties_blosc_t;

/**
 * @brief The storage properties that have a caterva array backed by a plain buffer.
 */
typedef struct {
    char *urlpath;
    //!< The plain buffer name. If @p urlpath is not @p NULL, the plain buffer will be stored on
    //!< disk. (Not implemented yet).
} caterva_storage_properties_plainbuffer_t;

/**
 * @brief The storage properties for an array.
 */
typedef union {
    caterva_storage_properties_blosc_t blosc;
    //!< The storage properties when the array is backed by a Blosc super-chunk.
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
struct chunk_cache_s {
    uint8_t *data;
    //!< The chunk data.
    int32_t nchunk;
    //!< The chunk number in cache. If @p nchunk equals to -1, it means that the cache is empty.
};

/**
 * @brief A multidimensional array of data that can be compressed data.
 */
typedef struct {
    caterva_storage_backend_t storage;
    //!< Storage type.
    caterva_config_t *cfg;
    //!< Array configuration.
    blosc2_schunk *sc;
    //!< Pointer to a Blosc super-chunk
    //!< Only is used if \p storage equals to @p CATERVA_STORAGE_BLOSC.
    uint8_t *buf;
    //!< Pointer to a plain buffer where data is stored.
    //!< Only is used if \p storage equals to @p CATERVA_STORAGE_PLAINBUFFER.
    int64_t shape[CATERVA_MAX_DIM];
    //!< Shape of original data.
    int32_t chunkshape[CATERVA_MAX_DIM];
    //!< Shape of each chunk. If @p storage equals to @p CATERVA_STORAGE_PLAINBUFFER, it is equal to
    //!< @p shape.
    int64_t extshape[CATERVA_MAX_DIM];
    //!< Shape of padded data.
    int32_t blockshape[CATERVA_MAX_DIM];
    //!< Shape of each block.
    int64_t extchunkshape[CATERVA_MAX_DIM];
    //!< Shape of padded chunk.
    int64_t nitems;
    //!< Number of items in original data.
    int32_t chunknitems;
    //!< Number of items in each chunk.
    int64_t extnitems;
    //!< Number of items in padded data.
    int32_t blocknitems;
    //!< Number of items in each block.
    int64_t extchunknitems;
    //!< Number of items in a padded chunk.
    uint8_t ndim;
    //!< Data dimensions.
    uint8_t itemsize;
    //!< Size of each item.
    int64_t nchunks;
    //!< Number of chunks in the array.
    struct chunk_cache_s chunk_cache;
    //!< A partition cache.
} caterva_array_t;

/**
 * @brief Create a context for caterva.
 *
 * @param cfg The configuration parameters needed for the context creation.
 * @param ctx The memory pointer where the context will be created.
 *
 * @return An error code.
 */
int caterva_ctx_new(caterva_config_t *cfg, caterva_ctx_t **ctx);

/**
 * @brief Free a context.
 *
 * @param ctx The The context to be freed.
 *
 * @return An error code.
 */
int caterva_ctx_free(caterva_ctx_t **ctx);

/**
 * @brief Create an empty array.
 *
 * @param ctx The caterva context to be used.
 * @param params The general params of the array desired.
 * @param storage The storage params of the array desired.
 * @param array The memory pointer where the array will be created.
 *
 * @return An error code.
 */
int caterva_empty(caterva_ctx_t *ctx, caterva_params_t *params,
                  caterva_storage_t *storage, caterva_array_t **array);


/**
 * Create an array, with zero being used as the default value for
 * uninitialized portions of the array.
 *
 * @param ctx The caterva context to be used.
 * @param params The general params of the array.
 * @param storage The storage params of the array.
 * @param array The memory pointer where the array will be created.
 *
 * @return An error code.
 */
int caterva_zeros(caterva_ctx_t *ctx, caterva_params_t *params,
                  caterva_storage_t *storage, caterva_array_t **array);


/**
 * Create an array, with @p fill_value being used as the default value for
 * uninitialized portions of the array.
 *
 * @param ctx The caterva context to be used.
 * @param params The general params of the array.
 * @param storage The storage params of the array.
 * @param fill_value Default value for uninitialized portions of the array.
 * @param array The memory pointer where the array will be created.
 *
 * @return An error code.
 */
int caterva_full(caterva_ctx_t *ctx, caterva_params_t *params,
                 caterva_storage_t *storage, void *fill_value, caterva_array_t **array);
/**
 * @brief Free an array.
 *
 * @param ctx The caterva context to be used.
 * @param array The memory pointer where the array is placed.
 *
 * @return An error code.
 */
int caterva_free(caterva_ctx_t *ctx, caterva_array_t **array);

/**
 * Append a chunk to a caterva array (until it is completely filled).
 *
 * @param ctx The caterva context to be used.
 * @param array The caterva array.
 * @param chunk The buffer where the chunk data is stored.
 * @param chunksize Size (in bytes) of the buffer.
 *
 * @return An error code.
 */
int caterva_append(caterva_ctx_t *ctx, caterva_array_t *array, void *chunk,
                   int64_t chunksize);

/**
 * @brief Create a caterva array from a super-chunk. It can only be used if the array
 * is backed by a blosc super-chunk.
 *
 * @param ctx The caterva context to be used.
 * @param schunk The blosc super-chunk where the caterva array is stored.
 * @param array The memory pointer where the array will be created.
 *
 * @return An error code.
 */
int
caterva_from_schunk(caterva_ctx_t *ctx, blosc2_schunk *schunk, caterva_array_t **array);

/**
 * @brief Create a caterva array from a serialized super-chunk. It can only be used if the array
 * is backed by a blosc super-chunk.
 *
 * @param ctx The caterva context to be used.
 * @param serial_schunk The serialized super-chunk where the caterva array is stored.
 * @param len The size (in bytes) of the serialized super-chunk.
 * @param array The memory pointer where the array will be created.
 *
 * @return An error code.
 */
int caterva_from_serial_schunk(caterva_ctx_t *ctx, uint8_t *serial_schunk, int64_t len,
                               caterva_array_t **array);

/**
 * @brief Read a caterva array from disk.
 *
 * @param ctx The caterva context to be used.
 * @param urlpath The urlpath of the caterva array on disk.
 * @param array The memory pointer where the array will be created.
 *
 * @return An error code.
 */
int caterva_open(caterva_ctx_t *ctx, const char *urlpath, caterva_array_t **array);

/**
 * @brief Save caterva array into a specific urlpath.
 *
 * @param ctx The context to be used.
 * @param array The array to be saved.
 * @param urlpath The urlpath where the array will be stored.
 *
 * @return An error code.
 */
int caterva_save(caterva_ctx_t *ctx, caterva_array_t *array, char *urlpath);

/**
 * @brief Create a caterva array from the data stored in a buffer.
 *
 * @param ctx The caterva context to be used.
 * @param buffer The buffer where source data is stored.
 * @param buffersize The size (in bytes) of the buffer.
 * @param params The general params of the array desired.
 * @param storage The storage params of the array desired.
 * @param array The memory pointer where the array will be created.
 *
 * @return An error code.
 */
int caterva_from_buffer(caterva_ctx_t *ctx, void *buffer, int64_t buffersize,
                        caterva_params_t *params, caterva_storage_t *storage,
                        caterva_array_t **array);

/**
 * @brief Extract the data into a C buffer from a caterva array.
 *
 * @param ctx The caterva context to be used.
 * @param array The caterva array.
 * @param buffer The buffer where the data will be stored.
 * @param buffersize Size (in bytes) of the buffer.
 *
 * @return An error code.
 */
int caterva_to_buffer(caterva_ctx_t *ctx, caterva_array_t *array, void *buffer,
                      int64_t buffersize);

/**
 * @brief Get a slice from an array and store it into a new array.
 *
 * @param ctx The caterva context to be used.
 * @param src The array from which the slice will be extracted
 * @param start The coordinates where the slice will begin.
 * @param stop The coordinates where the slice will end.
 * @param storage The storage params of the array desired.
 * @param array The memory pointer where the array will be created.
 *
 * @return An error code.
 */
int caterva_get_slice(caterva_ctx_t *ctx, caterva_array_t *src, int64_t *start,
                      int64_t *stop, caterva_storage_t *storage, caterva_array_t **array);

/**
 * @brief Squeeze a caterva array
 *
 * This function remove selected single-dimensional entries from the shape of a
 caterva array.
 *
 * @param ctx The caterva context to be used.
 * @param array The caterva array.
 * @param index Indexes of the single-dimensional entries to remove.
 *
 * @return An error code
 */
int caterva_squeeze_index(caterva_ctx_t *ctx, caterva_array_t *array,
                          bool *index);

/**
 * @brief Squeeze a caterva array
 *
 * This function remove single-dimensional entries from the shape of a caterva array.
 *
 * @param ctx The caterva context to be used.
 * @param array The caterva array.
 *
 * @return An error code
 */
int caterva_squeeze(caterva_ctx_t *ctx, caterva_array_t *array);

/**
 * @brief Get a slice from an array and store it into a C buffer.
 *
 * @param ctx The caterva context to be used.
 * @param array The array from which the slice will be extracted.
 * @param start The coordinates where the slice will begin.
 * @param stop The coordinates where the slice will end.
 * @param buffershape The shape of the buffer.
 * @param buffer The buffer where the data will be stored.
 * @param buffersize The size (in bytes) of the buffer.
 *
 * @return An error code.
 */
int caterva_get_slice_buffer(caterva_ctx_t *ctx, caterva_array_t *array,
                             int64_t *start, int64_t *stop,
                             void *buffer, int64_t *buffershape, int64_t buffersize);

/**
 * @brief Set a slice into a caterva array from a C buffer.
 *
 * @param ctx The caterva context to be used.
 * @param buffer The buffer where the slice data is.
 * @param buffersize The size (in bytes) of the buffer.
 * @param start The coordinates where the slice will begin.
 * @param stop The coordinates where the slice will end.
 * @param buffershape The shape of the buffer.
 * @param array The caterva array where the slice will be set
 *
 * @return An error code.
 */
int caterva_set_slice_buffer(caterva_ctx_t *ctx,
                             void *buffer, int64_t *buffershape, int64_t buffersize,
                             int64_t *start, int64_t *stop, caterva_array_t *array);

/**
 * @brief Make a copy of the array data. The copy is done into a new caterva array.
 *
 * @param ctx The caterva context to be used.
 * @param src The array from which data is copied.
 * @param storage The storage params of the array desired.
 * @param array The memory pointer where the array will be created.
 *
 * @return An error code
 */
int caterva_copy(caterva_ctx_t *ctx, caterva_array_t *src, caterva_storage_t *storage,
                 caterva_array_t **array);


/**
 * @brief Remove a Caterva file from the file system. Both backends are supported.
 *
 * @param ctx The caterva context to be used.
 * @param urlpath The urlpath of the array to be removed.
 *
 * @return An error code
 */
int caterva_remove(caterva_ctx_t *ctx, char *urlpath);


/**
 * @brief Add a vl-metalayer to the Caterva array.
 *
 * @param ctx The context to be used.
 * @param array The array where the metalayer will be added.
 * @param name The vl-metalayer to add.
 *
 * @return An error code
 */
int caterva_vlmeta_add(caterva_ctx_t *ctx, caterva_array_t *array, caterva_metalayer_t *vlmeta);


/**
 *
 * @brief Get a vl-metalayer from a Caterva array.
 *
 * @param ctx The context to be used.
 * @param array The array where the vl-metalayer will be added.
 * @param name The vl-metalayer name.
 * @param vlmeta Pointer to the metalayer where the data will be stored.
 *
 * @warning The contents of `vlmeta` are allocated inside the function.
 * Therefore, they must be released with a `free`.
 *
 * @return An error code
 */
int caterva_vlmeta_get(caterva_ctx_t *ctx, caterva_array_t *array,
                       const char *name, caterva_metalayer_t *vlmeta);

/**
 * @brief Check if a vl-metalayer exists or not.
 *
 * @param ctx The context to be used.
 * @param array The array where the check will be done.
 * @param name The name of the vl-metalayer to check.
 * @param exists Pointer where the result will be stored.
 *
 * @return An error code
 */
int caterva_vlmeta_exists(caterva_ctx_t *ctx, caterva_array_t *array,
                          const char *name, bool *exists);

/**
 * @brief Update a vl-metalayer content in a Caterva array.
 *
 * @param ctx The context to be used.
 * @param array The array where the vl-metalayer will be updated.
 * @param vlmeta The vl-metalayer to update.
 *
 * @return An error code
 */
int caterva_vlmeta_update(caterva_ctx_t *ctx, caterva_array_t *array,
                          caterva_metalayer_t *vlmeta);

/**
 *
 * @brief Get a metalayer from a Caterva array.
 *
 * @param ctx The context to be used.
 * @param array The array where the metalayer will be added.
 * @param name The vl-metalayer name.
 * @param meta Pointer to the metalayer where the data will be stored.
 *
 * @warning The contents of `meta` are allocated inside the function.
 * Therefore, they must be released with a `free`.
 *
 * @return An error code
 */
int caterva_meta_get(caterva_ctx_t *ctx, caterva_array_t *array,
                       const char *name, caterva_metalayer_t *meta);

/**
 * @brief Check if a metalayer exists or not.
 *
 * @param ctx The context to be used.
 * @param array The array where the check will be done.
 * @param name The name of the metalayer to check.
 * @param exists Pointer where the result will be stored.
 *
 * @return An error code
 */
int caterva_meta_exists(caterva_ctx_t *ctx, caterva_array_t *array,
                          const char *name, bool *exists);

/**
 * @brief Update a metalayer content in a Caterva array.
 *
 * @param ctx The context to be used.
 * @param array The array where the metalayer will be updated.
 * @param meta The metalayer to update.
 *
 * @return An error code
 */
int caterva_meta_update(caterva_ctx_t *ctx, caterva_array_t *array,
                          caterva_metalayer_t *meta);

#endif  // CATERVA_CATERVA_H_
