/*********************************************************************
  Blosc - Blocked Shuffling and Compression Library

  Author: Francesc Alted <francesc@blosc.org>

  See LICENSE.txt for details about copyright and rights to use.
**********************************************************************/



#ifndef NDLZ_H
#define NDLZ_H
#include "context.h"

#if defined (__cplusplus)
extern "C" {
#endif
#define XXH_INLINE_ALL
#include <xxhash.h>
/*
#include <stdio.h>
#include "blosc2-common.h"
#include "fastcopy.h"
*/


#define NDLZ_VERSION_STRING "1.0.0"

typedef struct {
    int ndim;
    int32_t* blockshape;
} ndlz_params;

#if defined (__cplusplus)
}
#endif

#endif /* NDLZ_H */
