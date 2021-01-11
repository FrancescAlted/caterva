/*
 * Copyright (c) 2018 Francesc Alted, Aleix Alcacer.
 * Copyright (C) 2019-present Blosc Development team <blosc@blosc.org>
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */

#include "test_common.h"

caterva_context_t *ctx;

int roundtrip() {

    CUTEST_GET_PARAMETER(backend, _test_backend);
    CUTEST_GET_PARAMETER(shapes, _test_shapes);
    CUTEST_GET_PARAMETER(itemsize, bool);

    caterva_params_t params;
    params.itemsize = itemsize;
    params.ndim = shapes.ndim;
    for (int i = 0; i < shapes.ndim; ++i) {
        params.shape[i] = shapes.shape[i];
    }

    caterva_storage_t storage = {0};
    storage.backend = backend.backend;
    switch (backend.backend) {
        case CATERVA_STORAGE_PLAINBUFFER:
            break;
        case CATERVA_STORAGE_BLOSC:
            if (backend.persistent) {
                storage.properties.blosc.filename = "test_roundtrip.b2frame";
            }
            storage.properties.blosc.enforceframe = backend.sequential;
            for (int i = 0; i < shapes.ndim; ++i) {
                storage.properties.blosc.chunkshape[i] = shapes.chunkshape[i];
                storage.properties.blosc.blockshape[i] = shapes.blockshape[i];
            }
            break;
        default:
            CUTEST_ASSERT_CATERVA(CATERVA_ERR_INVALID_STORAGE);
    }

    /* Create original data */
    size_t buffersize = (size_t) itemsize;
    for (int i = 0; i < shapes.ndim; ++i) {
        buffersize *= (size_t) shapes.shape[i];
    }
    uint8_t *buffer = malloc(buffersize);
    CUTEST_ASSERT(fill_buf(buffer, itemsize, buffersize / itemsize),
                  "Buffer filled incorrectly");

    /* Create caterva_array_t with original data */
    caterva_array_t *src;
    CUTEST_ASSERT_CATERVA(caterva_array_from_buffer(ctx, buffer, buffersize, &params, &storage,
                                                  &src));

    /* Fill dest array with caterva_array_t data */
    uint8_t *buffer_dest = malloc( buffersize);
    CUTEST_ASSERT_CATERVA(caterva_array_to_buffer(ctx, src, buffer_dest, buffersize));

    /* Testing */

    //TODO: MU_ASSERT_BUFFER(buffer, buffer_dest, buffersize);

    /* Free mallocs */
    free(buffer);
    free(buffer_dest);
    CUTEST_ASSERT_CATERVA(caterva_array_free(ctx, &src));
    return 0;
}


int main() {
    caterva_config_t cfg = CATERVA_CONFIG_DEFAULTS;
    cfg.nthreads = 2;
    cfg.compcodec = BLOSC_BLOSCLZ;
    caterva_context_new(&cfg, &ctx);

    caterva_default_parameters();

    CUTEST_RUN(roundtrip);

    caterva_context_free(&ctx);
    return 0;
}