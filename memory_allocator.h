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

#if 1
#define MEMORY_ALLOCATOR_IMPLEMENTATION
#endif
#ifdef MEMORY_ALLOCATOR_IMPLEMENTATION
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "types.h"

/******************************************************************************
 * TODO:                                                                      *
 ******************************************************************************/

#define ASSERT_WHICH_IS_VALID(_which, _file, _line)                            \
	mem_assertf_ex(_which == LIST_ALLOC || _which == LIST_FREE,            \
		       _file,                                                  \
		       _line,                                                  \
		       "The `which_list_e` parameter can only be "             \
		       "`LIST_ALLOC` or `LIST_FREE` (0 and 1 "                 \
		       "respectively); currently, it's %u\n",                  \
		       _which)

enum {
	FLAGS_NONE	  = 0,
	FLAG_IS_INIT	  = (1 << 0),
	FLAG_HAS_CALLBACK = (1 << 1),
	FLAGS_MASK	  = (FLAG_IS_INIT | FLAG_HAS_CALLBACK)
};
typedef u8 internal_flags_e;

enum { LIST_ALLOC = 0, LIST_FREE };
typedef u8 which_list_e;

struct block {
	const char *file;
	void	   *ptr;
	size_t	    sz;
	u32	    line; /* If your file has anywhere NEAR 4294967295 lines,
			     I think you're already pretty fucked, bucko... */
	u32 _pad;
};

static struct memory {
	struct block *alloc_arr;
	struct block *free_arr;
	size_t	      alloc_cnt;
	size_t	      free_cnt;
} memory;

static internal_flags_e flags = FLAGS_NONE;

/*********************
 * PRIVATE FUNCTIONS *
 *********************/

/*
 * Prints a formatted message to STDOUT only if `ALLOCATOR_LOG` is defined.
 */
#if defined(ALLOCATOR_LOG) ||                                                  \
		(!defined(ALLOCATOR_LOG) && defined(ALLOCATOR_LOG_END_ONLY))
static void
_mem_debugf_internal(const char *fmt, ...)
{
	va_list args;

	assert(fmt);
	va_start(args, fmt);
	(void)fprintf(stdout, "MEM: ");
	(void)vfprintf(stdout, fmt, args);
	va_end(args);
}
#endif /* #if defined(ALLOCATOR_LOG) ||                                        \
	      (!defined(ALLOCATOR_LOG) &&                                      \
	       defined(ALLOCATOR_LOG_END_ONLY)) */

#ifdef ALLOCATOR_LOG
#define mem_debugf(...) _mem_debugf_internal(__VA_ARGS__)
#else /* #ifdef ALLOCATOR_LOG */
#define mem_debugf(...) (void)0
#endif /* #ifdef ALLOCATOR_LOG #else */

/*
 * Stupid fucking hack for making it so that,
 * depending on the compilation flags, it will
 * only print the message at the end instead of
 * logging every single allocation made since
 * the beginning of the program.
 */
#ifdef ALLOCATOR_LOG
#define mem_debugf_end(...)                                                    \
	_mem_debugf_internal(__VA_ARGS__)
#else /* #ifdef ALLOCATOR_LOG */
#ifdef ALLOCATOR_LOG_END_ONLY
#define mem_debugf_end(...)                                                    \
	_mem_debugf_internal(__VA_ARGS__)
#else /* #ifdef ALLOCATOR_LOG_END_ONLY */
#define mem_debugf_end(...) (void)0
#endif /* #ifdef ALLOCATOR_LOG_END_ONLY #else */
#endif /* #ifdef ALLOCATOR_LOG #else */

#define mem_assertf_ex(_cond, _file, _line, _fmt, ...)                         \
	_mem_assertf_internal(!!(_cond),                                       \
			      #_cond,                                          \
			      _file,                                           \
			      _line,                                           \
			      _fmt,                                            \
			      __VA_ARGS__)
#define mem_assertm_ex(_cond, _file, _line, _fmt)                              \
	_mem_assertf_internal(!!(_cond), #_cond, _file, _line, _fmt, NULL)
#define mem_assertm(_cond, _fmt)                                               \
	_mem_assertf_internal(!!(_cond), #_cond, __FILE__, __LINE__, _fmt, NULL)
#define mem_assertf(_cond, _fmt, ...)                                          \
	_mem_assertf_internal(!!(_cond),                                       \
			      #_cond,                                          \
			      __FILE__,                                        \
			      __LINE__,                                        \
			      _fmt,                                            \
			      __VA_ARGS__)

/*
 * Effectively an assert wrapper that allows for a format string.
 */
static void _mem_assertf_internal(const bool_t cond,
				  const char  *cond_str,
				  const char  *file,
				  const int    line,
				  const char  *fmt,
				  ...)
{
#ifdef ALLOCATOR_LOG
	va_list args;

	if (cond)
		return;

	assert(cond_str);
	assert(fmt);
	va_start(args, fmt);
	(void)fprintf(stderr,
		      "MEM: ASSERTION (%s) AT %s:%d FAILED: ",
		      cond_str,
		      file,
		      line);
	(void)vfprintf(stderr, fmt, args);
	(void)fprintf(stderr, "\n");
	va_end(args);
	abort();
#else  /* #ifdef ALLOCATOR_LOG */
	(void)cond;
	(void)cond_str;
	(void)file;
	(void)line;
	(void)fmt;
#endif /* #ifdef ALLOCATOR_LOG #else */
}

/*
 * Verifies that the state of a memory block pointer is valid.
 *
 * NOTE: It does NOT matter whether the pointer is active or not, per say.
 * Rather, depending on if it's `NULL` or not, all of the _rest_ of the data
 * about that block must corroberate that.
 *
 * To give an example:
 *
 * `if (block->ptr == NULL && block->sz > 0)`
 *
 * That would be considered an invalid state, since the pointer is
 * inactive but it still thinks it has a size to it.
 *
 * Whereas:
 *
 * `if (block->ptr != NULL && block->sz == 0)`
 *
 * This would be the opposite case, since it has an
 * active pointer, but no size associated with it.
 */
static void _mem_block_verify(const struct block *b,
			      const which_list_e  which,
			      const char	 *file,
			      const int		  line)
{
	struct block *arr = NULL;
	size_t	      cnt = SIZE_MAX, ind = SIZE_MAX;
	const bool_t  is_alloc = (which == LIST_ALLOC);

	ASSERT_WHICH_IS_VALID(which, file, line);
	arr = is_alloc ? memory.alloc_arr : memory.free_arr;
	cnt = is_alloc ? memory.alloc_cnt : memory.free_cnt;

	mem_assertm_ex(b, file, line, "Block passed in is NULL");
	ind = (size_t)(b - arr);
	mem_assertf_ex(ind < cnt,
		       file,
		       line,
		       "Index %lu is out of range of %s list! (Max is %lu)",
		       ind,
		       is_alloc ? "ALLOC" : "FREE",
		       cnt);

#ifdef ALLOCATOR_LOG_VERBOSE
	mem_debugf("Verifying memory slot %lu\n", ind);
#endif /* #ifdef ALLOCATOR_LOG_VERBOSE */

	/* If our pointer is NULL, the rest of our data better match! */
	if (!b->ptr) {
		mem_assertf_ex(!b->sz,
			       file,
			       line,
			       "Block <%p>'s pointer is NULL, "
			       "but size is greater than 0",
			       b);
		mem_assertf_ex(!b->file,
			       file,
			       line,
			       "Block <%p>'s pointer is NULL, "
			       "but file string is non-NULL",
			       b);
		mem_assertf_ex(b->line == UINT32_MAX,
			       file,
			       line,
			       "Block <%p>'s pointer is NULL, but "
			       "file's line number is not UINT32_MAX (%u)",
			       b,
			       UINT32_MAX);

		return;
	}

	/* Same for if it's non-NULL, just in the other direction. */
	mem_assertf_ex(b->sz,
		       file,
		       line,
		       "Block <%p>'s pointer <%p> "
		       "is non-NULL, but size is 0",
		       b,
		       b->ptr);
	mem_assertf_ex(b->file,
		       file,
		       line,
		       "Block <%p>'s pointer <%p> is "
		       "non-NULL, but file string is NULL",
		       b,
		       b->ptr);
	mem_assertf_ex(b->line != UINT32_MAX,
		       file,
		       line,
		       "Block <%p>'s pointer <%p> is non-NULL, "
		       "but file's line number is UINT32_MAX (%u)",
		       b,
		       b->ptr,
		       UINT32_MAX);
}

/*
 * Safely gets and returns the pointer to a memory block
 * at index `i` from the allocator with bounds checking.
 *
 * It also calls `_mem_block_verify()` in order to ensure the block is valid.
 */
static struct block *_mem_block_get(const size_t       i,
				    const which_list_e which,
				    const char	      *file,
				    const int	       line)
{
	struct block *b = NULL;
	size_t	      cnt;

	ASSERT_WHICH_IS_VALID(which, file, line);
	cnt = (which == LIST_ALLOC) ? memory.alloc_cnt : memory.free_cnt;

	mem_assertf_ex(i < cnt,
		       file,
		       line,
		       "Block index %lu from %s list is invalid! (Max is %lu)",
		       i,
		       (which == LIST_ALLOC) ? "ALLOC" : "FREE",
		       cnt);

#ifdef ALLOCATOR_LOG_VERBOSE
	mem_debugf("Got block at index %lu @ %s:%d\n", i, file, line);
#else  /* #ifdef ALLOCATOR_LOG_VERBOSE */
	(void)file;
	(void)line;
#endif /* #ifdef ALLOCATOR_LOG_VERBOSE #else */

	ASSERT_WHICH_IS_VALID(which, file, line);

	b = ((which == LIST_ALLOC) ? memory.alloc_arr : memory.free_arr) + i;
	_mem_block_verify(b, which, file, line);

	return b;
}

/*
 * Gets a memory block from the allocator's array using- not an index- but
 * the pointer that was allocated TO the block's pointer.
 *
 * This is really only used internally for `mem_free()`, since
 * you pass in the user-allocated pointer, but you also need
 * to clear out the block that pointer is associated with.
 */
static struct block *_mem_get_block_from_user_ptr(const void	    *ptr,
						  const which_list_e which,
						  const char	    *file,
						  const int	     line)
{
	size_t i, cnt;

	ASSERT_WHICH_IS_VALID(which, file, line);

	cnt = (which == LIST_ALLOC) ? memory.alloc_cnt : memory.free_cnt;

	for (i = 0ul; i < cnt; ++i) {
		struct block *s = _mem_block_get(i, which, file, line);

		if (s->ptr == ptr)
			return s;
	}

	return NULL;
}

/*
 * Helper function for zero-initializing a single memory block internally.
 */
static void _mem_block_set_empty(struct block *b)
{
	mem_assertm(memory.alloc_cnt, "Memory count is 0");
	mem_assertm(memory.alloc_arr, "Memory array is NULL");

	mem_assertm(b, "Block pointer is NULL");
	mem_assertf((size_t)(b - memory.alloc_arr) < memory.alloc_cnt,
		    "Block pointer <%p> is out of range of memory "
		    "block array; (is %lu, should be less than %lu)",
		    (size_t)(b - memory.alloc_arr),
		    memory.alloc_cnt);

	b->ptr	= NULL;
	b->sz	= 0ul;
	b->file = NULL;
	b->line = UINT32_MAX;
}

/*
 * Conditionally call `malloc()` or `realloc()` depending on if the array
 * is NULL or not. Very handing for resizing arrays; will probably put
 * in some kind of util file once I get this more properly fleshed out.
 */
static void _array_resize(void	     **arr_ptr,
			  const size_t elem_size,
			  const size_t elem_cnt_old,
			  const size_t elem_cnt_new)
{
	size_t psz, nsz;
	void  *arr = NULL;

	assert(arr_ptr);
	arr = *arr_ptr;

	psz = elem_cnt_old * elem_size;
	nsz = elem_cnt_new * elem_size;

	assert(psz != nsz);

	if (!psz && nsz) { /* Allocate a fresh array */
		assert(!arr);
		arr = malloc(nsz);
	} else if (psz && nsz) { /* Reallocate the existing array */
		assert(arr);
		arr = realloc(arr, nsz);
	} else if (psz && !nsz) { /* Free the now-empty array */
		assert(arr);
		free(arr);
		arr = NULL;
	} else {
		fprintf(stderr, "INVALID ARRAY CONFIGURATION!\n");
		abort();
	}

	*arr_ptr = arr;
}

/*
 * Removes an element `ind` from an array and shifts the
 * entire thing down to accomodate. Calls `_array_resize()`
 * internally and returns the new size of the array.
 */
static size_t _array_remove(void       **arr_ptr,
			    const size_t ind,
			    const size_t elem_size,
			    const size_t elem_cnt)
{
#if 0
	size_t i;
#endif

	/*
	 * TODO:
	 * The plan eventually is that this will be in it's own
	 * rp_array.h module, so it may be worth the investment
	 * to have two different versions of this function.
	 * One that does the original method of bucket-brigading
	 * the data downward if the order is important, and the
	 * one being used specifically for this purpose where
	 * the order of the elements doesn't matter, just as
	 * long as it can resize the array with one less element.
	 */

	/*
	 * Measurements are from 15000 element test, 5 run average.
	 *
	 * Old method: 1.1242 sec
	 * New method: 1.0746 sec
	 *
	 * Result: ~4% performance improvement
	 */

#if 0
	/* Remove that slot from the alloc array and shift it all down */
	for (i = ind; i < elem_cnt; ++i) {
		/* If it's the last element in our array, we're good */
		if (i + 1ul >= elem_cnt)
			break;

		/* Otherwise, just bucket-brigade the shit down */
		memcpy((u8 *)(*arr_ptr) + (i * elem_size),
		       (u8 *)(*arr_ptr) + ((i + 1ul) * elem_size),
		       elem_size);
	}
#else
	/* Copy the last slot to the slot we wanna remove and then realloc. */
	memcpy((u8 *)(*arr_ptr) + (ind * elem_size),
	       (u8 *)(*arr_ptr) + ((elem_cnt * elem_size) - elem_size),
	       elem_size);
#endif

	/* Then resize the array like nothing ever happened. :D */
	_array_resize(arr_ptr, elem_size, elem_cnt, elem_cnt - 1ul);

	return elem_cnt - 1ul;
}

static void
_mem_move_block_to_free_list(struct block *b, const char *file, const int line)
{
	const size_t aind = (size_t)(b - memory.alloc_arr);

	mem_assertm_ex(b,
		       file,
		       line,
		       "Trying to move a NULL pointer to the free list");
	mem_assertf_ex(aind < memory.alloc_cnt,
		       file,
		       line,
		       "Trying to move block <%p> from alloc "
		       "array to free array, but it was never "
		       "in the alloc array to begin with!",
		       b->ptr);

	/* Just fucken' paranoid, honestly... */
	_mem_block_verify(b, LIST_ALLOC, file, line);

	/* Add a new slot to the end of the free list */
	_array_resize((void **)&memory.free_arr,
		      sizeof(*memory.free_arr),
		      memory.free_cnt,
		      ++memory.free_cnt);

	/*
	 * Copy the data over to the newly-created slot,
	 * but change the `file` and `line` variables
	 * to reflect the point at which it was freed.
	 */
	{
		struct block *t = memory.free_arr + (memory.free_cnt - 1);

		*t = *b;

		/* Most importantly, ACTUALLY FREE THE FUCKING POINTER! */
		free(t->ptr);

		t->file = file;
		t->line = (u32)line;
	}

	memory.alloc_cnt = _array_remove((void **)&memory.alloc_arr,
					 aind,
					 sizeof(*memory.alloc_arr),
					 memory.alloc_cnt);
}

/*
 * The function that gets registered by `mem_register_exit_callback()` to be
 * called at the end of the program's execution. If there are any blocks still
 * active by the time the program ends, it will list all of them as well as the
 * files and line in which they were allocated.
 *
 * Either way, it still has to free the allocator's array _storing_ said
 * blocks, similar to what Valgrind does, I think.
 */
static void _mem_atexit(void)
{
	size_t i;

	/* If we never called `malloc()` in our program, we're good to exit. */
	if (!(flags & FLAG_IS_INIT)) {
		assert(!memory.alloc_arr);
		assert(!memory.alloc_cnt);
		mem_debugf_end("No memory was allocated in this "
			       "program; nothing to report.\n");
		goto finish_terminate_no_free;
	}

	if (!memory.alloc_cnt) {
		assert(!memory.alloc_arr);
		mem_debugf_end("\n");
		mem_debugf_end("NO BLOCKS LEFT ACTIVE AT EXIT; GOOD JOB!\n");
		goto finish_terminate_free_free;
	}

	mem_debugf_end("\n");
	mem_debugf_end("WARNING: %d BLOCKS STILL ACTIVE AT EXIT:\n",
		       memory.alloc_cnt);

	i = 0ul;
	while (memory.alloc_cnt) {
		struct block *s = _mem_block_get(0,
						 LIST_ALLOC,
						 __FILE__,
						 __LINE__);
		mem_assertf(s,
			    "Failed to get active memory block in "
			    "terminate function at index %lu\n",
			    i);
		mem_assertf(s->ptr,
			    "Block [p:<%p> i:%lu is in active "
			    "list, but its pointer is NULL.",
			    s,
			    i);

		mem_debugf_end("\tLEAK %lu: [p:<%p> sz:%lu] %s:%u\n",
			       i,
			       s->ptr,
			       s->sz,
			       s->file,
			       s->line);
		_mem_move_block_to_free_list(s, __FILE__, __LINE__);
		++i;
	}

	free(memory.alloc_arr);
	memory.alloc_arr = NULL;
	memory.alloc_cnt = 0ul;

finish_terminate_free_free:
	free(memory.free_arr);
	memory.free_arr = NULL;
	memory.free_cnt = 0ul;

finish_terminate_no_free:
	flags &= ~FLAG_IS_INIT;

	mem_debugf_end("TERMINATED SUCCESSFULLY!\n");
}

/*
 * This simply enables `FLAG_IS_INIT` in `flags`,
 * as that's all it really needs to do for now.
 *
 * This function is only called internally by `mem_alloc()` in order to
 * ensure that it's set up and the rest of the internal's recognize that.
 */
static void _mem_init(void)
{
	assert(!memory.alloc_cnt);
	assert(!memory.alloc_arr);
	assert(!memory.free_cnt);
	assert(!memory.free_arr);
	assert(!(flags & FLAG_IS_INIT)); /* Make sure we didn't already */
	flags |= FLAG_IS_INIT;
	mem_debugf("INITIALIZED SUCCESSFULLY!\n");
}

/********************
 * PUBLIC FUNCTIONS *
 ********************/

/*
 * Internal function for `mem_register_exit_callback()`.
 */
void _mem_register_exit_callback_internal(const char *file, const int line)
{
	mem_assertm(!(flags & FLAG_IS_INIT),
		    "Somehow, first allocation has already "
		    "been made before registering the callback.");
	mem_assertm(!(flags & FLAG_HAS_CALLBACK),
		    "Callback was already registered");

#ifndef ALLOCATOR_LOG
	(void)file;
	(void)line;
#endif /* #ifndef ALLOCATOR_LOG */

	mem_debugf("Registered exit callback at %s:%d\n", file, line);
	atexit(_mem_atexit);

	flags |= FLAG_HAS_CALLBACK;
}

void *_mem_alloc_internal(const size_t sz, const char *file, const int line)
{
	struct block *s = NULL, *t = NULL;
	void	     *p = NULL;

	/* If we didn't register a callback, don't go further. */
	mem_assertm_ex(flags & FLAG_HAS_CALLBACK,
		       file,
		       line,
		       "User hasn't called `mem_register_exit_"
		       "callback()` before calling `mem_alloc()`.");

	/* First call of `mem_alloc()`. */
	if (!(flags & FLAG_IS_INIT))
		_mem_init();

	mem_assertm_ex(sz, file, line, "Trying to allocate 0 bytes!");

	/* Grow the array from the allocation */
	_array_resize((void **)&memory.alloc_arr,
		      sizeof(*memory.alloc_arr),
		      memory.alloc_cnt,
		      ++memory.alloc_cnt);

	/* Get the new empty slot from the end of it */
	s = memory.alloc_arr + memory.alloc_cnt - 1;
	_mem_block_set_empty(s);
	_mem_block_verify(s, LIST_ALLOC, file, line);

	p = malloc(sz);

	/*
	 * When allocating a new pointer, check to see if the
	 * memory address we get is found anywhere in a block
	 * that was already freed and remove it from the free list.
	 *
	 * This is EXTREMELY important, because if we don't do this,
	 * it will fuck up and give a false-positive for a double-free.
	 */
	t = _mem_get_block_from_user_ptr(p, LIST_FREE, file, line);
	if (t) {
		memory.free_cnt = _array_remove((void **)&memory.free_arr,
						(size_t)(t - memory.free_arr),
						sizeof(*memory.alloc_arr),
						memory.free_cnt);
	}

	/* Set it up with the allocation data */
	assert(p);
	s->ptr	= p;
	s->sz	= sz;
	s->file = file;
	s->line = (u32)line;
	_mem_block_verify(s, LIST_ALLOC, file, line);

	mem_debugf("mem_alloc(%lu) -> <%p> %s:%d\n", sz, p, file, line);

	/* Send the user pointer on it's way to the caller. */
	return p;
}

void _mem_free_internal(void *ptr, const char *file, const int line)
{
	struct block *s;

	mem_assertm_ex(flags & FLAG_IS_INIT,
		       file,
		       line,
		       "No allocation was previously made, "
		       "so there's nothing to free.");
	mem_assertm_ex(ptr, file, line, "Trying to free a NULL pointer");

	/*
	 * Make sure that it's not in the free list.
	 * If so, that would be considered a double-free!
	 */
	s = _mem_get_block_from_user_ptr(ptr, LIST_FREE, file, line);
	if (s) {
		mem_debugf("Trying to double-free pointer "
			   "<%p>\n\tOriginally freed at %s:%d\n",
			   s->ptr,
			   s->file,
			   s->line);
		abort();
	}

	s = _mem_get_block_from_user_ptr(ptr, LIST_ALLOC, file, line);
	mem_assertf(s,
		    file,
		    line,
		    "Failed to get allocated block from user pointer <%p>",
		    ptr);
	mem_debugf("mem_free(<%p>) [sz:%lu] %s:%d\n",
		   s->ptr,
		   s->sz,
		   file,
		   line);
	_mem_move_block_to_free_list(s, file, line);
}
#endif /* #ifdef MEMORY_ALLOCATOR_IMPLEMENTATION */

/*
 * Macro defines for wrapping the calls to `_mem_*_internal()` by giving
 * them prettier names and passing in the __FILE__ and __LINE__ they were
 * called from for glorious, glorious debugging purposes!
 */
#define mem_register_exit_callback()                                           \
	_mem_register_exit_callback_internal(__FILE__, __LINE__)
#define mem_alloc(_sz) _mem_alloc_internal(_sz, __FILE__, __LINE__)
#define mem_free(_ptr) _mem_free_internal(_ptr, __FILE__, __LINE__)

#ifdef MEMORY_ALLOCATOR_WRAP_STDLIB
#define malloc(_sz) mem_alloc(_sz)
#define free(_sz)   mem_free(_sz)
#endif /* #ifdef MEMORY_ALLOCATOR_WRAP_STDLIB */

#endif /* #ifndef _MEMORY_ALLOCATOR_H_ */
