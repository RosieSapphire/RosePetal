/*
 * The memory allocator/debugger module for Rose Petal.
 * It keeps track of how much memory you currently have allocated,
 * how much you've freed, what blocks were left over in memory
 * upon exiting the program, among other useful stuff!
 *
 * Below is a list of the macros required to make this file work...
 *
 * NOTE: There is some parity between the way these macros
 *	 interact with each other, but it will error out if you
 * 	 try to do anything *particularly* catastrophic. lol
 *
 * RP_MEMORY_IMPLEMENTATION:
 *	The cornerstone of any header-only library is the ole' IMPLEMENTATION
 *	macro that has to be defined before even including it. If you don't
 *	do this, none of the functions will be made visibile.
 *
 *	IMPORTANT: Make sure you only ever define this macro in
 *	           ONE C file that gets linked with any given
 *	           program, otherwise the linker will bitch at
 *	           you for a multiple definition thingy-majiggy.
 *
 *	If you wanna include this file in any other translation units, you
 *	can do that just fine, just make sure not to re-define this macro.
 *
 *	NOTE: All the below macros assume this one is defined.
 *
 * RP_MEMORY_WRAP_STDLIB:
 *	Probably the most important macro if you
 *	plan on using this for a serious project.
 *
 *	Essentially, by default, in order to call the allocation and freeing
 *	functions for this module, you'd have to type `rp_mem_alloc()` and
 *	`rp_mem_free()` respectively. However, if you want to effectively
 *	"toggle" this module, all you have to do is define this before
 *	including and you'll
 *
 * RP_MEMORY_LOG:
 *	Use `stdout` to print information about the debugger as it goes.
 *	This will give all your basic stuff, like when you allocate a new
 *	block, when you free one, if you fucked up and accidentally freed
 *	a block twice, and potentially even if you accessed it out of range.
 *
 * RP_MEMORY_LOG_VERBOSE:
 *	I could bullshit you and say this is for more "advanced" debugging,
 *	but the be honest, I put it in here as a way to spit out as much
 *	terminal output about whatever problem I'm having in the hopes of
 *	figuring it out.
 *
 *	Honestly, it's shit, don't bother with it.
 *	You're better with a proper debugger. I haven't
 *	bothered to remove it because I'm fucking lazy.
 *	Sue me.
 *
 *	NOTE: This macro can ONLY be defined if `RP_MEMORY_LOG` is.
 *
 * RP_MEMORY_LOG_END_ONLY:
 *	This is for if you ONLY wanna know what's happening with the memory
 *	after the program is over. Similar to valgrind or address-sanitizer,
 *	it just prints out how many blocks were still in use at the end of
 *	the program and nothing more; skips all the intermediate printing.
 *
 *	This is what you'll wanna use 90% of the time, unless
 *	you need a proper roadmap of every allocation happening
 *	in the program, then just use `RP_MEMORY_LOG`.
 *
 *	NOTE: This macro can ONLY be defined if `RP_MEMORY_LOG`
 *	      and `RP_MEMORY_LOG_VERBOSE` are NOT defined.
 *
 * RP_MEMORY_TEST:
 *	This defines a function called `rp_memory_test()` which you can call
 *	anywhere in your program to run the test code for this module. This
 *	is really only ever used for internal use, specifically for `test.c`
 *	in the repository, but here's a macro to define it so you can have
 *	it anywhere in your program. Have a ball.
 *
 *	NOTE: This macro can ONLY be defined if `RP_MEMORY_TEST_ALLOC_CNT`
 *	      is also defined. That's a necessary part of the test.
 *	      More information about that one below.
 *
 * RP_MEMORY_TEST_ALLOC_CNT:
 *	This define is required for `RP_MEMORY_TEST` to work.
 *	This MUST be defined with an integer value (preferably
 *	suffixed with `ul`) or it won't be able to compile.
 *
 * RP_MEMORY_TEST_SIMULATE_LEAK:
 *	An optional define for `RP_MEMORY_TEST` that makes it
 *	so it leaves a few blocks left in memory after the
 *	test to see how it reacts to having memory left over.
 *
 *	You could also just be a lazy fuck and not
 *	free your memory, but the choice is yours.
 *
 * Oh yeah, and this is all provided, like, "as is" or whatever.
 * I dunno, I was never really good with licenses. ¯\_(ツ)_/¯
 */

#ifndef _RP_MEMORY_H_
#define _RP_MEMORY_H_

/*
 * Before doing anything, we must handle macro parity!
 */
#if defined(RP_MEMORY_TEST) && !defined(RP_RANDOM_IMPLEMENTATION)
#error "The `rp_memory_test()` function requires that `RP_RANDOM_IMPLEMENTATION` is defined, otherwise it can't use the random functions it requires."
#endif /* #if defined(RP_MEMORY_TEST) && !defined(RP_RANDOM_IMPLEMENTATION) */

#ifdef RP_MEMORY_LOG
#ifdef RP_MEMORY_LOG_END_ONLY
#error "You cannot define `RP_MEMORY_LOG_END_ONLY` if `RP_MEMORY_LOG` is already defined; pick one or the other."
#endif /* #ifdef RP_MEMORY_LOG_END_ONLY */
#else  /* #ifndef RP_MEMORY_LOG */
#ifdef RP_MEMORY_LOG_VERBOSE
#error "You cannot defined `RP_MEMORY_LOG_VERBOSE` if `RP_MEMORY_LOG` isn't defined first."
#endif /* #ifdef RP_MEMORY_LOG_VERBOSE */
#endif /* #ifndef RP_MEMORY_LOG #else */

#if defined(RP_MEMORY_LOG_VERBOSE) && defined(RP_MEMORY_LOG_END_ONLY)
#error "There's no valid reason to defined `RP_MEMORY_LOG_VERBOSE` when `RP_MEMORY_LOG_END_ONLY` is defined."
#endif /* #if defined(RP_MEMORY_LOG_VERBOSE) &&                                \
	  defined(RP_MEMORY_LOG_END_ONLY) */

#ifdef RP_MEMORY_TEST
#ifndef RP_MEMORY_TEST_ALLOC_CNT
#error "In order for `RP_MEMORY_TEST` to work, `RP_MEMORY_TEST_ALLOC_CNT` must be defined with an integer value."
#else /* #ifndef RP_MEMORY_TEST_ALLOC_CNT */
#if !RP_MEMORY_TEST_ALLOC_CNT
#error "`RP_MEMORY_TEST_ALLOC_CNT` must be greater than 0."
#endif /* #if !RP_MEMORY_TEST_ALLOC_CNT */
#endif /* #ifndef RP_MEMORY_TEST_ALLOC_CNT #else */
#ifndef RP_MEMORY_IMPLEMENTATION
#error "In order for `RP_MEMORY_TEST` to work, `RP_MEMORY_IMPLEMENTATION` must be defined, otherwise the function won't exist."
#endif /* #ifndef RP_MEMORY_IMPLEMENTATION */
#else  /* #ifdef RP_MEMORY_TEST */
#ifdef RP_MEMORY_TEST_SIMULATE_LEAK
#error "`RP_MEMORY_TEST_SIMULATE_LEAK` has no reason to be defined if `RP_MEMORY_TEST` isn't."
#endif /* #ifdef RP_MEMORY_TEST_SIMULATE_LEAK */
#ifdef RP_MEMORY_TEST_ALLOC_CNT
#error "`RP_MEMORY_TEST_ALLOC_CNT` has no reason to be defined if `RP_MEMORY_TEST` isn't."
#endif /* #ifdef RP_MEMORY_TEST_ALLOC_CNT */
#endif /* #ifdef RP_MEMORY_TEST #else */

/*
 * God fucking DAMN that was ugly!
 * Alright then, moving on.
 */

#include <stddef.h>

/********************************
 * PUBLIC FUNCTION DECLARATIONS *
 ********************************/

/*
 * Registeres the internal exit callback for the memory allocator
 * which makes sure to clean up and memory left behind and notify
 * you about any pointers you forgot to free (ya greasy bastard).
 */
extern void _rp_mem_exit_callback_register_internal(const char *file,
						    const int	line);

/*
 * Allocates a user pointer of a specified `sz` and returns it.
 *
 * This function allocates internal memory for storing
 * the blocks, specifically for debugging purposes.
 *
 * If `RP_MEMORY_WRAP_STDLIB` is defined, this function will be called
 * by any instances of `malloc()` where this file is included in.
 */
extern void *
_rp_mem_alloc_internal(const size_t sz, const char *file, const int line);

/*
 * Frees a user pointer that was previously allocated.
 *
 * This function also nullifies the internal memory block the
 * pointer is associated with, _HOWEVER_, it does not free the
 * block itself, as it can be used later to re-allocate another
 * user pointer to. It just moves it over to the "free list"
 *
 * If `RP_MEMORY_WRAP_STDLIB` is defined, this function will be called
 * by any instances of `free()` where this file is included in.
 */
extern void _rp_mem_free_internal(void *ptr, const char *file, const int line);

/******************************
 * IMPLEMENTATION DEFINITIONS *
 ******************************/

#ifdef RP_MEMORY_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "rp_types.h"

#define _ASSERT_WHICH_IS_VALID(_which, _file, _line)                           \
	rp_assertf_ex(_which == LIST_ALLOC || _which == LIST_FREE,             \
		      _file,                                                   \
		      _line,                                                   \
		      "The `which_list_e` parameter can only be "              \
		      "`LIST_ALLOC` or `LIST_FREE` (0 and 1 "                  \
		      "respectively); currently, it's %u\n",                   \
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
 * Prints a formatted message to STDOUT only if `RP_MEMORY_LOG` is defined.
 */
#if defined(RP_MEMORY_LOG) ||                                                  \
		(!defined(RP_MEMORY_LOG) && defined(RP_MEMORY_LOG_END_ONLY))
static void _mem_debugf_internal(const char *fmt, ...)
{
	va_list args;

	if (!fmt)
		abort();
	va_start(args, fmt);
	(void)fprintf(stdout, "MEM: ");
	(void)vfprintf(stdout, fmt, args);
	va_end(args);
}
#endif /* #if defined(RP_MEMORY_LOG) ||                                        \
	      (!defined(RP_MEMORY_LOG) &&                                      \
	       defined(RP_MEMORY_LOG_END_ONLY)) */

#ifdef RP_MEMORY_LOG
#define mem_debugf(...) _mem_debugf_internal(__VA_ARGS__)
#else /* #ifdef RP_MEMORY_LOG */
#define mem_debugf(...) (void)0
#endif /* #ifdef RP_MEMORY_LOG #else */

/*
 * Stupid fucking hack for making it so that,
 * depending on the compilation flags, it will
 * only print the message at the end instead of
 * logging every single allocation made since
 * the beginning of the program.
 */
#ifdef RP_MEMORY_LOG
#define mem_debugf_end(...) _mem_debugf_internal(__VA_ARGS__)
#else /* #ifdef RP_MEMORY_LOG */
#ifdef RP_MEMORY_LOG_END_ONLY
#define mem_debugf_end(...) _mem_debugf_internal(__VA_ARGS__)
#else /* #ifdef RP_MEMORY_LOG_END_ONLY */
#define mem_debugf_end(...) (void)0
#endif /* #ifdef RP_MEMORY_LOG_END_ONLY #else */
#endif /* #ifdef RP_MEMORY_LOG #else */

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

	_ASSERT_WHICH_IS_VALID(which, file, line);
	arr = is_alloc ? memory.alloc_arr : memory.free_arr;
	cnt = is_alloc ? memory.alloc_cnt : memory.free_cnt;

	rp_assertf_ex(b, file, line, "Block passed in is NULL");
	ind = (size_t)(b - arr);
	rp_assertf_ex(ind < cnt,
		      file,
		      line,
		      "Index %zu is out of range of %s list! (Max is %zu)",
		      ind,
		      is_alloc ? "ALLOC" : "FREE",
		      cnt);

#ifdef RP_MEMORY_LOG_VERBOSE
	mem_debugf("Verifying memory slot %zu\n", ind);
#endif /* #ifdef RP_MEMORY_LOG_VERBOSE */

	/* If our pointer is NULL, the rest of our data better match! */
	if (!b->ptr) {
		rp_assertf_ex(!b->sz,
			      file,
			      line,
			      "Block <%p>'s pointer is NULL, "
			      "but size is greater than 0",
			      b);
		rp_assertf_ex(!b->file,
			      file,
			      line,
			      "Block <%p>'s pointer is NULL, "
			      "but file string is non-NULL",
			      b);
		rp_assertf_ex(b->line == UINT32_MAX,
			      file,
			      line,
			      "Block <%p>'s pointer is NULL, but "
			      "file's line number is not UINT32_MAX (%u)",
			      b,
			      UINT32_MAX);

		return;
	}

	/* Same for if it's non-NULL, just in the other direction. */
	rp_assertf_ex(b->sz,
		      file,
		      line,
		      "Block <%p>'s pointer <%p> "
		      "is non-NULL, but size is 0",
		      b,
		      b->ptr);
	rp_assertf_ex(b->file,
		      file,
		      line,
		      "Block <%p>'s pointer <%p> is "
		      "non-NULL, but file string is NULL",
		      b,
		      b->ptr);
	rp_assertf_ex(b->line != UINT32_MAX,
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

	_ASSERT_WHICH_IS_VALID(which, file, line);
	cnt = (which == LIST_ALLOC) ? memory.alloc_cnt : memory.free_cnt;

	rp_assertf_ex(i < cnt,
		      file,
		      line,
		      "Block index %zu from %s list is invalid! (Max is %zu)",
		      i,
		      (which == LIST_ALLOC) ? "ALLOC" : "FREE",
		      cnt);

#ifdef RP_MEMORY_LOG_VERBOSE
	mem_debugf("Got block at index %zu @ %s:%d\n", i, file, line);
#else  /* #ifdef RP_MEMORY_LOG_VERBOSE */
	(void)file;
	(void)line;
#endif /* #ifdef RP_MEMORY_LOG_VERBOSE #else */

	_ASSERT_WHICH_IS_VALID(which, file, line);

	b = ((which == LIST_ALLOC) ? memory.alloc_arr : memory.free_arr) + i;
	_mem_block_verify(b, which, file, line);

	return b;
}

/*
 * Gets a memory block from the allocator's array using- not an index- but
 * the pointer that was allocated TO the block's pointer.
 *
 * This is really only used internally for `rp_mem_free()`, since
 * you pass in the user-allocated pointer, but you also need
 * to clear out the block that pointer is associated with.
 */
static struct block *_mem_get_block_from_user_ptr(const void	    *ptr,
						  const which_list_e which,
						  const char	    *file,
						  const int	     line)
{
	size_t i, cnt;

	_ASSERT_WHICH_IS_VALID(which, file, line);

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
static void _mem_block_set_empty(struct block *b, const which_list_e w)
{
	const char   *wstr;
	struct block *arr;
	size_t	      cnt;
	const bool_t  is_alloc = (w == LIST_ALLOC);

	_ASSERT_WHICH_IS_VALID(w, __FILE__, __LINE__);

	arr  = is_alloc ? memory.alloc_arr : memory.free_arr;
	cnt  = is_alloc ? memory.alloc_cnt : memory.free_cnt;
	wstr = is_alloc ? "ALLOC" : "FREE";

	rp_assertf(cnt, "Memory %s count is 0", wstr);
	rp_assertf(arr, "Memory %s array is NULL", wstr);

	rp_assertf(b, "Block pointer in %s list is NULL", wstr);
	rp_assertf((size_t)(b - arr) < cnt,
		   "Block pointer <%p> is out of range of memory block "
		   "array for %s; (is %zu, should be less than %zu)",
		   b,
		   wstr,
		   (size_t)(b - arr),
		   cnt);

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

	rp_assertf(arr_ptr, "Array pointer is NULL");
	arr = *arr_ptr;

	rp_assertf(elem_size, "Array element size cannot be 0");

	psz = elem_cnt_old * elem_size;
	nsz = elem_cnt_new * elem_size;

	rp_assertf(psz != nsz, "Array's previous and next size are identical");

	if (!psz && nsz) { /* Allocate a fresh array */
		rp_assertf(!arr,
			   "Trying to allocate a new array, "
			   "but one already exists");
		arr = malloc(nsz);
		rp_assertf(arr, "Failed to allocate new array");
	} else if (psz && nsz) { /* Reallocate the existing array */
		rp_assertf(arr,
			   "Trying to reallocate an existing array, "
			   "but the original pointer is NULL");
		arr = realloc(arr, nsz);
		rp_assertf(arr, "Failed to reallocate array.");
	} else if (psz && !nsz) { /* Free the now-empty array */
		rp_assertf(arr, "Trying to free array that never existed");
		free(arr);
		arr = NULL;
		rp_assertf(!arr, "Dude, what the ACTUAL FUCK did you do?!");
	} else {
		rp_assertf(0, "INVALID ARRAY CONFIGURATION!");
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
	 * NOTE: this was measured in debug mode with assertions turned on.
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

	rp_assertf_ex(b,
		      file,
		      line,
		      "Trying to move a NULL pointer to the free list");
	rp_assertf_ex(aind < memory.alloc_cnt,
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
		rp_assertf(!memory.alloc_arr,
			   "Init flag is off, but "
			   "array somehow exists");
		rp_assertf(!memory.alloc_cnt,
			   "Init flag is off, but "
			   "array somehow has size");
		mem_debugf_end("No memory was allocated in this "
			       "program; nothing to report.\n");
		goto finish_terminate_no_free;
	}

	if (!memory.alloc_cnt) {
		rp_assertf(!memory.alloc_arr,
			   "Array size is 0, but "
			   "array pointer is non-NULL");
		mem_debugf_end("\n");
		mem_debugf_end("NO BLOCKS LEFT ACTIVE AT EXIT; GOOD JOB!\n");
		goto finish_terminate_free_free;
	}

	mem_debugf_end("\n");
	mem_debugf_end("WARNING: %zu BLOCKS STILL ACTIVE AT EXIT:\n",
		       memory.alloc_cnt);

	i = 0ul;
	while (memory.alloc_cnt) {
		struct block *s = _mem_block_get(0,
						 LIST_ALLOC,
						 __FILE__,
						 __LINE__);
		rp_assertf(s,
			   "Failed to get active memory block in "
			   "terminate function at index %zu\n",
			   i);
		rp_assertf(s->ptr,
			   "Block [p:<%p> i:%zu is in active "
			   "list, but its pointer is NULL.",
			   s,
			   i);

		mem_debugf_end("\tLEAK %zu: [p:<%p> sz:%zu] %s:%u\n",
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
 * This function is only called internally by `rp_mem_alloc()` in order to
 * ensure that it's set up and the rest of the internal's recognize that.
 */
static void _mem_init(void)
{
	rp_assertf(!(flags & FLAG_IS_INIT),
		   "Somehow `_mem_init()` was already called");
	rp_assertf(!memory.alloc_cnt,
		   "Trying to init, but array already has size");
	rp_assertf(!memory.alloc_arr,
		   "Trying to init, but array was already allocated");
	rp_assertf(!memory.free_cnt,
		   "Trying to init, but free list already has size");
	rp_assertf(!memory.free_arr,
		   "Trying to init, but free list was already allocated");
	flags |= FLAG_IS_INIT;
	mem_debugf("INITIALIZED SUCCESSFULLY!\n");
}

/********************
 * PUBLIC FUNCTIONS *
 ********************/

/*
 * Internal function for `mem_register_exit_callback()`.
 */
void _rp_mem_exit_callback_register_internal(const char *file, const int line)
{
	rp_assertf(!(flags & FLAG_IS_INIT),
		   "Somehow, first allocation has already "
		   "been made before registering the callback.");
	rp_assertf(!(flags & FLAG_HAS_CALLBACK),
		   "Callback was already registered");

#ifndef RP_MEMORY_LOG
	(void)file;
	(void)line;
#endif /* #ifndef RP_MEMORY_LOG */

	mem_debugf("Registered exit callback at %s:%d\n", file, line);
	atexit(_mem_atexit);

	flags |= FLAG_HAS_CALLBACK;
}

void *_rp_mem_alloc_internal(const size_t sz, const char *file, const int line)
{
	struct block *s = NULL, *t = NULL;
	void	     *p = NULL;

	/* If we didn't register a callback, don't go further. */
	rp_assertf_ex(flags & FLAG_HAS_CALLBACK,
		      file,
		      line,
		      "User hasn't called `mem_register_exit_"
		      "callback()` before calling `rp_mem_alloc()`.");

	/* First call of `rp_mem_alloc()`. */
	if (!(flags & FLAG_IS_INIT))
		_mem_init();

	rp_assertf_ex(sz, file, line, "Trying to allocate 0 bytes!");

	/* Grow the array from the allocation */
	_array_resize((void **)&memory.alloc_arr,
		      sizeof(*memory.alloc_arr),
		      memory.alloc_cnt,
		      ++memory.alloc_cnt);

	/* Get the new empty slot from the end of it */
	s = memory.alloc_arr + memory.alloc_cnt - 1;
	_mem_block_set_empty(s, LIST_ALLOC);
	_mem_block_verify(s, LIST_ALLOC, file, line);

	p = malloc(sz);
	rp_assertf(p, "Failed to allocate pointer of size %zu", sz);

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
	s->ptr	= p;
	s->sz	= sz;
	s->file = file;
	s->line = (u32)line;
	_mem_block_verify(s, LIST_ALLOC, file, line);

	mem_debugf("rp_mem_alloc(%zu) -> <%p> %s:%d\n", sz, p, file, line);

	/* Send the user pointer on it's way to the caller. */
	return p;
}

void _rp_mem_free_internal(void *ptr, const char *file, const int line)
{
	struct block *s;

	rp_assertf_ex(flags & FLAG_IS_INIT,
		      file,
		      line,
		      "No allocation was previously made, "
		      "so there's nothing to free.");
	rp_assertf_ex(ptr, file, line, "Trying to free a NULL pointer");

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
	rp_assertf(s,
		   file,
		   line,
		   "Failed to get allocated block from user pointer <%p>",
		   ptr);
	mem_debugf("rp_mem_free(<%p>) [sz:%zu] %s:%d\n",
		   s->ptr,
		   s->sz,
		   file,
		   line);
	_mem_move_block_to_free_list(s, file, line);
}

#ifdef RP_MEMORY_TEST
#include "rp_random.h"

static void rp_memory_test(void)
{
#ifdef malloc
#error "MEM_TEST: `malloc` as a macro was already defined elsewhere"
#endif /* #ifdef malloc */

#ifdef free
#error "MEM_TEST: `free` as a macro was already defined elsewhere"
#endif /* #ifdef free */

#define malloc(_sz) _rp_mem_alloc_internal(_sz, __FILE__, __LINE__)
#define free(_sz)   _rp_mem_free_internal(_sz, __FILE__, __LINE__)

	static u32 *ptr_test[RP_MEMORY_TEST_ALLOC_CNT] = { NULL };
	size_t	    i;

	fprintf(stdout, "\n[TEST] rp_memory.h:\n\n");

	_rp_mem_exit_callback_register_internal(__FILE__, __LINE__);

	rp_random_seed(UINT32_MAX);

	/* Allocate a bunch of memory */
	for (i = 0ul; i < RP_MEMORY_TEST_ALLOC_CNT; ++i) {
		size_t j;
		u8     num;

		do {
			num = rp_random_u8();
		} while (!num);

		ptr_test[i] = (u32 *)malloc(sizeof(**ptr_test) * num);
		rp_assertf(ptr_test[i],
			   "Failed to allocate pointer "
			   "test index %zu of size %zu",
			   i,
			   sizeof(**ptr_test) * num);

		/* 2/3 of the time, just continue like normal */
		if (rp_random_u32() % 3) {
			for (j = 0ul; j < num; ++j)
				*(ptr_test[i]) = rp_random_u32();

			continue;
		}

		/*
		 * The other 1/3 of the time, go through all that's currently
		 * allocated, and randomly (another 50%) free those pointers.
		 */
		for (j = 0ul; j <= i; ++j) {
			if (rp_random_bool_50_percent())
				continue;

			if (!ptr_test[j])
				continue;

			free(ptr_test[j]);
			ptr_test[j] = NULL;
		}
	}

	/* Free all of it */
#ifndef RP_MEMORY_TEST_SIMULATE_LEAK
	for (i = 0ul; i < RP_MEMORY_TEST_ALLOC_CNT; ++i) {
		if (!ptr_test[i])
			continue;

		free(ptr_test[i]);
		ptr_test[i] = NULL;
	}
#endif /* #ifndef RP_MEMORY_TEST_SIMULATE_LEAK */
#undef free
#undef malloc
}
#endif /* #ifdef RP_MEMORY_TEST */

#undef _ASSERT_WHICH_IS_VALID

#endif /* #ifdef RP_MEMORY_IMPLEMENTATION */

/*************************
 * PUBLIC MACRO WRAPPERS *
 *************************/

/*
 * Macro defines for wrapping the calls to `_mem_*_internal()` by giving
 * them prettier names and passing in the __FILE__ and __LINE__ they were
 * called from for glorious, glorious debugging purposes!
 */
#define rp_mem_exit_callback_register()                                        \
	_rp_mem_exit_callback_register_internal(__FILE__, __LINE__)
#define rp_mem_alloc(_sz) _rp_mem_alloc_internal(_sz, __FILE__, __LINE__)
#define rp_mem_free(_ptr) _rp_mem_free_internal(_ptr, __FILE__, __LINE__)

#ifdef RP_MEMORY_WRAP_STDLIB
#define malloc(_sz) rp_mem_alloc(_sz)
#define free(_sz)   rp_mem_free(_sz)
#endif /* #ifdef RP_MEMORY_WRAP_STDLIB */

#endif /* #ifndef _RP_MEMORY_H_ */

/*
 * Also, for what it's worth, for as much as I talk shit
 * about forgetting to free your memory, most of the time
 * it's actually me who's making that mistake. Hence most
 * of the reason I decided to make this in the first place.
 *
 * So don't take it personally, you're doing great! :D
 */
