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

#ifndef CATERVA_CATERVA_UTILS_H_
#define CATERVA_CATERVA_UTILS_H_

#include <caterva.h>

void index_unidim_to_multidim(int8_t ndim, int64_t *shape, int64_t i, int64_t *index);
void index_multidim_to_unidim(int64_t *index, int8_t ndim, int64_t *strides, int64_t *i);

int32_t serialize_meta(uint8_t ndim, int64_t *shape, const int32_t *chunkshape,
                       const int32_t *blockshape, uint8_t **smeta);
int32_t deserialize_meta(uint8_t *smeta, uint32_t smeta_len, uint8_t *ndim, int64_t *shape,
                         int32_t *chunkshape, int32_t *blockshape);

void swap_store(void *dest, const void *pa, int size);

int caterva_copy_buffer(uint8_t ndim,
                        uint8_t itemsize,
                        void *src, int64_t *src_pad_shape,
                        int64_t *src_start, int64_t *src_stop,
                        void *dst, int64_t *dst_pad_shape,
                        int64_t *dst_start);

#endif  // CATERVA_CATERVA_UTILS_H_
