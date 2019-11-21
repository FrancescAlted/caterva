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


#ifndef CATERVA_CATERVA_BLOSC_H
#define CATERVA_CATERVA_BLOSC_H

caterva_array_t *caterva_blosc_from_frame(caterva_ctx_t *ctx, blosc2_frame *frame, bool copy);

caterva_array_t *caterva_blosc_from_sframe(caterva_ctx_t *ctx, uint8_t *sframe, int64_t len, bool copy);

caterva_array_t *caterva_blosc_from_file(caterva_ctx_t *ctx, const char *filename, bool copy);

int caterva_blosc_free_array(caterva_array_t *carr);

int caterva_blosc_append(caterva_array_t *carr, void *part, int64_t partsize);

#endif //CATERVA_CATERVA_BLOSC_H
