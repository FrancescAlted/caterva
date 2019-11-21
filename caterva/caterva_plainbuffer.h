/*
 * Copyright (C) 2018  Francesc Alted
 * Copyright (C) 2018  Aleix Alcacer
 */

#ifndef CATERVA_CATERVA_PLAINBUFFER_H
#define CATERVA_CATERVA_PLAINBUFFER_H

int caterva_plainbuffer_free_array(caterva_array_t *carr);

int caterva_plainbuffer_append(caterva_array_t *carr, void *part, int64_t partsize);

#endif //CATERVA_CATERVA_PLAINBUFFER_H
