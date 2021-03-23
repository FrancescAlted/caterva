//
// Created by sosca on 12/03/2021.
//

#ifndef CATERVA_CATERVA_UTILITIES_H
#define CATERVA_CATERVA_UTILITIES_H

#include <assert.h>
#include <caterva.h>

static void index_unidim_to_multidim(int8_t ndim, int64_t *shape, int64_t i, int64_t *index) {
    int64_t strides[CATERVA_MAX_DIM];
    strides[ndim - 1] = 1;
    for (int j = ndim - 2; j >= 0; --j) {
        strides[j] = shape[j + 1] * strides[j + 1];
    }

    index[0] = i / strides[0];
    for (int j = 1; j < ndim; ++j) {
        index[j] = (i % strides[j - 1]) / strides[j];
    }
}

// big <-> little-endian and store it in a memory position.  Sizes supported: 1, 2, 4, 8 bytes.
static void swap_store(void *dest, const void *pa, int size) {
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

static int32_t serialize_meta(int8_t ndim, int64_t *shape, const int32_t *chunkshape,
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
    assert(pmeta - *smeta < max_smeta_len);

    // ndim entry
    *pmeta++ = (uint8_t) ndim;  // positive fixnum (7-bit positive integer)
    assert(pmeta - *smeta < max_smeta_len);

    // shape entry
    *pmeta++ = (uint8_t)(0x90) + ndim;  // fix array with ndim elements
    for (int8_t i = 0; i < ndim; i++) {
        *pmeta++ = 0xd3;  // int64
        swap_store(pmeta, shape + i, sizeof(int64_t));
        pmeta += sizeof(int64_t);
    }
    assert(pmeta - *smeta < max_smeta_len);

    // chunkshape entry
    *pmeta++ = (uint8_t)(0x90) + ndim;  // fix array with ndim elements
    for (int8_t i = 0; i < ndim; i++) {
        *pmeta++ = 0xd2;  // int32
        swap_store(pmeta, chunkshape + i, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    assert(pmeta - *smeta <= max_smeta_len);

    // blockshape entry
    *pmeta++ = (uint8_t)(0x90) + ndim;  // fix array with ndim elements
    for (int8_t i = 0; i < ndim; i++) {
        *pmeta++ = 0xd2;  // int32
        swap_store(pmeta, blockshape + i, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    assert(pmeta - *smeta <= max_smeta_len);
    int32_t slen = (int32_t)(pmeta - *smeta);

    return slen;
}

static int32_t deserialize_meta(uint8_t *smeta, uint32_t smeta_len, int8_t *ndim, int64_t *shape,
                                int32_t *chunkshape, int32_t *blockshape) {
    uint8_t *pmeta = smeta;
    CATERVA_UNUSED_PARAM(smeta_len);

    // Check that we have an array with 5 entries (version, ndim, shape, chunkshape, blockshape)
    assert(*pmeta == 0x90 + 5);
    pmeta += 1;
    assert((uint32_t)(pmeta - smeta) < smeta_len);

    // version entry
    int8_t version = pmeta[0];  // positive fixnum (7-bit positive integer)
    CATERVA_UNUSED_PARAM(version);
    assert(version <= CATERVA_METALAYER_VERSION);
    pmeta += 1;
    assert((uint32_t)(pmeta - smeta) < smeta_len);

    // ndim entry
    *ndim = pmeta[0];
    int8_t ndim_aux = *ndim;  // positive fixnum (7-bit positive integer)
    assert(ndim_aux <= CATERVA_MAX_DIM);
    pmeta += 1;
    assert((uint32_t)(pmeta - smeta) < smeta_len);

    // shape entry
    // Initialize to ones, as required by Caterva
    for (int i = 0; i < CATERVA_MAX_DIM; i++) shape[i] = 1;
    assert(*pmeta == (uint8_t)(0x90) + ndim_aux);  // fix array with ndim elements
    pmeta += 1;
    for (int8_t i = 0; i < ndim_aux; i++) {
        assert(*pmeta == 0xd3);  // int64
        pmeta += 1;
        swap_store(shape + i, pmeta, sizeof(int64_t));
        pmeta += sizeof(int64_t);
    }
    assert((uint32_t)(pmeta - smeta) < smeta_len);

    // chunkshape entry
    // Initialize to ones, as required by Caterva
    for (int i = 0; i < CATERVA_MAX_DIM; i++) chunkshape[i] = 1;
    assert(*pmeta == (uint8_t)(0x90) + ndim_aux);  // fix array with ndim elements
    pmeta += 1;
    for (int8_t i = 0; i < ndim_aux; i++) {
        assert(*pmeta == 0xd2);  // int32
        pmeta += 1;
        swap_store(chunkshape + i, pmeta, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    assert((uint32_t)(pmeta - smeta) <= smeta_len);

    // blockshape entry
    // Initialize to ones, as required by Caterva
    for (int i = 0; i < CATERVA_MAX_DIM; i++) blockshape[i] = 1;
    assert(*pmeta == (uint8_t)(0x90) + ndim_aux);  // fix array with ndim elements
    pmeta += 1;
    for (int8_t i = 0; i < ndim_aux; i++) {
        assert(*pmeta == 0xd2);  // int32
        pmeta += 1;
        swap_store(blockshape + i, pmeta, sizeof(int32_t));
        pmeta += sizeof(int32_t);
    }
    assert((uint32_t)(pmeta - smeta) <= smeta_len);
    uint32_t slen = (uint32_t)(pmeta - smeta);
    CATERVA_UNUSED_PARAM(slen);
    assert(slen == smeta_len);
    return 0;
}


#endif //CATERVA_CATERVA_UTILITIES_H
