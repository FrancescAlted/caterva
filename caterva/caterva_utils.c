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


int caterva_copy_buffer(uint8_t ndim,
                        uint8_t itemsize,
                        void *src, int64_t *src_pad_shape,
                        int64_t *src_start, int64_t *src_stop,
                        void *dst, int64_t *dst_pad_shape,
                        int64_t *dst_start) {
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

    // Compute the shape of the copy
    int64_t copy_shape[CATERVA_MAX_DIM] = {0};
    for (int i = 0; i < ndim; ++i) {
        copy_shape[i] = src_stop[i] - src_start[i];
    }
    int64_t copy_nbytes = copy_shape[ndim - 1];

    // Copy contiguous memory blocks
    int64_t number_of_copies = 1;
    for (int i = 0; i < ndim - 1; ++i) {
        number_of_copies *= src_stop[i] - src_start[i];
    }
    for (int ncopy = 0; ncopy < number_of_copies; ++ncopy) {
        // Compute the start of the copy
        int64_t copy_start[CATERVA_MAX_DIM] = {0};
        index_unidim_to_multidim(ndim - 1, copy_shape, ncopy, copy_start);

        // Translate this index to the src buffer
        int64_t src_copy_start;
        index_multidim_to_unidim(copy_start, ndim - 1, src_strides, &src_copy_start);

        // Translate this index to the dst buffer
        int64_t dst_copy_start;
        index_multidim_to_unidim(copy_start, ndim - 1, dst_strides, &dst_copy_start);

        // Perform the copy
        memcpy(&bdst[dst_copy_start * itemsize],
               &bsrc[src_copy_start * itemsize],
               copy_nbytes * itemsize);
    }

    return CATERVA_SUCCEED;
}
