#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <blosc.h>

#define MAXDIM 8

typedef struct
{
    size_t shape[MAXDIM]; /* the shape of original data */
    size_t cshape[MAXDIM]; /* the shape of each chunk */
    size_t dimensions; /* data dimensions */
} caterva_pparams;

typedef struct
{
    blosc2_schunk* sc;
    size_t shape[MAXDIM]; /* shape of original data */
    size_t cshape[MAXDIM]; /* shape of each chunk */
    size_t eshape[MAXDIM]; /* shape of schunk */
    size_t size; /* size of original data */
    size_t csize; /* size of each chunnk */
    size_t esize; /* shape of schunk */
    size_t dimensions; /* data dimensions */
} caterva_array;

void schunk_fill_from_array(void *arr, caterva_array *carr);

void array_fill_from_schunk(caterva_array *carr, void *arr);

#endif
