#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include "memory_allocator.h"
#include "types.h"

/******************************************************************************
 * TODO:                                                                      *
 *                                                                            *
 * > Implement a free-list for double-free checking                           *
 *                                                                            *
 * > Make this into a header-only library. Not                                *
 *   ideal, obviously, but extremely portable.                                *
 *                                                                            *
 * > The custom error codes I use in `_mem_block_verify()` should be          *
 *   expanded to encapsulate all errors that could be experienced by the      *
 *   allocator so that it doesn't just crash when something goes wrong.       *
 *   I personally love crashing when shit goes wrong, since that forces me    *
 *   to fix it, but the general public doesn't seem to be a big fan. lol      *
 *                                                                            *
 * > I think a cleaner implementation for how `_mem_atexit()` handles doing   *
 *   the whole free-all-the-blocks-left-over-by-the-user shtick is that,      *
 *   instead of allocating an indices array and indexing them that way, just  *
 *   sort the existing memory block array such that all the active blocks are *
 *   at the beginning of the array and loop through those. At this point, the *
 *   program is already over and the user isn't going to be using those       *
 *   pointers anymore anyway, so sorting the data really won't affect         *
 *   anything. It also doesn't allocate anything extra, so there's that. lol  *
 ******************************************************************************/

#undef mem_alloc
#undef mem_free

#ifdef MEMORY_ALLOCATOR_WRAP_STDLIB
#undef malloc
#undef free
#endif /* #ifdef MEMORY_ALLOCATOR_WRAP_STDLIB */

struct block {
	const char *file;
	void	   *ptr;
	size_t	    sz;
	u32	    line; /* If your file has anywhere NEAR 4294967295 lines,
			     I think you're already pretty fucked, bucko... */
	u32 _pad;
};

static struct memory {
	struct block *arr;
	size_t	      cnt;
} memory;

enum {
	FLAGS_NONE	  = 0,
	FLAG_IS_INIT	  = (1 << 0),
	FLAG_HAS_CALLBACK = (1 << 1),
	FLAGS_MASK	  = (FLAG_IS_INIT | FLAG_HAS_CALLBACK)
};

typedef u8 internal_flags_e;

static internal_flags_e flags = FLAGS_NONE;

/*********************
 * PRIVATE FUNCTIONS *
 *********************/

/*
 * Prints a formatted message to STDERR only if `ALLOCATOR_DEBUG` is defined.
 */
static void _mem_debugf(const char *fmt, ...)
{
#ifdef ALLOCATOR_DEBUG
	va_list args;

	assert(fmt);
	va_start(args, fmt);
	(void)fprintf(stderr, "MEM: ");
	(void)vfprintf(stderr, fmt, args);
	va_end(args);
#else  /* #ifdef ALLOCATOR_DEBUG */
	(void)fmt;
#endif /* #ifdef ALLOCATOR_DEBUG #else */
}

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

#define ASSERT_MEM_ARR(_file, _line)                                           \
	do {                                                                   \
		mem_assertm_ex(memory.cnt, file, line, "Memory count is 0");   \
		mem_assertm_ex(memory.arr,                                     \
			       _file,                                          \
			       _line,                                          \
			       "Memory array is NULL");                        \
	} while (0)

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
#ifdef ALLOCATOR_DEBUG
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
	exit(EXIT_FAILURE);
#else  /* #ifdef ALLOCATOR_DEBUG */
	(void)cond;
	(void)cond_str;
	(void)file;
	(void)line;
	(void)fmt;
#endif /* #ifdef ALLOCATOR_DEBUG #else */
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
static void
_mem_block_verify(const struct block *b, const char *file, const int line)
{
	size_t ind = SIZE_MAX;

	ASSERT_MEM_ARR(file, line);

	mem_assertm_ex(b, file, line, "Block passed in is NULL");
	ind = (size_t)(b - memory.arr);
	mem_assertf_ex(ind < memory.cnt,
		       file,
		       line,
		       "Index %lu is out of range! (Max is %lu)",
		       ind,
		       memory.cnt);

#ifdef ALLOCATOR_DEBUG_VERBOSE
	_mem_debugf("Verifying memory slot %lu\n", ind);
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE */

	/* If our pointer is NULL, the rest of our data better match! */
	if (!b->ptr) {
		mem_assertf_ex(!b->sz,
			       file,
			       line,
			       "Block <%p>'s pointer is NULL, "
			       "but size is greather than 0",
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
static struct block *
_mem_block_get(const size_t i, const char *file, const int line)
{
	struct block *b = NULL;

	ASSERT_MEM_ARR(file, line);
	mem_assertf_ex(i < memory.cnt,
		       file,
		       line,
		       "Block index %lu is invalid! (Max is %lu)",
		       i,
		       memory.cnt);

#ifdef ALLOCATOR_DEBUG_VERBOSE
	_mem_debugf("Got block at index %lu @ %s:%d\n", i, file, line);
#else  /* #ifdef ALLOCATOR_DEBUG_VERBOSE */
	(void)file;
	(void)line;
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE #else */

	b = memory.arr + i;
	_mem_block_verify(b, file, line);

	return b;
}

/*
 * Safely gets and returns the pointer to the first block
 * at the beginning of the allocator memory block array.
 */
static struct block *_mem_block_get_first(const char *file, const int line)
{
	ASSERT_MEM_ARR(file, line);

	return _mem_block_get(0ul, file, line);
}

/*
 * Safely gets and returns the pointer to the last block
 * at the end of the allocator memory block array.
 */
static struct block *_mem_block_get_last(const char *file, const int line)
{
	ASSERT_MEM_ARR(file, line);

	return _mem_block_get(memory.cnt - 1ul, file, line);
}

/*
 * Gets a memory block from the allocator's array using- not an index- but
 * the pointer that was allocated TO the block's pointer.
 *
 * This is really only used internally for `mem_free()`, since
 * you pass in the user-allocated pointer, but you also need
 * to clear out the block that pointer is associated with.
 */
static struct block *
_mem_block_from_pointer(const void *ptr, const char *file, const int line)
{
	size_t i;

	ASSERT_MEM_ARR(file, line);

	for (i = 0ul; i < memory.cnt; ++i) {
		struct block *s = _mem_block_get(i, file, line);

		if (s->ptr == ptr)
			return s;
	}

	mem_assertf_ex(0,
		       file,
		       line,
		       "Failed to get memory slot from pointer <%p>",
		       ptr);
	return NULL;
}

/*
 * Helper function for zero-initializing a single memory block internally.
 */
static void _mem_block_set_empty(struct block *b)
{
	mem_assertm(memory.cnt, "Memory count is 0");
	mem_assertm(memory.arr, "Memory array is NULL");

	mem_assertm(b, "Block pointer is NULL");
	mem_assertf((size_t)(b - memory.arr) < memory.cnt,
		    "Block pointer <%p> is out of range of memory "
		    "block array; (is %lu, should be less than %lu)",
		    (size_t)(b - memory.arr),
		    memory.cnt);

	b->ptr	= NULL;
	b->sz	= 0ul;
	b->file = NULL;
	b->line = UINT32_MAX;
}

/*
 * Frees the user-allocated pointer associated with a memory block.
 */
static void _mem_block_pointer_free(struct block *s,
				    const bool_t  was_called_from_exit,
				    const char	 *file,
				    const int	  line)
{
	ASSERT_MEM_ARR(file, line);

	_mem_block_verify(s, file, line);

	/*
	 * If this function was called from exit, it means that
	 * these were blocks that were failed to be freed by the
	 * user, which already has debug information printing
	 * about it, so we don't need to print this again.
	 */
	if (!was_called_from_exit) {
		_mem_debugf("mem_free(<%p>) [sz:%lu] %s:%d\n",
			    s->ptr,
			    s->sz,
			    file,
			    line);
	}

	free(s->ptr);
	_mem_block_set_empty(s);
}

/*
 * Just gets the number of blocks in the array
 * that have active pointers attached to them.
 */
static __inline size_t _mem_active_blocks_get_count(const char *file,
						    const int	line)
{
	size_t i, cnt = 0ul;

	ASSERT_MEM_ARR(file, line);

	for (i = 0ul; i < memory.cnt; ++i)
		if (memory.arr[i].ptr)
			++cnt;

	return cnt;
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
	size_t i, j, active_cnt = SIZE_MAX;

	/* If we never called `malloc()` in our program, we're good to exit. */
	if (!(flags & FLAG_IS_INIT)) {
		assert(!memory.arr);
		assert(!memory.cnt);
		_mem_debugf("No memory was allocated in this "
			    "program; nothing to report.\n");
		goto finish_terminate;
	}

	active_cnt = _mem_active_blocks_get_count(__FILE__, __LINE__);
	if (!active_cnt) {
		_mem_debugf("\n");
		_mem_debugf("NO BLOCKS LEFT ACTIVE AT EXIT; GOOD JOB!\n");
		goto finish_terminate_free_internals;
	}

	_mem_debugf("\n");
	_mem_debugf("WARNING: %d BLOCKS STILL ACTIVE AT EXIT:\n", active_cnt);

	j = 0ul;
	for (i = 0ul; i < memory.cnt; ++i) {
		struct block *s = _mem_block_get(i, __FILE__, __LINE__);

		if (!s->ptr)
			continue;

		_mem_debugf("\tLEAK %lu: [p:<%p> sz:%lu] %s:%u\n",
			    j,
			    s->ptr,
			    s->sz,
			    s->file,
			    s->line);
		_mem_block_pointer_free(s, TRUE, __FILE__, __LINE__);
		++j;
	}

finish_terminate_free_internals:
	free(memory.arr);
	memory.arr = NULL;
	memory.cnt = 0ul;

finish_terminate:
	flags &= ~FLAG_IS_INIT;

	_mem_debugf("TERMINATED SUCCESSFULLY!\n");
}

#if 0
static __inline s32 _tmp_arr_sort(const void *a, const void *b)
{
	const s32 a8 = (s16)(*(const u8 *)a);
	const s32 b8 = (s16)(*(const u8 *)b);
	s32 r = INT32_MAX;

	if (a8 > b8)
		r = 1;
	else if (a8 < b8)
		r = -1;
	else
		r = 0;

	_mem_debugf("sort res: %d\n", r);

	return r;
}
#endif

/*
 * The comparison function necessary for the below function to work at all.
 */
static int _mem_block_null_cmp(const void *a, const void *b)
{
	const struct block *at = (const struct block *)a;
	const struct block *bt = (const struct block *)b;

#if 0
	_mem_block_verify(at, __FILE__, __LINE__);
	_mem_block_verify(bt, __FILE__, __LINE__);
#endif

	if (!at->ptr)
		return -1;

	if (!bt->ptr)
		return 1;

	return 0;
}

#if 0
static __inline size_t rand_size(void)
{
	size_t r = 0ul;

	r |= (size_t)(rand() & 0xFFFF) << 0ul;
	r |= (size_t)(rand() & 0xFFFF) << 16ul;
	r |= (size_t)(rand() & 0xFFFF) << 32ul;
	r |= (size_t)(rand() & 0xFFFF) << 48ul;

	return r;
}
#endif

/*
 * This simply enables `FLAG_IS_INIT` in `flags`,
 * as that's all it really needs to do for now.
 *
 * This function is only called internally by `mem_alloc()` in order to
 * ensure that it's set up and the rest of the internal's recognize that.
 */
static void _mem_init(void)
{
	assert(!memory.cnt);
	assert(!memory.arr);
	assert(!(flags & FLAG_IS_INIT)); /* Make sure we didn't already */
	flags |= FLAG_IS_INIT;
	_mem_debugf("INITIALIZED SUCCESSFULLY!\n");
}

/*
 * Internal helper function
 */
static __inline bool_t _mem_block_array_has_null_elements(const char *file,
							  const int   line)
{
	size_t i;

	for (i = 0ul; i < memory.cnt; ++i)
		if (!_mem_block_get(i, file, line)->ptr)
			return TRUE;

	return FALSE;
}

/*
 * Sorts the internal memory block array such that the unused slots
 * are sorted towards the beginning and the non-NULL ones are at the
 * end. That way it's easy to just allocate a new slot right at the
 * beginning of the array instead of needing to search for it.
 *
 * Returns if the array needs to be (re)allocated.
 */
static void _mem_block_array_sort_null_first(const char *file, const int line)
{
	assert(memory.cnt);
	assert(memory.arr);

	if (!_mem_block_get_first(file, line)->ptr) {
#ifdef ALLOCATOR_DEBUG_VERBOSE
		_mem_debugf("First mem block is already NULL; "
			    "no need to sort array.\n");
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE */
		return;
	}

	qsort(memory.arr,
	      memory.cnt,
	      sizeof(*memory.arr),
	      &_mem_block_null_cmp);

#ifdef ALLOCATOR_DEBUG_VERBOSE
	{
		size_t i;

		_mem_debugf("Sorted array, here's the results... %s:%d\n",
			    file,
			    line);
		for (i = 0ul; i < memory.cnt; ++i) {
			_mem_debugf("\t%lu: ptr = <%p>\n",
				    i,
				    _mem_block_get(i, file, line)->ptr);
		}
	}
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE */
}

/*
 * To cut down on the amount of sorts needed to perform, get the last
 * available NULL slot post-sort so that each subsequent one will make it
 * so that if there are multiple, there will be no need to re-sort the array
 * until all of the NULL slots have been taken up.
 */
static struct block *_mem_block_get_last_empty_post_sort(const char *file,
							 const int   line)
{
	struct block *last_null = NULL;
	size_t	      i;

	ASSERT_MEM_ARR(file, line);

	for (i = 0ul; i < memory.cnt; ++i) {
		struct block *b = _mem_block_get(i, file, line);

		if (!b->ptr)
			last_null = b;
	}

	if (last_null)
		return last_null;

	mem_assertm_ex(0,
		       file,
		       line,
		       "Failed to find any NULL slots in array post-sort");

	return NULL;
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

	_mem_debugf("Registered exit callback at %s:%d\n", file, line);
	atexit(_mem_atexit);

	flags |= FLAG_HAS_CALLBACK;
}

void *_mem_alloc_internal(const size_t sz, const char *file, const int line)
{
	struct block *s = NULL;
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

	/* Call `malloc()` or `realloc()` on the array if necessary */
	if (!_mem_block_array_has_null_elements(file, line)) {
		const size_t  psz  = memory.cnt;
		const size_t  nsz  = sizeof(*memory.arr) * ++memory.cnt;
		struct block *last = NULL;
		void	     *all  = NULL;

		if (!psz) {
			assert(!memory.arr);
			all = malloc(nsz);
		} else {
			assert(memory.arr);
			all = realloc(memory.arr, nsz);
		}

		assert(all);
		memory.arr = (struct block *)all;

		last = _mem_block_get_last(file, line);
		assert(last);
		_mem_block_set_empty(last);
		_mem_block_verify(last, file, line);
	}

	_mem_block_array_sort_null_first(file, line);
	s = _mem_block_get_last_empty_post_sort(file, line);
	_mem_block_verify(s, file, line);
	p = malloc(sz);
	assert(p);
	s->ptr	= p;
	s->sz	= sz;
	s->file = file;
	s->line = (u32)line;

	_mem_debugf("mem_alloc(%lu) -> <%p> %s:%d\n", sz, p, file, line);

	return p;
}

void _mem_free_internal(void *ptr, const char *file, const int line)
{
	struct block *s;

	assert(flags & FLAG_IS_INIT);

	assert(ptr);

	s = _mem_block_from_pointer(ptr, file, line);
	assert(s);

	_mem_block_pointer_free(s, FALSE, file, line);
}
