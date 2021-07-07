/*
 * Copyright (C) 2018 Francesc Alted, Aleix Alcacer.
 * Copyright (C) 2019-present Blosc Development team <blosc@blosc.org>
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

#include <caterva.h>
#include <caterva_utils.h>

int caterva_blosc_update_shape(caterva_array_t *array, int8_t ndim, int64_t *shape,
                               int32_t *chunkshape, int32_t *blockshape) {
    array->ndim = ndim;
    array->nitems = 1;
    array->extnitems = 1;
    array->extchunknitems = 1;
    array->chunknitems = 1;
    array->blocknitems = 1;
    for (int i = 0; i < CATERVA_MAX_DIM; ++i) {
        if (i < ndim) {
            array->shape[i] = shape[i];
            array->chunkshape[i] = chunkshape[i];
            array->blockshape[i] = blockshape[i];
            if (shape[i] != 0) {
                if (shape[i] % array->chunkshape[i] == 0) {
                    array->extshape[i] = shape[i];
                } else {
                    array->extshape[i] = shape[i] + chunkshape[i] - shape[i] % chunkshape[i];
                }
                if (chunkshape[i] % blockshape[i] == 0) {
                    array->extchunkshape[i] = chunkshape[i];
                } else {
                    array->extchunkshape[i] =
                            chunkshape[i] + blockshape[i] - chunkshape[i] % blockshape[i];
                }
            } else {
                array->extchunkshape[i] = 0;
                array->extshape[i] = 0;
            }
        } else {
            array->blockshape[i] = 1;
            array->chunkshape[i] = 1;
            array->extshape[i] = 1;
            array->extchunkshape[i] = 1;
            array->shape[i] = 1;
        }
        array->nitems *= array->shape[i];
        array->extnitems *= array->extshape[i];
        array->extchunknitems *= array->extchunkshape[i];
        array->chunknitems *= array->chunkshape[i];
        array->blocknitems *= array->blockshape[i];
    }

    if (array->sc) {
        uint8_t *smeta = NULL;
        // Serialize the dimension info ...
        int32_t smeta_len =
                serialize_meta(array->ndim, array->shape, array->chunkshape, array->blockshape,
                               &smeta);
        if (smeta_len < 0) {
            fprintf(stderr, "error during serializing dims info for Caterva");
            return -1;
        }
        // ... and update it in its metalayer
        if (blosc2_meta_exists(array->sc, "caterva") < 0) {
            if (blosc2_meta_add(array->sc, "caterva", smeta, (uint32_t) smeta_len) < 0) {
                CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
            }
        } else {
            if (blosc2_meta_update(array->sc, "caterva", smeta, (uint32_t) smeta_len) < 0) {
                CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
            }
        }
        free(smeta);
    }

    return CATERVA_SUCCEED;
}


// Create an array without data
int caterva_blosc_array_without_schunk(caterva_ctx_t *ctx, caterva_params_t *params,
                                       caterva_storage_t *storage, caterva_array_t **array) {
    /* Create a caterva_array_t buffer */
    (*array) = (caterva_array_t *) ctx->cfg->alloc(sizeof(caterva_array_t));
    CATERVA_ERROR_NULL(*array);

    (*array)->cfg = (caterva_config_t *) ctx->cfg->alloc(sizeof(caterva_config_t));
    memcpy((*array)->cfg, ctx->cfg, sizeof(caterva_config_t));

    (*array)->buf = NULL;
    (*array)->sc = NULL;

    (*array)->storage = storage->backend;
    (*array)->ndim = params->ndim;
    (*array)->itemsize = params->itemsize;

    int64_t *shape = params->shape;
    int32_t *chunkshape = storage->properties.blosc.chunkshape;
    int32_t *blockshape = storage->properties.blosc.blockshape;

    caterva_blosc_update_shape(*array, params->ndim, shape, chunkshape, blockshape);

    // The partition cache (empty initially)
    (*array)->chunk_cache.data = NULL;
    (*array)->chunk_cache.nchunk = -1;  // means no valid cache yet

    if ((*array)->nitems != 0) {
        (*array)->nchunks = (*array)->extnitems / (*array)->chunknitems;
    } else {
        (*array)->nchunks = 0;
    }

    return CATERVA_SUCCEED;
}

int caterva_create_blosc_params(caterva_ctx_t *ctx,
                                caterva_params_t *params,
                                caterva_storage_t *storage,
                                blosc2_cparams *cparams,
                                blosc2_dparams *dparams,
                                blosc2_storage *b_storage) {
    int32_t blocknitems = 1;
    for (int i = 0; i < params->ndim; ++i) {
        blocknitems *= storage->properties.blosc.blockshape[i];
    }

    memcpy(cparams, &BLOSC2_CPARAMS_DEFAULTS, sizeof(blosc2_cparams));
    cparams->blocksize = blocknitems * params->itemsize;
    cparams->schunk = NULL;
    cparams->typesize = params->itemsize;
    cparams->prefilter = ctx->cfg->prefilter;
    cparams->preparams = ctx->cfg->pparams;
    cparams->use_dict = ctx->cfg->usedict;
    cparams->nthreads = (int16_t) ctx->cfg->nthreads;
    cparams->clevel = (uint8_t) ctx->cfg->complevel;
    cparams->compcode = (uint8_t) ctx->cfg->compcodec;
    cparams->compcode_meta = (uint8_t) ctx->cfg->compmeta;
    for (int i = 0; i < BLOSC2_MAX_FILTERS; ++i) {
        cparams->filters[i] = ctx->cfg->filters[i];
        cparams->filters_meta[i] = ctx->cfg->filtersmeta[i];
    }
    cparams->udbtune = ctx->cfg->udbtune;
    cparams->splitmode = ctx->cfg->splitmode;

    memcpy(dparams, &BLOSC2_DPARAMS_DEFAULTS, sizeof(blosc2_dparams));
    dparams->schunk = NULL;
    dparams->nthreads = ctx->cfg->nthreads;

    memcpy(b_storage, &BLOSC2_STORAGE_DEFAULTS, sizeof(blosc2_storage));
    b_storage->cparams = cparams;
    b_storage->dparams = dparams;

    if (storage->properties.blosc.sequencial) {
        b_storage->contiguous = true;
    }
    if (storage->properties.blosc.urlpath != NULL) {
        b_storage->urlpath = storage->properties.blosc.urlpath;
    }

    return CATERVA_SUCCEED;
}

// Only for internal use
int caterva_blosc_array_new(caterva_ctx_t *ctx, caterva_params_t *params,
                            caterva_storage_t *storage,
                            int special_value, caterva_array_t **array) {
    CATERVA_ERROR(caterva_blosc_array_without_schunk(ctx, params, storage, array));
    blosc2_storage b_storage;
    blosc2_cparams b_cparams;
    blosc2_dparams b_dparams;
    CATERVA_ERROR(caterva_create_blosc_params(ctx, params, storage, &b_cparams, &b_dparams, &b_storage));

    blosc2_schunk *sc = blosc2_schunk_new(&b_storage);
    if (sc == NULL) {
        DEBUG_PRINT("Pointer is NULL");
        return CATERVA_ERR_BLOSC_FAILED;
    }

    // Serialize the dimension info
    if (sc->nmetalayers >= BLOSC2_MAX_METALAYERS) {
        DEBUG_PRINT("the number of metalayers for this schunk has been exceeded");
        return CATERVA_ERR_BLOSC_FAILED;
    }
    uint8_t *smeta = NULL;
    int32_t smeta_len = serialize_meta(params->ndim,
                                       (*array)->shape,
                                       (*array)->chunkshape,
                                       (*array)->blockshape, &smeta);
    if (smeta_len < 0) {
        DEBUG_PRINT("error during serializing dims info for Caterva");
        return CATERVA_ERR_BLOSC_FAILED;
    }

    // And store it in caterva metalayer
    if (blosc2_meta_add(sc, "caterva", smeta, (uint32_t) smeta_len) < 0) {
        return CATERVA_ERR_BLOSC_FAILED;
    }

    free(smeta);

    for (int i = 0; i < storage->properties.blosc.nmetalayers; ++i) {
        char *name = storage->properties.blosc.metalayers[i].name;
        uint8_t *data = storage->properties.blosc.metalayers[i].sdata;
        int32_t size = storage->properties.blosc.metalayers[i].size;
        if (blosc2_meta_add(sc, name, data, size) < 0) {
            CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
        }
    }

    // Fill schunk with uninit values
    if ((*array)->nitems != 0) {
        int32_t chunksize = (int32_t) (*array)->extchunknitems * (*array)->itemsize;
        int64_t nchunks = (*array)->extnitems / (*array)->chunknitems;
        int64_t nitems = nchunks * (*array)->extchunknitems;
        // blosc2_schunk_fill_special(sc, nitems, BLOSC2_SPECIAL_ZERO, chunksize);
        blosc2_schunk_fill_special(sc, nitems, special_value, chunksize);
    }
    (*array)->sc = sc;
    (*array)->nchunks = sc->nchunks;

    return CATERVA_SUCCEED;
}


int caterva_blosc_array_empty(caterva_ctx_t *ctx, caterva_params_t *params,
                              caterva_storage_t *storage, caterva_array_t **array) {
    // CATERVA_ERROR(caterva_blosc_array_new(ctx, params, storage, BLOSC2_SPECIAL_UNINIT, array));
    // Avoid variable cratios
    CATERVA_ERROR(caterva_blosc_array_new(ctx, params, storage, BLOSC2_SPECIAL_ZERO, array));

    return CATERVA_SUCCEED;
}


int caterva_blosc_array_zeros(caterva_ctx_t *ctx, caterva_params_t *params,
                              caterva_storage_t *storage, caterva_array_t **array) {
    CATERVA_ERROR(caterva_blosc_array_new(ctx, params, storage, BLOSC2_SPECIAL_ZERO, array));
    return CATERVA_SUCCEED;
}


int caterva_blosc_array_full(caterva_ctx_t *ctx, caterva_params_t *params,
                              caterva_storage_t *storage, void *fill_value, caterva_array_t **array) {
    CATERVA_ERROR(caterva_empty(ctx, params, storage, array));

    int64_t chunkbytes = (*array)->extchunknitems * (*array)->itemsize;

    blosc2_cparams *cparams;
    if (blosc2_schunk_get_cparams((*array)->sc, &cparams) != 0) {
        CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
    }

    int32_t chunksize = BLOSC_EXTENDED_HEADER_LENGTH + (*array)->itemsize;
    uint8_t *chunk = malloc(chunksize);
    if (blosc2_chunk_repeatval(*cparams, chunkbytes, chunk, chunksize, fill_value) < 0) {
        CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
    }
    free(cparams);

    for (int i = 0; i < (*array)->sc->nchunks; ++i) {
        if (blosc2_schunk_update_chunk((*array)->sc, i, chunk, true) < 0) {
            CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
        }
    }
    free(chunk);

    return CATERVA_SUCCEED;
}


int caterva_config_from_schunk(caterva_ctx_t *ctx, blosc2_schunk *sc, caterva_config_t *cfg) {
    cfg->alloc = ctx->cfg->alloc;
    cfg->free = ctx->cfg->free;

    cfg->complevel = sc->storage->cparams->clevel;
    cfg->compcodec = sc->storage->cparams->compcode;
    cfg->compmeta = sc->storage->cparams->compcode_meta;
    cfg->usedict = sc->storage->cparams->use_dict;
    cfg->splitmode = sc->storage->cparams->splitmode;
    cfg->nthreads = ctx->cfg->nthreads;
    for (int i = 0; i < BLOSC2_MAX_FILTERS; ++i) {
        cfg->filters[i] = sc->storage->cparams->filters[i];
        cfg->filtersmeta[i] = sc->storage->cparams->filters_meta[i];
    }

    cfg->prefilter = ctx->cfg->prefilter;
    cfg->pparams = ctx->cfg->pparams;
    cfg->udbtune = ctx->cfg->udbtune;

    return CATERVA_SUCCEED;
}


int caterva_blosc_from_schunk(caterva_ctx_t *ctx, blosc2_schunk *schunk, caterva_array_t **array) {
    if (ctx == NULL) {
        DEBUG_PRINT("Context is null");
        return CATERVA_ERR_NULL_POINTER;
    }
    if (schunk == NULL) {
        DEBUG_PRINT("Schunk is null");
        return CATERVA_ERR_NULL_POINTER;
    }

    blosc2_cparams *cparams;
    if (blosc2_schunk_get_cparams(schunk, &cparams) < 0) {
        DEBUG_PRINT("Blosc error");
        return CATERVA_ERR_NULL_POINTER;
    }
    uint8_t itemsize = (int8_t) cparams->typesize;
    free(cparams);

    caterva_params_t params = {0};
    params.itemsize = itemsize;
    caterva_storage_t storage = {0};
    storage.backend = CATERVA_STORAGE_BLOSC;
    storage.properties.blosc.urlpath = schunk->storage->urlpath;
    storage.properties.blosc.sequencial = schunk->storage->contiguous;

    // Deserialize the caterva metalayer
    uint8_t *smeta;
    uint32_t smeta_len;
    if (blosc2_meta_get(schunk, "caterva", &smeta, &smeta_len) < 0) {
        DEBUG_PRINT("Blosc error");
        return CATERVA_ERR_BLOSC_FAILED;
    }
    deserialize_meta(smeta, smeta_len, &params.ndim,
                     params.shape,
                     storage.properties.blosc.chunkshape,
                     storage.properties.blosc.blockshape);
    free(smeta);

    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    caterva_config_from_schunk(ctx, schunk, &cfg);

    caterva_ctx_t *ctx_sc;
    caterva_ctx_new(&cfg, &ctx_sc);

    caterva_blosc_array_without_schunk(ctx_sc, &params, &storage, array);

    caterva_ctx_free(&ctx_sc);

    (*array)->sc = schunk;

    return CATERVA_SUCCEED;
}

int caterva_blosc_from_serial_schunk(caterva_ctx_t *ctx, uint8_t *serial_schunk, int64_t len,
                                     caterva_array_t **array) {
    blosc2_schunk *sc = blosc2_schunk_from_buffer(serial_schunk, len, true);
    if (sc == NULL) {
        DEBUG_PRINT("Blosc error");
        return CATERVA_ERR_BLOSC_FAILED;
    }
    // ...and create a caterva array out of it
    CATERVA_ERROR(caterva_from_schunk(ctx, sc, array));

    return CATERVA_SUCCEED;
}

int caterva_blosc_open(caterva_ctx_t *ctx, const char *urlpath, caterva_array_t **array) {
    blosc2_schunk *sc = blosc2_schunk_open(urlpath);

    // ...and create a caterva array out of it
    CATERVA_ERROR(caterva_from_schunk(ctx, sc, array));

    return CATERVA_SUCCEED;
}

int caterva_blosc_array_free(caterva_ctx_t *ctx, caterva_array_t **array) {
    CATERVA_UNUSED_PARAM(ctx);

    if ((*array)->sc != NULL) {
        blosc2_schunk_free((*array)->sc);
    }
    return CATERVA_SUCCEED;
}


// Only for internal use: It is used for setting slices and for getting slices.
int caterva_blosc_slice(caterva_ctx_t *ctx, void *buffer,
                        int64_t buffersize, int64_t *start, int64_t *stop, int64_t *shape,
                        caterva_array_t *array, bool set_slice) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(buffer);
    CATERVA_ERROR_NULL(start);
    CATERVA_ERROR_NULL(stop);
    CATERVA_ERROR_NULL(array);
    if (buffersize < 0) {
        DEBUG_PRINT("buffersize is < 0");
        CATERVA_ERROR(CATERVA_ERR_INVALID_ARGUMENT);
    }

    uint8_t *buffer_b = (uint8_t *) buffer;
    int64_t *buffer_start = start;
    int64_t *buffer_stop = stop;
    int64_t *buffer_shape = shape;

    int8_t ndim = array->ndim;
    int64_t nchunks = array->extnitems / array->chunknitems;

    // 0-dim case
    if (ndim == 0) {
        if (set_slice) {
            uint32_t chunk_size = array->itemsize + BLOSC_MAX_OVERHEAD;
            uint8_t *chunk = malloc(chunk_size);
            if (blosc2_compress_ctx(array->sc->cctx, buffer_b, array->itemsize, chunk, chunk_size) < 0) {
                CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
            }
            if (blosc2_schunk_update_chunk(array->sc, 0, chunk, false) < 0) {
                CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
            }

        } else {
            if (blosc2_schunk_decompress_chunk(array->sc, 0, buffer_b, array->itemsize) < 0) {
                CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
            }
        }
        return CATERVA_SUCCEED;
    }

    int32_t data_nbytes = array->extchunknitems * array->itemsize;
    uint8_t *data = malloc(data_nbytes);

    int64_t chunks_in_array[CATERVA_MAX_DIM] = {0};
    for (int i = 0; i < ndim; ++i) {
        chunks_in_array[i] = array->extshape[i] / array->chunkshape[i];
    }

    int64_t chunks_in_array_strides[CATERVA_MAX_DIM];
    chunks_in_array_strides[ndim - 1] = 1;
    for (int i = ndim - 2; i >= 0; --i) {
        chunks_in_array_strides[i] = chunks_in_array_strides[i + 1] * chunks_in_array[i + 1];
    }

    int64_t blocks_in_chunk[CATERVA_MAX_DIM] = {0};
    for (int i = 0; i < ndim; ++i) {
        blocks_in_chunk[i] = array->extchunkshape[i] / array->blockshape[i];
    }

    // Compute the number of chunks to update
    int64_t update_start[CATERVA_MAX_DIM];
    int64_t update_shape[CATERVA_MAX_DIM];

    int64_t update_nchunks = 1;
    for (int i = 0; i < ndim; ++i) {
        int64_t pos = 0;
        while (pos <= buffer_start[i]) {
            pos += array->chunkshape[i];
        }
        update_start[i] = pos / array->chunkshape[i] - 1;
        while (pos < buffer_stop[i]) {
            pos += array->chunkshape[i];
        }
        update_shape[i] = pos / array->chunkshape[i] - update_start[i];
        update_nchunks *= update_shape[i];
    }

    for (int update_nchunk = 0; update_nchunk < update_nchunks; ++update_nchunk) {
        int64_t nchunk_ndim[CATERVA_MAX_DIM] = {0};
        index_unidim_to_multidim(ndim, update_shape, update_nchunk, nchunk_ndim);
        for (int i = 0; i < ndim; ++i) {
            nchunk_ndim[i] += update_start[i];
        }
        int64_t nchunk;
        index_multidim_to_unidim(nchunk_ndim, ndim, chunks_in_array_strides, &nchunk);

        // check if the chunk needs to be updated
        int64_t chunk_start[CATERVA_MAX_DIM] = {0};
        int64_t chunk_stop[CATERVA_MAX_DIM] = {0};
        for (int i = 0; i < ndim; ++i) {
            chunk_start[i] = nchunk_ndim[i] * array->chunkshape[i];
            chunk_stop[i] = chunk_start[i] + array->chunkshape[i];
            if (chunk_stop[i] > array->shape[i]) {
                chunk_stop[i] = array->shape[i];
            }
        }
        bool chunk_empty = false;
        for (int i = 0; i < ndim; ++i) {
            chunk_empty |= (chunk_stop[i] <= buffer_start[i] || chunk_start[i] >= buffer_stop[i]);
        }
        if (chunk_empty) {
            continue;
        }

        int64_t nblocks = array->extchunknitems / array->blocknitems;



        if (set_slice) {
            // Check if all the chunk is going to be updated and avoid the decompression
            bool decompress_chunk = false;
            for (int i = 0; i < ndim; ++i) {
                decompress_chunk |= (chunk_start[i] < buffer_start[i] || chunk_stop[i] > buffer_stop[i]);
            }

            if (decompress_chunk) {
                int err = blosc2_schunk_decompress_chunk(array->sc, nchunk, data, data_nbytes);
                if (err < 0) {
                    DEBUG_PRINT("Error decompressing chunk");
                    CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
                }
            } else {
                // memset(data, 0, data_nbytes);
            }
        } else {
            bool *block_maskout = ctx->cfg->alloc(nblocks);
            CATERVA_ERROR_NULL(block_maskout);
            for (int nblock = 0; nblock < nblocks; ++nblock) {
                int64_t nblock_ndim[CATERVA_MAX_DIM] = {0};
                index_unidim_to_multidim(ndim, blocks_in_chunk, nblock, nblock_ndim);

                // check if the block needs to be updated
                int64_t block_start[CATERVA_MAX_DIM] = {0};
                int64_t block_stop[CATERVA_MAX_DIM] = {0};
                for (int i = 0; i < ndim; ++i) {
                    block_start[i] = nblock_ndim[i] * array->blockshape[i];
                    block_stop[i] = block_start[i] + array->blockshape[i];
                    block_start[i] += chunk_start[i];
                    block_stop[i] += chunk_start[i];

                    if (block_start[i] > chunk_stop[i]) {
                        block_start[i] = chunk_stop[i];
                    }
                    if (block_stop[i] > chunk_stop[i]) {
                        block_stop[i] = chunk_stop[i];
                    }
                }

                bool block_empty = false;
                for (int i = 0; i < ndim; ++i) {
                    block_empty |= (block_stop[i] <= start[i] || block_start[i] >= stop[i]);
                }
                block_maskout[nblock] = block_empty ? true : false;
            }

            if (blosc2_set_maskout(array->sc->dctx, block_maskout, nblocks) != BLOSC2_ERROR_SUCCESS) {
                DEBUG_PRINT("Error setting the maskout");
                CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
            }

            int err = blosc2_schunk_decompress_chunk(array->sc, nchunk, data, data_nbytes);
            if (err < 0) {
                DEBUG_PRINT("Error decompressing chunk");
                CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
            }

            ctx->cfg->free(block_maskout);
        }

        // Iterate over blocks

        for (int nblock = 0; nblock < nblocks; ++nblock) {
            int64_t nblock_ndim[CATERVA_MAX_DIM] = {0};
            index_unidim_to_multidim(ndim, blocks_in_chunk, nblock, nblock_ndim);

            // check if the block needs to be updated
            int64_t block_start[CATERVA_MAX_DIM] = {0};
            int64_t block_stop[CATERVA_MAX_DIM] = {0};
            for (int i = 0; i < ndim; ++i) {
                block_start[i] = nblock_ndim[i] * array->blockshape[i];
                block_stop[i] = block_start[i] + array->blockshape[i];
                block_start[i] += chunk_start[i];
                block_stop[i] += chunk_start[i];

                if (block_start[i] > chunk_stop[i]) {
                    block_start[i] = chunk_stop[i];
                }
                if (block_stop[i] > chunk_stop[i]) {
                    block_stop[i] = chunk_stop[i];
                }
            }
            int64_t block_shape[CATERVA_MAX_DIM] = {0};
            for (int i = 0; i < ndim; ++i) {
                block_shape[i] = block_stop[i] - block_start[i];
            }
            bool block_empty = false;
            for (int i = 0; i < ndim; ++i) {
                block_empty |= (block_stop[i] <= start[i] || block_start[i] >= stop[i]);
            }
            if (block_empty) {
                continue;
            }

            // compute the start of the slice inside the block
            int64_t slice_start[CATERVA_MAX_DIM] = {0};
            for (int i = 0; i < ndim; ++i) {
                if (block_start[i] < buffer_start[i]) {
                    slice_start[i] = buffer_start[i] - block_start[i];
                } else {
                    slice_start[i] = 0;
                }
                slice_start[i] += block_start[i];
            }

            int64_t slice_stop[CATERVA_MAX_DIM] = {0};
            for (int i = 0; i < ndim; ++i) {
                if (block_stop[i] > buffer_stop[i]) {
                    slice_stop[i] = block_shape[i] - (block_stop[i] - buffer_stop[i]);
                } else {
                    slice_stop[i] = block_stop[i] - block_start[i];
                }
                slice_stop[i] += block_start[i];
            }

            int64_t slice_shape[CATERVA_MAX_DIM] = {0};
            for (int i = 0; i < ndim; ++i) {
                slice_shape[i] = slice_stop[i] - slice_start[i];
            }


            uint8_t *src = &buffer_b[0];
            int64_t *src_pad_shape = buffer_shape;

            int64_t src_start[CATERVA_MAX_DIM] = {0};
            int64_t src_stop[CATERVA_MAX_DIM] = {0};
            for (int i = 0; i < ndim; ++i) {
                src_start[i] = slice_start[i] - buffer_start[i];
                src_stop[i] = slice_stop[i] - buffer_start[i];
            }

            uint8_t *dst = &data[nblock * array->blocknitems * array->itemsize];
            int64_t dst_pad_shape[CATERVA_MAX_DIM];
            for (int i = 0; i < ndim; ++i) {
                dst_pad_shape[i] = array->blockshape[i];
            }

            int64_t dst_start[CATERVA_MAX_DIM] = {0};
            int64_t dst_stop[CATERVA_MAX_DIM] = {0};
            for (int i = 0; i < ndim; ++i) {
                dst_start[i] = slice_start[i] - block_start[i];
                dst_stop[i] = dst_start[i] + slice_shape[i];
            }

            if (set_slice) {
                caterva_copy_buffer(ndim, array->itemsize,
                                    src, src_pad_shape, src_start, src_stop,
                                    dst, dst_pad_shape, dst_start);
            } else {
                caterva_copy_buffer(ndim, array->itemsize,
                                    dst, dst_pad_shape, dst_start, dst_stop,
                                    src, src_pad_shape, src_start);
            }
        }

        if (set_slice) {
            // Recompress the data
            int32_t chunk_nbytes = data_nbytes + BLOSC_MAX_OVERHEAD;
            uint8_t *chunk = malloc(chunk_nbytes);
            int brc;
            brc = blosc2_compress_ctx(array->sc->cctx, data, data_nbytes, chunk, chunk_nbytes);
            if (brc < 0) {
                DEBUG_PRINT("Blosc can not compress the data");
                CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
            }
            brc = blosc2_schunk_update_chunk(array->sc, nchunk, chunk, false);
            if (brc < 0) {
                DEBUG_PRINT("Blosc can not update the chunk");
                CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
            }
        }
    }

    free(data);


    return CATERVA_SUCCEED;
}

int caterva_blosc_array_set_slice_buffer(caterva_ctx_t *ctx, void *buffer, int64_t buffersize,
                                         int64_t *start, int64_t *stop, int64_t *shape,
                                         caterva_array_t *array) {
    CATERVA_ERROR(caterva_blosc_slice(ctx, buffer, buffersize, start, stop, shape, array, true));

    return CATERVA_SUCCEED;
}

int caterva_blosc_array_get_slice_buffer(caterva_ctx_t *ctx, caterva_array_t *array,
                                         int64_t *start, int64_t *stop, int64_t *shape,
                                         void *buffer, int64_t buffersize) {
    CATERVA_ERROR(caterva_blosc_slice(ctx, buffer, buffersize, start, stop, shape, array, false));

    return CATERVA_SUCCEED;
}

int caterva_blosc_array_to_buffer(caterva_ctx_t *ctx, caterva_array_t *array, void *buffer,
                                  int64_t buffersize) {
    int64_t start[CATERVA_MAX_DIM] = {0};
    int64_t *stop = array->shape;
    CATERVA_ERROR(caterva_get_slice_buffer(ctx, array, start, stop,
                                           buffer, array->shape, buffersize));

    return CATERVA_SUCCEED;
}

int caterva_blosc_array_get_slice(caterva_ctx_t *ctx, caterva_array_t *src,
                                  int64_t *start, int64_t *stop,
                                  caterva_storage_t *storage, caterva_array_t **array) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(src);
    CATERVA_ERROR_NULL(start);
    CATERVA_ERROR_NULL(stop);
    CATERVA_ERROR_NULL(storage);

    caterva_params_t params;
    params.ndim = src->ndim;
    params.itemsize = src->itemsize;
    for (int i = 0; i < src->ndim; ++i) {
        params.shape[i] = stop[i] - start[i];
    }

    // Add data
    CATERVA_ERROR(caterva_empty(ctx, &params, storage, array));

    if ((*array)->nitems == 0) {
        return CATERVA_SUCCEED;
    }

    uint8_t ndim = (*array)->ndim;
    int64_t chunks_in_array[CATERVA_MAX_DIM] = {0};
    for (int i = 0; i < ndim; ++i) {
        chunks_in_array[i] = (*array)->extshape[i] / (*array)->chunkshape[i];
    }
    int64_t nchunks = (*array)->sc->nchunks;
    for (int nchunk = 0; nchunk < nchunks; ++nchunk) {
        int64_t nchunk_ndim[CATERVA_MAX_DIM] = {0};
        index_unidim_to_multidim(ndim, chunks_in_array, nchunk, nchunk_ndim);

        // check if the chunk needs to be updated
        int64_t chunk_start[CATERVA_MAX_DIM] = {0};
        int64_t chunk_stop[CATERVA_MAX_DIM] = {0};
        int64_t chunk_shape[CATERVA_MAX_DIM] = {0};
        for (int i = 0; i < ndim; ++i) {
            chunk_start[i] = nchunk_ndim[i] * (*array)->chunkshape[i];
            chunk_stop[i] = chunk_start[i] + (*array)->chunkshape[i];
            if (chunk_stop[i] > (*array)->shape[i]) {
                chunk_stop[i] = (*array)->shape[i];
            }
            chunk_shape[i] = chunk_stop[i] - chunk_start[i];
        }

        int64_t src_start[CATERVA_MAX_DIM] = {0};
        int64_t src_stop[CATERVA_MAX_DIM] = {0};
        for (int i = 0; i < ndim; ++i) {
            src_start[i] = chunk_start[i] + start[i];
            src_stop[i] = chunk_stop[i] + start[i];
        }
        int64_t buffersize = params.itemsize;
        for (int i = 0; i < ndim; ++i) {
            buffersize *= chunk_shape[i];
        }
        uint8_t *buffer = ctx->cfg->alloc(buffersize);
        CATERVA_ERROR(caterva_get_slice_buffer(ctx, src, src_start, src_stop, buffer, chunk_shape,
                                               buffersize));
        CATERVA_ERROR(caterva_set_slice_buffer(ctx, buffer, chunk_shape, buffersize, chunk_start,
                                               chunk_stop, *array));
        ctx->cfg->free(buffer);
    }

    return CATERVA_SUCCEED;
}



int caterva_blosc_array_copy(caterva_ctx_t *ctx, caterva_params_t *params,
                             caterva_storage_t *storage, caterva_array_t *src,
                             caterva_array_t **dest) {
    CATERVA_UNUSED_PARAM(params);

    bool equals = true;
    if (src->storage == CATERVA_STORAGE_PLAINBUFFER) {
        equals = false;
    }
    for (int i = 0; i < src->ndim; ++i) {
        if (src->chunkshape[i] != storage->properties.blosc.chunkshape[i]) {
            equals = false;
            break;
        }
        if (src->blockshape[i] != storage->properties.blosc.blockshape[i]) {
            equals = false;
            break;
        }
    }

    if (equals) {
        CATERVA_ERROR(caterva_blosc_array_without_schunk(ctx, params, storage, dest));
        blosc2_storage b_storage;
        blosc2_cparams cparams;
        blosc2_dparams dparams;
        CATERVA_ERROR(
                caterva_create_blosc_params(ctx, params, storage, &cparams, &dparams, &b_storage));
        blosc2_schunk *new_sc = blosc2_schunk_copy(src->sc, &b_storage);

        if (new_sc == NULL) {
            return CATERVA_ERR_BLOSC_FAILED;
        }
        (*dest)->sc = new_sc;

    } else {
        int64_t start[CATERVA_MAX_DIM] = {0, 0, 0, 0, 0, 0, 0, 0};

        int64_t stop[CATERVA_MAX_DIM];
        for (int i = 0; i < src->ndim; ++i) {
            stop[i] = src->shape[i];
        }
        // Copy metalayers
        caterva_storage_t storage_meta;
        memcpy(&storage_meta, storage, sizeof(storage_meta));
        int j = 0;
        if (src->storage == CATERVA_STORAGE_BLOSC) {
            for (int i = 0; i < src->sc->nmetalayers; ++i) {
                if (strcmp(src->sc->metalayers[i]->name, "caterva") == 0) {
                    continue;
                }
                caterva_metalayer_t *meta = &storage_meta.properties.blosc.metalayers[j];
                meta->name = src->sc->metalayers[i]->name;
                meta->sdata = src->sc->metalayers[i]->content;
                meta->size = src->sc->metalayers[i]->content_len;
                j++;
            }
            storage_meta.properties.blosc.nmetalayers = j;
        }
        // Copy data
        CATERVA_ERROR(caterva_get_slice(ctx, src, start, stop, &storage_meta, dest));

        // Copy vlmetayers
        if (src->storage == CATERVA_STORAGE_BLOSC) {
            for (int i = 0; i < src->sc->nvlmetalayers; ++i) {
                uint8_t *content;
                uint32_t content_len;
                if (blosc2_vlmeta_get(src->sc, src->sc->vlmetalayers[i]->name, &content,
                                      &content_len) < 0) {
                    CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
                }
                caterva_metalayer_t vlmeta;
                vlmeta.name = src->sc->vlmetalayers[i]->name;
                vlmeta.sdata = content;
                vlmeta.size = (int32_t) content_len;
                CATERVA_ERROR(caterva_vlmeta_add(ctx, *dest, &vlmeta));
                free(content);
            }
        }
    }

    return CATERVA_SUCCEED;
}


// Squeeze functions

int caterva_blosc_array_squeeze_index(caterva_ctx_t *ctx, caterva_array_t *array, bool *index) {
    CATERVA_UNUSED_PARAM(ctx);
    uint8_t nones = 0;
    int64_t newshape[CATERVA_MAX_DIM];
    int32_t newchunkshape[CATERVA_MAX_DIM];
    int32_t newblockshape[CATERVA_MAX_DIM];

    for (int i = 0; i < array->ndim; ++i) {
        if (index[i] == true) {
            if (array->shape[i] != 1) {
                CATERVA_ERROR(CATERVA_ERR_INVALID_INDEX);
            }
        } else {
            newshape[nones] = array->shape[i];
            newchunkshape[nones] = array->chunkshape[i];
            newblockshape[nones] = array->blockshape[i];
            nones += 1;
        }
    }

    for (int i = 0; i < CATERVA_MAX_DIM; ++i) {
        if (i < nones) {
            array->chunkshape[i] = newchunkshape[i];
            array->blockshape[i] = newblockshape[i];
        } else {
            array->chunkshape[i] = 1;
            array->blockshape[i] = 1;
        }
    }

    CATERVA_ERROR(caterva_blosc_update_shape(array, nones, newshape, newchunkshape, newblockshape));

    return CATERVA_SUCCEED;
}


int caterva_blosc_array_squeeze(caterva_ctx_t *ctx, caterva_array_t *array) {
    CATERVA_UNUSED_PARAM(ctx);
    bool index[CATERVA_MAX_DIM];

    for (int i = 0; i < array->ndim; ++i) {
        if (array->shape[i] != 1) {
           index[i] = false;
        } else {
            index[i] = true;
        }
    }
    CATERVA_ERROR(caterva_blosc_array_squeeze_index(ctx, array, index));

    return CATERVA_SUCCEED;
}


int caterva_blosc_remove(caterva_ctx_t *ctx, char *urlpath) {
    CATERVA_ERROR_NULL(ctx);
    CATERVA_ERROR_NULL(urlpath);

    int rc = blosc2_remove_urlpath(urlpath);
    if (rc != BLOSC2_ERROR_SUCCESS) {
        CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
    }
    return CATERVA_SUCCEED;
}

int caterva_blosc_save(caterva_ctx_t *ctx, caterva_array_t *array, char *urlpath) {
    caterva_array_t *tmp;
    caterva_storage_t storage;
    storage.backend = CATERVA_STORAGE_BLOSC;
    caterva_storage_properties_blosc_t *blosc_prop = &storage.properties.blosc;
    blosc_prop->urlpath = urlpath;
    blosc_prop->sequencial = array->sc->storage->contiguous;

    for (int i = 0; i < array->ndim; ++i) {
        blosc_prop->chunkshape[i] = array->chunkshape[i];
        blosc_prop->blockshape[i] = array->blockshape[i];
    }

    caterva_copy(ctx, array, &storage, &tmp);
    caterva_free(ctx, &tmp);

    return CATERVA_SUCCEED;
}


// VLMetalayers

int caterva_blosc_vlmeta_add(caterva_ctx_t *ctx, caterva_array_t *array,
                             const char *name, uint8_t *content, uint32_t content_len) {
    CATERVA_UNUSED_PARAM(ctx);
    blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
    if (blosc2_vlmeta_add(array->sc, name, content, content_len, &cparams) < 0) {
        CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
    }
    return CATERVA_SUCCEED;
}

int caterva_blosc_vlmeta_get(caterva_ctx_t *ctx, caterva_array_t *array,
                             const char *name, uint8_t **content, int32_t *content_len) {
    CATERVA_UNUSED_PARAM(ctx);
    if (blosc2_vlmeta_get(array->sc, name, content, content_len) < 0) {
        CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
    }
    return CATERVA_SUCCEED;
}

int caterva_blosc_vlmeta_exists(caterva_ctx_t *ctx, caterva_array_t *array,
                                const char *name, bool *exists) {
    CATERVA_UNUSED_PARAM(ctx);
    if (blosc2_vlmeta_exists(array->sc, name) < 0) {
        *exists = false;
    } else {
        *exists = true;
    }
    return CATERVA_SUCCEED;
}

int caterva_blosc_vlmeta_update(caterva_ctx_t *ctx, caterva_array_t *array,
                                const char *name, uint8_t *content, uint32_t content_len) {
    CATERVA_UNUSED_PARAM(ctx);
    blosc2_cparams cparams = BLOSC2_CPARAMS_DEFAULTS;
    if (blosc2_vlmeta_update(array->sc, name, content, content_len, &cparams) < 0) {
        CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
    }
    return CATERVA_SUCCEED;
}

int caterva_blosc_meta_get(caterva_ctx_t *ctx, caterva_array_t *array,
                             const char *name, uint8_t **content, int32_t *content_len) {
    CATERVA_UNUSED_PARAM(ctx);
    if (blosc2_meta_get(array->sc, name, content, content_len) < 0) {
        CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
    }
    return CATERVA_SUCCEED;
}

int caterva_blosc_meta_exists(caterva_ctx_t *ctx, caterva_array_t *array,
                                const char *name, bool *exists) {
    CATERVA_UNUSED_PARAM(ctx);
    if (blosc2_meta_exists(array->sc, name) < 0) {
        *exists = false;
    } else {
        *exists = true;
    }
    return CATERVA_SUCCEED;
}

int caterva_blosc_meta_update(caterva_ctx_t *ctx, caterva_array_t *array,
                                const char *name, uint8_t *content, int32_t content_len) {
    CATERVA_UNUSED_PARAM(ctx);
    if (blosc2_meta_update(array->sc, name, content, content_len) < 0) {
        CATERVA_ERROR(CATERVA_ERR_BLOSC_FAILED);
    }
    return CATERVA_SUCCEED;
}
