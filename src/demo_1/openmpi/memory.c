
#include "memory.h"

void *** alloc_3D(unsigned height, unsigned width, unsigned dim, unsigned elem_size)
{
	unsigned u, v, height_width = height * width, dim_elem_size = dim * elem_size;
	void *** ret = malloc(height * sizeof(void **) + height_width * (sizeof(void *) + dim_elem_size));
	
	if(ret != NULL)
	{
		void *** ppp = ret;
		void ** pp = (void **) (ppp + height);
		void * p = (void *) (pp + height_width);

		for(u = 0; u < height; u++, ppp++)
		{
			(* ppp) = pp;

			for(v = 0; v < width; v++, pp++, p += dim_elem_size)
				(* pp) = p;
		}
	}

	return ret;
}

void ** alloc_2D(unsigned height, unsigned dim, unsigned elem_size)
{
	unsigned u, dim_elem_size = dim * elem_size;
	void ** ret = malloc(height * (sizeof(void *) + dim_elem_size));
	
	if(ret != NULL)
	{
		void ** pp = ret;
		void * p = (void *) (pp + height);

		for(u = 0; u < height; u++, pp++, p += dim_elem_size)
			(* pp) = p;
	}

	return ret;
}

void * alloc_1D(unsigned dim, unsigned elem_size)
{
	void * ret = malloc(dim * elem_size);
	
	return ret;
}

void ** realloc_2D(void ** mem, unsigned old_height, unsigned height, unsigned dim, unsigned elem_size)
{
	void ** ret;

	if(
		old_height > height
		||
		(ret = alloc_2D(height, dim, elem_size)) == NULL
	  )
		return NULL;

	memcpy((void *) (ret + height), (void *) (mem + old_height), old_height * dim * elem_size);
	free_ND(mem);

	return ret;
}

void * realloc_1D(void * mem, unsigned old_dim, unsigned dim, unsigned elem_size)
{
	void * ret;

	if(
		old_dim > dim
		||
		(ret = alloc_1D(dim, elem_size)) == NULL
	  )
		return NULL;

	memcpy(ret, mem, old_dim * elem_size);
	free_ND(mem);

	return ret;
}

void free_ND(void * mem)
{
	if(mem != NULL)
		free(mem);
}

