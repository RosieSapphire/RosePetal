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

/*
 * Registeres the internal exit callback for the memory allocator
 * which makes sure to clean up and memory left behind and notify
 * you about any pointers you forgot to free (ya greasy bastard. lol).
 */
extern void _mem_register_exit_callback_internal(const char *file,
						 const int   line);

/*
 * Allocates a user pointer of a specified `sz` and returns it.
 *
 * This function allocates internal memory for storing
 * the blocks, specifically for debugging purposes.
 *
 * If `ALLOCATOR_WRAP_STDLIB` is defined, this function will be called
 * by any instances of `malloc()` where this file is included in.
 */
extern void *
_mem_alloc_internal(const size_t sz, const char *file, const int line);

/*
 * Frees a user pointer that was previously allocated.
 *
 * This function also nullifies the internal memory block
 * the pointer is associated with, _HOWEVER_, it does not
 * free the block itself, as it can be used later to re-allocate
 * another user pointer to.
 *
 * If `ALLOCATOR_WRAP_STDLIB` is defined, this function will be called
 * by any instances of `free()` where this file is included in.
 */
extern void _mem_free_internal(void *ptr, const char *file, const int line);

/*
 * Macro defines for wrapping the calls to `_mem_*_internal()` by giving
 * them prettier names and passing in the __FILE__ and __LINE__ they were
 * called from for glorious, glorious debugging purposes!
 */
#define mem_register_exit_callback()                                           \
	_mem_register_exit_callback_internal(__FILE__, __LINE__)
#define mem_alloc(_sz)	_mem_alloc_internal(_sz, __FILE__, __LINE__)
#define mem_free(_ptr)	_mem_free_internal(_ptr, __FILE__, __LINE__)

#ifdef MEMORY_ALLOCATOR_WRAP_STDLIB
#define malloc(_sz) _mem_alloc_internal(_sz, __FILE__, __LINE__)
#define free(_sz)   _mem_free_internal(_sz, __FILE__, __LINE__)
#endif /* #ifdef MEMORY_ALLOCATOR_WRAP_STDLIB */

#endif /* #ifndef _MEMORY_ALLOCATOR_H_ */
