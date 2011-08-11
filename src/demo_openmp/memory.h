
#ifndef MEMORY_H
#define MEMORY_H

#include "all.h"

void *** alloc_3D(unsigned height, unsigned width, unsigned dim, unsigned elem_size);
void ** alloc_2D(unsigned height, unsigned dim, unsigned elem_size);
void * alloc_1D(unsigned dim, unsigned elem_size);
void ** realloc_2D(void ** mem, unsigned old_height, unsigned height, unsigned dim, unsigned elem_size);
void * realloc_1D(void * mem, unsigned old_dim, unsigned dim, unsigned elem_size);
void free_ND(void * mem);

#endif

