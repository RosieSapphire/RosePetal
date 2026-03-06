#ifndef _MEMORY_ALLOCATOR_H_
#define _MEMORY_ALLOCATOR_H_

#include <stddef.h>

extern void _mem_init_internal(const char *file, const int line);
extern void *
_mem_alloc_internal(const size_t sz, const char *file, const int line);
extern void _mem_free_internal(void *ptr, const char *file, const int line);
extern void _mem_terminate_internal(const char *file, const int line);

#define mem_init()	_mem_init_internal(__FILE__, __LINE__)
#define mem_alloc(_sz)	_mem_alloc_internal(_sz, __FILE__, __LINE__)
#define mem_free(_ptr)	_mem_free_internal(_ptr, __FILE__, __LINE__)
#define mem_terminate() _mem_terminate_internal(__FILE__, __LINE__)

#endif /* #ifndef _MEMORY_ALLOCATOR_H_ */
