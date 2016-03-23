/*
	Here is an interface of the memory allocation to hide the platform-dependent libraries. 
*/

#ifndef __MEM_MEM_ALLOC_H__
#define __MEM_MEM_ALLOC_H__

#include <stddef.h>

// API
extern void* mem_alloc (size_t size);
extern void* mem_realloc (void* ptr, size_t size);
extern int mem_free (void* ptr);

#endif
