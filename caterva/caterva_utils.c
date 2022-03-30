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
#include <caterva_utils.h>

void index_unidim_to_multidim(int8_t ndim, int64_t *shape, int64_t i, int64_t *index) {
    int64_t strides[CATERVA_MAX_DIM];
    if (ndim == 0) {
        return;
    }
    strides[ndim - 1] = 1;
    for (int j = ndim - 2; j >= 0; --j) {
        strides[j] = shape[j + 1] * strides[j + 1];
    }

    index[0] = i / strides[0];
    for (int j = 1; j < ndim; ++j) {
        index[j] = (i % strides[j - 1]) / strides[j];
    }
}

void index_multidim_to_unidim(int64_t *index, int8_t ndim, int64_t *strides, int64_t *i) {
    *i = 0;
    for (int j = 0; j < ndim; ++j) {
        *i += index[j] * strides[j];
    }
}

// big <-> little-endian and store it in a memory position.  Sizes supported: 1, 2, 4, 8 bytes.
void swap_store(void *dest, const void *pa, int size) {
    uint8_t *pa_ = (uint8_t *) pa;
    uint8_t *pa2_ = malloc((size_t) size);
    int i = 1; /* for big/little endian detection */
    char *p = (char *) &i;

    if (p[0] == 1) {
        /* little endian */
        switch (size) {
            case 8:
                pa2_[0] = pa_[7];
                pa2_[1] = pa_[6];
                pa2_[2] = pa_[5];
                pa2_[3] = pa_[4];
                pa2_[4] = pa_[3];
                pa2_[5] = pa_[2];
                pa2_[6] = pa_[1];
                pa2_[7] = pa_[0];
                break;
            case 4:
                pa2_[0] = pa_[3];
                pa2_[1] = pa_[2];
                pa2_[2] = pa_[1];
                pa2_[3] = pa_[0];
                break;
            case 2:
                pa2_[0] = pa_[1];
                pa2_[1] = pa_[0];
                break;
            case 1:
                pa2_[0] = pa_[0];
                break;
            default:
                fprintf(stderr, "Unhandled nitems: %d\n", size);
        }
    }
    memcpy(dest, pa2_, size);
    free(pa2_);
}

int32_t serialize_meta(uint8_t ndim, int64_t *shape, const int32_t *chunkshape,
                              const int32_t *blockshape, uint8_t **smeta) {
    // Allocate space for Caterva metalayer
    int32_t max_smeta_len = 1 + 1 + 1 + (1 + ndim * (1 + sizeof(int64_t))) +
                            (1 + ndim * (1 + sizeof(int32_t))) + (1 + ndim * (1 + sizeof(int32_t)));
    *smeta = malloc((size_t) max_smeta_len);
    CATERVA_ERROR_NULL(smeta);
    uint8_t *pmeta = *smeta;

    // Build an array with 5 entries (version, ndim, shape, chunkshape, blockshape)
    *pmeta++ = 0x90 + 5;

    // version entry
    *pmeta++ = CATERVA_METALAYER_VERSION;  // positive fixnum (7-bit positive integer)

    // ndim entry
    *pmeta++ = (uint8_t) ndim;  // positive fixnum (7-bit positive integer)

    // shape entry
    *pmeta++ = (uint8_t)(0x90) + ndim;  // fix array with ndim elements
    for (uint8_t i = 0; i < ndim; i++) {
        *pmeta++ = 0xd3;  // int64
        swap_store(pmeta, shape + i, sizeof(int64_t));
        pmeta += sizeof(int64_t);
    }

    // chunkshape entry
    *pmeta++ = (uint8_t)(0x90) + ndim;  // fix array with ndim elements
    for (uint8_t i = 0; i < ndim; i++) {
        *pmeta++ = 0xd2;  // int32
        swap_store(pmeta, chunkshape + i, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }

    // blockshape entry
    *pmeta++ = (uint8_t)(0x90) + ndim;  // fix array with ndim elements
    for (uint8_t i = 0; i < ndim; i++) {
        *pmeta++ = 0xd2;  // int32
        swap_store(pmeta, blockshape + i, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    int32_t slen = (int32_t)(pmeta - *smeta);

    return slen;
}

int32_t deserialize_meta(uint8_t *smeta, uint32_t smeta_len, uint8_t *ndim, int64_t *shape,
                                int32_t *chunkshape, int32_t *blockshape) {
    uint8_t *pmeta = smeta;
    CATERVA_UNUSED_PARAM(smeta_len);

    // Check that we have an array with 5 entries (version, ndim, shape, chunkshape, blockshape)
    pmeta += 1;

    // version entry
    int8_t version = pmeta[0];  // positive fixnum (7-bit positive integer)
    CATERVA_UNUSED_PARAM(version);

    pmeta += 1;

    // ndim entry
    *ndim = pmeta[0];
    int8_t ndim_aux = *ndim;  // positive fixnum (7-bit positive integer)
    pmeta += 1;

    // shape entry
    // Initialize to ones, as required by Caterva
    for (int i = 0; i < CATERVA_MAX_DIM; i++) shape[i] = 1;
    pmeta += 1;
    for (int8_t i = 0; i < ndim_aux; i++) {
        pmeta += 1;
        swap_store(shape + i, pmeta, sizeof(int64_t));
        pmeta += sizeof(int64_t);
    }

    // chunkshape entry
    // Initialize to ones, as required by Caterva
    for (int i = 0; i < CATERVA_MAX_DIM; i++) chunkshape[i] = 1;
    pmeta += 1;
    for (int8_t i = 0; i < ndim_aux; i++) {
        pmeta += 1;
        swap_store(chunkshape + i, pmeta, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }

    // blockshape entry
    // Initialize to ones, as required by Caterva
    for (int i = 0; i < CATERVA_MAX_DIM; i++) blockshape[i] = 1;
    pmeta += 1;
    for (int8_t i = 0; i < ndim_aux; i++) {
        pmeta += 1;
        swap_store(blockshape + i, pmeta, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    uint32_t slen = (uint32_t)(pmeta - smeta);
    CATERVA_UNUSED_PARAM(slen);

    return 0;
}

// copyNdim where N = {2-8} - specializations of copy loops to be used by caterva_copy_buffer
// since we don't have c++ templates, substitute manual specializations for up to known CATERVA_MAX_DIM (8)
// it's not pretty, but it substantially reduces overhead vs. the generic method
void copy8dim(const uint8_t itemsize, const int64_t* copy_shape,
uint8_t *bdst, const uint8_t *bsrc, const int64_t* src_strides, const int64_t* dst_strides) {
    int64_t copy_nbytes = copy_shape[7] * itemsize;
    int64_t copy_start[7] = {0};
    do {
    do {
    do {
    do {
    do {
    do {
    do {
        int64_t src_copy_start = 0;
        int64_t dst_copy_start = 0;
        for (int j = 0; j < 7; ++j) {
            src_copy_start += copy_start[j] * src_strides[j];
            dst_copy_start += copy_start[j] * dst_strides[j];
        }

        memcpy(&bdst[dst_copy_start * itemsize], &bsrc[src_copy_start * itemsize], copy_nbytes);
    ++copy_start[6]; } while(copy_start[6] < copy_shape[6]);
    ++copy_start[5]; copy_start[6] = 0; } while(copy_start[5] < copy_shape[5]);
    ++copy_start[4]; copy_start[5] = 0; } while(copy_start[4] < copy_shape[4]);
    ++copy_start[3]; copy_start[4] = 0; } while(copy_start[3] < copy_shape[3]);
    ++copy_start[2]; copy_start[3] = 0; } while(copy_start[2] < copy_shape[2]);
    ++copy_start[1]; copy_start[2] = 0; } while(copy_start[1] < copy_shape[1]);
    ++copy_start[0]; copy_start[1] = 0; } while(copy_start[0] < copy_shape[0]);
}

void copy7dim(const uint8_t itemsize, const int64_t* copy_shape,
uint8_t *bdst, const uint8_t *bsrc, const int64_t* src_strides, const int64_t* dst_strides) {
    int64_t copy_nbytes = copy_shape[6] * itemsize;
    int64_t copy_start[6] = {0};
    do {
    do {
    do {
    do {
    do {
    do {
        int64_t src_copy_start = 0;
        int64_t dst_copy_start = 0;
        for (int j = 0; j < 6; ++j) {
            src_copy_start += copy_start[j] * src_strides[j];
            dst_copy_start += copy_start[j] * dst_strides[j];
        }

        memcpy(&bdst[dst_copy_start * itemsize], &bsrc[src_copy_start * itemsize], copy_nbytes);
    ++copy_start[5]; } while(copy_start[5] < copy_shape[5]);
    ++copy_start[4]; copy_start[5] = 0; } while(copy_start[4] < copy_shape[4]);
    ++copy_start[3]; copy_start[4] = 0; } while(copy_start[3] < copy_shape[3]);
    ++copy_start[2]; copy_start[3] = 0; } while(copy_start[2] < copy_shape[2]);
    ++copy_start[1]; copy_start[2] = 0; } while(copy_start[1] < copy_shape[1]);
    ++copy_start[0]; copy_start[1] = 0; } while(copy_start[0] < copy_shape[0]);
}

void copy6dim(const uint8_t itemsize, const int64_t* copy_shape,
uint8_t *bdst, const uint8_t *bsrc, const int64_t* src_strides, const int64_t* dst_strides) {
    int64_t copy_nbytes = copy_shape[5] * itemsize;
    int64_t copy_start[5] = {0};
    do {
    do {
    do {
    do {
    do {
        int64_t src_copy_start = 0;
        int64_t dst_copy_start = 0;
        for (int j = 0; j < 5; ++j) {
            src_copy_start += copy_start[j] * src_strides[j];
            dst_copy_start += copy_start[j] * dst_strides[j];
        }

        memcpy(&bdst[dst_copy_start * itemsize], &bsrc[src_copy_start * itemsize], copy_nbytes);
    ++copy_start[4]; } while(copy_start[4] < copy_shape[4]);
    ++copy_start[3]; copy_start[4] = 0; } while(copy_start[3] < copy_shape[3]);
    ++copy_start[2]; copy_start[3] = 0; } while(copy_start[2] < copy_shape[2]);
    ++copy_start[1]; copy_start[2] = 0; } while(copy_start[1] < copy_shape[1]);
    ++copy_start[0]; copy_start[1] = 0; } while(copy_start[0] < copy_shape[0]);
}

void copy5dim(const uint8_t itemsize, const int64_t* copy_shape,
uint8_t *bdst, const uint8_t *bsrc, const int64_t* src_strides, const int64_t* dst_strides) {
    int64_t copy_nbytes = copy_shape[4] * itemsize;
    int64_t copy_start[4] = {0};
    do {
    do {
    do {
    do {
        int64_t src_copy_start = 0;
        int64_t dst_copy_start = 0;
        for (int j = 0; j < 4; ++j) {
            src_copy_start += copy_start[j] * src_strides[j];
            dst_copy_start += copy_start[j] * dst_strides[j];
        }

        memcpy(&bdst[dst_copy_start * itemsize], &bsrc[src_copy_start * itemsize], copy_nbytes);
    ++copy_start[3]; } while(copy_start[3] < copy_shape[3]);
    ++copy_start[2]; copy_start[3] = 0; } while(copy_start[2] < copy_shape[2]);
    ++copy_start[1]; copy_start[2] = 0; } while(copy_start[1] < copy_shape[1]);
    ++copy_start[0]; copy_start[1] = 0; } while(copy_start[0] < copy_shape[0]);
}

void copy4dim(const uint8_t itemsize, const int64_t* copy_shape,
uint8_t *bdst, const uint8_t *bsrc, const int64_t* src_strides, const int64_t* dst_strides) {
    int64_t copy_nbytes = copy_shape[3] * itemsize;
    int64_t copy_start[3] = {0};
    do {
    do {
    do {
        int64_t src_copy_start = 0;
        int64_t dst_copy_start = 0;
        for (int j = 0; j < 3; ++j) {
            src_copy_start += copy_start[j] * src_strides[j];
            dst_copy_start += copy_start[j] * dst_strides[j];
        }

        memcpy(&bdst[dst_copy_start * itemsize], &bsrc[src_copy_start * itemsize], copy_nbytes);
    ++copy_start[2]; } while(copy_start[2] < copy_shape[2]);
    ++copy_start[1]; copy_start[2] = 0; } while(copy_start[1] < copy_shape[1]);
    ++copy_start[0]; copy_start[1] = 0; } while(copy_start[0] < copy_shape[0]);
}

void copy3dim(const uint8_t itemsize, const int64_t* copy_shape,
uint8_t *bdst, const uint8_t *bsrc, const int64_t* src_strides, const int64_t* dst_strides) {
    int64_t copy_nbytes = copy_shape[2] * itemsize;
    int64_t copy_start[2] = {0};
    do {
    do {
        int64_t src_copy_start = 0;
        int64_t dst_copy_start = 0;
        for (int j = 0; j < 2; ++j) {
            src_copy_start += copy_start[j] * src_strides[j];
            dst_copy_start += copy_start[j] * dst_strides[j];
        }

        memcpy(&bdst[dst_copy_start * itemsize], &bsrc[src_copy_start * itemsize], copy_nbytes);
    ++copy_start[1]; } while(copy_start[1] < copy_shape[1]);
    ++copy_start[0]; copy_start[1] = 0; } while(copy_start[0] < copy_shape[0]);
}

void copy2dim(const uint8_t itemsize, const int64_t* copy_shape,
uint8_t *bdst, const uint8_t *bsrc, const int64_t* src_strides, const int64_t* dst_strides) {
    int64_t copy_nbytes = copy_shape[1] * itemsize;
    int64_t copy_start = 0;
    do {
        int64_t src_copy_start = copy_start * src_strides[0];
        int64_t dst_copy_start = copy_start * dst_strides[0];
        memcpy(&bdst[dst_copy_start * itemsize], &bsrc[src_copy_start * itemsize], copy_nbytes);
    ++copy_start; } while(copy_start < copy_shape[0]);
}

int caterva_copy_buffer(uint8_t ndim,
                        uint8_t itemsize,
                        void *src, int64_t *src_pad_shape,
                        int64_t *src_start, int64_t *src_stop,
                        void *dst, int64_t *dst_pad_shape,
                        int64_t *dst_start) {
    // Compute the shape of the copy
    int64_t copy_shape[CATERVA_MAX_DIM] = {0};
    for (int i = 0; i < ndim; ++i) {
        copy_shape[i] = src_stop[i] - src_start[i];
        if(copy_shape[i] == 0) {
            return CATERVA_SUCCEED;
        }
    }

    // Compute the strides
    int64_t src_strides[CATERVA_MAX_DIM];
    src_strides[ndim - 1] = 1;
    for (int i = ndim - 2; i >= 0; --i) {
        src_strides[i] = src_strides[i + 1] * src_pad_shape[i + 1];
    }

    int64_t dst_strides[CATERVA_MAX_DIM];
    dst_strides[ndim - 1] = 1;
    for (int i = ndim - 2; i >= 0; --i) {
        dst_strides[i] = dst_strides[i + 1] * dst_pad_shape[i + 1];
    }

    // Align the buffers removing unnecessary data
    int64_t src_start_n;
    index_multidim_to_unidim(src_start, ndim, src_strides, &src_start_n);
    uint8_t *bsrc = (uint8_t *) src;
    bsrc = &bsrc[src_start_n * itemsize];

    int64_t dst_start_n;
    index_multidim_to_unidim(dst_start, ndim, dst_strides, &dst_start_n);
    uint8_t *bdst = (uint8_t *) dst;
    bdst = &bdst[dst_start_n * itemsize];

switch(ndim) {
    case 1:
        memcpy(&bdst[0], &bsrc[0], copy_shape[0] * itemsize);
    break;
    case 2:
        copy2dim(itemsize, copy_shape, bdst, bsrc, src_strides, dst_strides);
    break;
    case 3:
        copy3dim(itemsize, copy_shape, bdst, bsrc, src_strides, dst_strides);
    break;
    case 4:
        copy4dim(itemsize, copy_shape, bdst, bsrc, src_strides, dst_strides);
    break;
    case 5:
        copy5dim(itemsize, copy_shape, bdst, bsrc, src_strides, dst_strides);
    break;
    case 6:
        copy6dim(itemsize, copy_shape, bdst, bsrc, src_strides, dst_strides);
    break;
    case 7:
        copy7dim(itemsize, copy_shape, bdst, bsrc, src_strides, dst_strides);
    break;
    case 8:
        copy8dim(itemsize, copy_shape, bdst, bsrc, src_strides, dst_strides);
    break;
    default:
        return CATERVA_ERR_INVALID_INDEX; // guard against potential future increase to CATERVA_MAX_DIM
    break;
    }
    
    return CATERVA_SUCCEED;
}


int create_blosc_params(caterva_ctx_t *ctx,
                        caterva_params_t *params,
                        caterva_storage_t *storage,
                        blosc2_cparams *cparams,
                        blosc2_dparams *dparams,
                        blosc2_storage *b_storage) {
    int32_t blocknitems = 1;
    for (int i = 0; i < params->ndim; ++i) {
        blocknitems *= storage->blockshape[i];
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

    if (storage->sequencial) {
        b_storage->contiguous = true;
    }
    if (storage->urlpath != NULL) {
        b_storage->urlpath = storage->urlpath;
    }

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
