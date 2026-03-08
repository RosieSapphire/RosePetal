#ifndef _MEMORY_ALLOCATOR_H_
#define _MEMORY_ALLOCATOR_H_

/*
 * Just a custom memory allocation wrapper I wrote for the sake of debugging
 * memory leaks and where/when/how much memory was allocated and what- if
 * anything- went wrong.
 *
 * Make sure to define `MEMORY_ALLOCATOR_WRAP_STDLIB` in order to wrap
 * calls to `malloc()` and `free()` with `mem_alloc()` and `mem_free()`.
 * Disablind this macro will just use them like you normally would,
 * effictively disabling this library- unless the functions are called
 * directly- freeing up performance for release mode.
 */

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

#ifdef MEMORY_ALLOCATOR_WRAP_STDLIB
#define malloc(_sz) _mem_alloc_internal(_sz, __FILE__, __LINE__)
#define free(_sz)   _mem_free_internal(_sz, __FILE__, __LINE__)
#endif /* #ifdef MEMORY_ALLOCATOR_WRAP_STDLIB */

#endif /* #ifndef _MEMORY_ALLOCATOR_H_ */
