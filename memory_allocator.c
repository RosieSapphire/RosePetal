#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include "memory_allocator.h"
#include "boolean.h"
#include "int_types.h"

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
 * > `_mem_block_verify()` is _always_ called before a                        *
 *   subsequent call to `_mem_block_exit_if_error()`, so                      *
 *   these two functions should be concatonated into one.                     *
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

enum mem_verify_result {
	MVR_INVALID_RESULT = -10,
	MVR_FAIL_0_SLOTS_CNT,
	MVR_FAIL_INDEX_TOO_HIGH,
	MVR_FAIL_NULL_SLOTS_ARR,
	MVR_FAIL_PTR_NULL_WITH_SIZE,
	MVR_FAIL_PTR_NULL_WITH_FILE,
	MVR_FAIL_PTR_NULL_WITH_LINE,
	MVR_FAIL_PTR_NON_NULL_NO_SIZE,
	MVR_FAIL_PTR_NON_NULL_NO_FILE,
	MVR_FAIL_PTR_NON_NULL_NO_LINE,
	MVR_SUCCESS
};

struct block {
	const char *file;
	void	   *ptr;
	size_t	    sz;
	u32	    line; /* If your file has anywhere near 4294967295 lines,
			     I think you're already pretty fucked, bucko... */
};

static struct memory {
	struct block *arr;
	size_t	      cnt;
} memory;

enum internal_flags {
	FLAGS_NONE	  = 0,
	FLAG_IS_INIT	  = (1 << 0),
	FLAG_HAS_CALLBACK = (1 << 1),
	FLAGS_MASK	  = (FLAG_IS_INIT | FLAG_HAS_CALLBACK)
};

static enum internal_flags flags = FLAGS_NONE;

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

/*
 * Safely gets and returns the pointer to a memory block
 * at index `i` from the allocator with bounds checking.
 */
static struct block *
_mem_block_get(const size_t i, const char *file, const int line)
{
	assert(memory.cnt);
	assert(memory.arr);
	assert(i < memory.cnt);

#ifdef ALLOCATOR_DEBUG_VERBOSE
	_mem_debugf("Got block at index %lu @ %s:%d\n", i, file, line);
#else  /* #ifdef ALLOCATOR_DEBUG_VERBOSE */
	(void)file;
	(void)line;
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE #else */

	return memory.arr + i;
}

/*
 * Safely gets and returns the pointer to the last block
 * at the end of the allocator memory block array.
 */
static struct block *_mem_block_get_last(const char *file, const int line)
{
	assert(memory.cnt);
	assert(memory.arr);

	return _mem_block_get(memory.cnt - 1, file, line);
}

/*
 * Safely gets and returns the index of a specified memory block `s`
 * within the allocator's array.
 *
 * Effectively the counterpart to `_mem_block_get()`
 * which returns a pointer FROM an index; this
 * one converts a pointer TO an index.
 */
static size_t
_mem_block_get_index(const struct block *s, const char *file, const int line)
{
	size_t ind = SIZE_MAX;

	assert(memory.cnt);
	assert(memory.arr);
	assert(s);

	ind = (size_t)(s - memory.arr);
	assert(ind < memory.cnt);

#ifdef ALLOCATOR_DEBUG_VERBOSE
	_mem_debugf("Block pointer <%p> is index %lu in memory array.\n",
		    file,
		    line);
#else  /* #ifdef ALLOCATOR_DEBUG_VERBOSE */
	(void)file;
	(void)line;
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE #else */

	return ind;
}

/*
 * Verifies that the state of a memory block at index `ind` is valid.
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
static enum mem_verify_result
_mem_block_verify(const size_t ind, const char *file, const int line)
{
#define GOTO_END(_code)                                                        \
	do {                                                                   \
		r = (_code);                                                   \
		goto mem_verify_end;                                           \
	} while (0)

	struct block	      *s = NULL;
	enum mem_verify_result r = MVR_INVALID_RESULT;

#ifdef ALLOCATOR_DEBUG_VERBOSE
	_mem_debugf("Verifying memory slot %lu\n", ind);
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE */

	if (!memory.cnt)
		GOTO_END(MVR_FAIL_0_SLOTS_CNT);

#ifdef ALLOCATOR_DEBUG_VERBOSE
	_mem_debugf("Block %lu: Allocator has block count. %s:%d\n",
		    ind,
		    file,
		    line);
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE */

	if (ind >= memory.cnt)
		GOTO_END(MVR_FAIL_INDEX_TOO_HIGH);

#ifdef ALLOCATOR_DEBUG_VERBOSE
	_mem_debugf("Block %lu: Index is within range. %s:%d\n",
		    ind,
		    file,
		    line);
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE */

	if (!memory.arr)
		GOTO_END(MVR_FAIL_NULL_SLOTS_ARR);

#ifdef ALLOCATOR_DEBUG_VERBOSE
	_mem_debugf("Block %lu: Allocator has an array. %s:%d\n",
		    ind,
		    file,
		    line);
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE */

	s = _mem_block_get(ind, file, line);

	/* If our pointer is NULL, the rest of our data better match! */
	if (!s->ptr) {
		if (s->sz)
			GOTO_END(MVR_FAIL_PTR_NULL_WITH_SIZE);

#ifdef ALLOCATOR_DEBUG_VERBOSE
		_mem_debugf("Block %lu: No ptr; No size. %s:%d\n",
			    ind,
			    file,
			    line);
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE */

		if (s->file)
			GOTO_END(MVR_FAIL_PTR_NULL_WITH_FILE);

#ifdef ALLOCATOR_DEBUG_VERBOSE
		_mem_debugf("Block %lu: No ptr; No file. %s:%d\n",
			    ind,
			    file,
			    line);
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE */

		if (s->line != UINT32_MAX)
			GOTO_END(MVR_FAIL_PTR_NULL_WITH_LINE);

#ifdef ALLOCATOR_DEBUG_VERBOSE
		_mem_debugf("Block %lu: No ptr; No line. %s:%d\n",
			    ind,
			    file,
			    line);
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE */

		GOTO_END(MVR_SUCCESS);
	}

	/* Same for if it's non-NULL, just in the other direction. */
	if (!s->sz)
		GOTO_END(MVR_FAIL_PTR_NON_NULL_NO_SIZE);

#ifdef ALLOCATOR_DEBUG_VERBOSE
	_mem_debugf("Block %lu: Has ptr; Has size (%lu). %s:%d\n",
		    ind,
		    s->sz,
		    file,
		    line);
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE */

	if (!s->file)
		GOTO_END(MVR_FAIL_PTR_NON_NULL_NO_FILE);

#ifdef ALLOCATOR_DEBUG_VERBOSE
	_mem_debugf("Block %lu: Has ptr; Has file (\"%s\"). %s:%d\n",
		    ind,
		    s->file,
		    file,
		    line);
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE */

	if (s->line == UINT32_MAX)
		GOTO_END(MVR_FAIL_PTR_NON_NULL_NO_LINE);

#ifdef ALLOCATOR_DEBUG_VERBOSE
	_mem_debugf("Block %lu: Has ptr; Has line (%lu). %s:%d\n",
		    ind,
		    s->line,
		    file,
		    line);
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE */

	GOTO_END(MVR_SUCCESS);

mem_verify_end:
#ifdef ALLOCATOR_DEBUG_VERBOSE
	if (r == MVR_SUCCESS) {
		_mem_debugf("Memory slot %lu is valid! %s:%d\n",
			    ind,
			    file,
			    line);
	}
#else  /* #ifdef ALLOCATOR_DEBUG_VERBOSE */
	(void)file;
	(void)line;
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE #else */

	return r;

#undef GOTO_END
}

/*
 * Takes a memory block verify return code `r` and crashes the program
 * after printing out the description associated with said error code.
 *
 * NOTE: This function simply returns if the code happens to be `MVR_SUCCESS`,
 * as there is no earthy reason to crash if nothing went wrong. lol
 */
static void _mem_block_exit_if_error(const enum mem_verify_result r,
				     const size_t		  i,
				     const char			 *file,
				     const int			  line)
{
#define MSG_ARR_LEN	     (-MVR_INVALID_RESULT + 1)
#define MVR_TO_MSG_INDEX(_r) ((size_t)(_r + MSG_ARR_LEN - 1))

	const char *msg_arr[MSG_ARR_LEN] = {
		"<INVALID RESULT>",
		"Allocator has 0 slots",
		"Index is too high",
		"Allocator slots array is NULL",
		"Has NULL pointer, but has >= 0 size",
		"Has NULL pointer, but has non-NULL file",
		"Has NULL pointer, but has non-UINT32_MAX line",
		"Pointer is non-NULL, but size is 0",
		"Pointer is non-NULL, but file is NULL",
		"Pointer is non-NULL, but line is UINT32_MAX",
		"Success"
	};
	const char *msg = NULL;

	if (r == MVR_SUCCESS)
		return;

	assert(r >= MVR_INVALID_RESULT);
	assert(r < MVR_SUCCESS);

	msg = msg_arr[MVR_TO_MSG_INDEX(r)];

	_mem_debugf("Memory slot %lu is invalid: \"%s\". Occured at %s:%d\n",
		    i,
		    msg,
		    file,
		    line);
	exit(EXIT_FAILURE);

#undef MVR_TO_MSG_INDEX
#undef MSG_ARR_LEN
}

/*
 * Finds the first available empty memory block in the allocator's array.
 *
 * There are a few cases to this function:
 *	1. The allocator's array is NULL, as no blocks have previously
 *	   been allocated, and so it uses `malloc()` to initialize the
 *	   block array and returns that pointer, as it's the first element.
 *
 *	2. The allocator's array already has slots, but none of them are
 *	   available to assign to, so it allocates a brand new slot into
 *	   the array.
 *
 *	3. It goes through the allocator's array and
 *	   successfully finds an empty slot to use.
 *
 * Regardless of the outcome, unless the system is out of memory,
 * this function should _never_ return NULL.
 */
static struct block *_mem_block_first_empty_get(const char *file,
						const int   line)
{
	struct block	      *s = NULL;
	enum mem_verify_result r = MVR_INVALID_RESULT;
	size_t		       i, newsz;

	assert(flags & FLAG_IS_INIT);
	assert(file);
	assert(line >= 0);

	/* Search in the already-existing array for a free slot. */
	for (i = 0ul; i < memory.cnt; ++i) {
		s = _mem_block_get(i, file, line);
		r = _mem_block_verify(i, file, line);
		_mem_block_exit_if_error(r, i, file, line);

		if (!s->ptr)
			return s;
	}

	/* If we couldn't find one, allocate a new one and return it! */
	newsz = sizeof(*memory.arr) * ++memory.cnt;
	if (memory.cnt && memory.arr)
		s = (struct block *)realloc(memory.arr, newsz);
	else
		s = (struct block *)malloc(newsz);

	assert(s);
	memory.arr = s;

	return _mem_block_get_last(file, line);
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

	assert(memory.cnt && memory.arr);

	for (i = 0ul; i < memory.cnt; ++i) {
		struct block		    *s = _mem_block_get(i, file, line);
		const enum mem_verify_result r =
				_mem_block_verify(i, file, line);

		_mem_block_exit_if_error(r, i, file, line);

		if (s->ptr == ptr)
			return s;
	}

	_mem_debugf("Failed to get memory slot from pointer <%p>\n", ptr);
	exit(EXIT_FAILURE);
}

/*
 * Frees the user-allocated pointer associated with a memory block.
 */
static void _mem_block_pointer_free(struct block *s,
				    const bool_t  was_called_from_exit,
				    const char	 *file,
				    const int	  line)
{
	assert(s);
	assert(s->ptr);
	assert(s->sz);

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
	s->ptr	= NULL;
	s->sz	= 0ul;
	s->file = NULL;
	s->line = UINT32_MAX;
}

/*
 * Searches through the allocator's memory block array and finds all the slots
 * that have actively allocated pointers, and returns an array of `size_t`
 * elements containing the indices into the allocator's array, as well as the
 * size of that index array to `cnt_out`.
 *
 * This is only called internally from `_mem_atexit()` as a means of
 * determining how much user-allocated memory was still in use at the
 * time of the program ending.
 *
 * RATIONALE:
 * The reason this function is important is because of the way the
 * allocator works in general. Starting from the first call to `mem_alloc()`,
 * when you allocate a new pointer, it has to allocate an internal array for
 * storing the actual memory blocks before it can call `malloc()` for that
 * block for storing the user's pointer.
 *
 * However, when a pointer is _freed_, the user pointer stored at the internal
 * block location is wiped out cleanly, but the BLOCK stays in memory. There
 * are a few reaons for this:
 *
 *	1. If we wanted the internal memory block array to be packed tightly,
 *	   that would mean re-sorting and re-allocating it every single time
 *	   a called to `mem_alloc()` or `mem_free()` is made. This is really
 *	   slow and inefficient, but also mostly pointless.
 *
 *	2. It cuts down on the amount of `realloc()` calls since if there's
 *	   a bunch of free slots already in the internal memory allocator,
 *	   it's better to just use those than freeing and reallocating the
 *	   same space we literally just fucking freed.
 *
 *	3. This provides a benchmark for us to know what the maximum amount
 *	   of memory blocks active at a single point in the program was.
 *	   This may not be the _most_ useful thing in the works, but it means
 *	   if our program is running on a low-memory machine, we can know how
 *	   many blocks were active at once at the peak of the program's memory
 *	   usage to hopefully work around those limitations better.
 *
 * Anyway, this function is probably gonna be replaced in the future with
 * one that just sorts the array such that the non-NULL elements are all at
 * the front of the list so they can be looped through more efficiently without
 * needing to allocate more memory for an index buffer.
 *
 * Counter to this, it might be a decent idea to sort the memory array the
 * _opposite_ (NULL elements at front) to make searching for an empty slot
 * not take O(n) time. Basically, we'd sort the array like that and just get
 * the first element of the array. If it's NULL, we know for sure there are
 * no more memory slots and we can just skip a bunch of looping through shit.
 *
 * Granted, there is a question of if the performance trade-off between
 * searching for first free element linearly through an array is faster
 * than sorting the array such that the NULL elements (if any) are present
 * at the beginning of the array, but that'll require some measuring. lol
 */
static size_t *
_mem_active_block_indices_get(size_t *cnt_out, const char *file, const int line)
{
	size_t *arr = NULL, cnt = 0ul, i;

	arr = malloc(0);
	assert(arr);

	for (i = 0ul; i < memory.cnt; ++i) {
		const struct block	    *b = _mem_block_get(i, file, line);
		const enum mem_verify_result r =
				_mem_block_verify(i, file, line);
		size_t *re = NULL;

		_mem_block_exit_if_error(r, i, file, line);
		if (!b->ptr)
			continue;

		re = realloc(arr, sizeof(*arr) * ++cnt);
		assert(re);
		re[cnt - 1ul] = i;
		arr	      = re;
	}

	*cnt_out = cnt;

	if (!cnt) {
		free(arr);
		arr = NULL;
	}

	return arr;
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
	size_t	i, active_cnt = SIZE_MAX;
	size_t *active_index_arr = NULL;

	if (!(flags & FLAG_IS_INIT)) {
		assert(!memory.arr);
		assert(!memory.cnt);
		_mem_debugf("No memory was allocated in this "
			    "program; nothing to report.\n");
		exit(EXIT_SUCCESS);
	}

	active_index_arr = _mem_active_block_indices_get(&active_cnt,
							 __FILE__,
							 __LINE__);
	if (!active_cnt) {
		assert(!active_index_arr);
		_mem_debugf("\n");
		_mem_debugf("NO BLOCKS LEFT ACTIVE AT EXIT; GOOD JOB!\n");
		goto finish_terminate;
	}

	_mem_debugf("\n");
	_mem_debugf("WARNING: %d BLOCKS STILL ACTIVE AT EXIT:\n", active_cnt);

	for (i = 0ul; i < active_cnt; ++i) {
		struct block *s = _mem_block_get(active_index_arr[i],
						 __FILE__,
						 __LINE__);
		const enum mem_verify_result r =
				_mem_block_verify(i, __FILE__, __LINE__);

		_mem_block_exit_if_error(r, i, __FILE__, __LINE__);

		if (!s->ptr)
			continue;

		_mem_debugf("\tLEAK %lu: [p:<%p> sz:%lu sl:%lu] "
			    "%s:%lu\n",
			    i,
			    s->ptr,
			    s->sz,
			    _mem_block_get_index(s, __FILE__, __LINE__),
			    s->file,
			    s->line);
		_mem_block_pointer_free(s, TRUE, __FILE__, __LINE__);
	}

	if (active_index_arr)
		free(active_index_arr);

finish_terminate:
	flags &= ~FLAG_IS_INIT;

	_mem_debugf("TERMINATED SUCCESSFULLY!\n");
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
	assert(!memory.cnt);
	assert(!memory.arr);
	assert(!(flags & FLAG_IS_INIT)); /* Make sure we didn't already */
	flags |= FLAG_IS_INIT;
	_mem_debugf("INITIALIZED SUCCESSFULLY!\n");
}

/********************
 * PUBLIC FUNCTIONS *
 ********************/

/*
 * Internal function for `mem_register_exit_callback()`.
 */
void _mem_register_exit_callback_internal(const char *file, const int line)
{
	/*
	 * Make sure the callback is registered before we initialize the
	 * module, which would correspond to the first memory allocation.
	 */
	assert(!(flags & FLAG_IS_INIT));

	/* Obviously, make sure we haven't already called this function. */
	assert(!(flags & FLAG_HAS_CALLBACK));

	assert(file);
	assert(line >= 0);
	_mem_debugf("Registered exit callback at %s:%d\n", file, line);
	atexit(_mem_atexit);

	flags |= FLAG_HAS_CALLBACK;
}

void *_mem_alloc_internal(const size_t sz, const char *file, const int line)
{
	struct block *s = NULL;
	void	     *p = NULL;

	assert(file);
	assert(line >= 0);

	/* If we didn't register a callback, don't go further. */
	if (!(flags & FLAG_HAS_CALLBACK)) {
		_mem_debugf("User hasn't called `mem_register_exit_callback()` "
			    "before calling `mem_alloc()`.\n");
		exit(EXIT_FAILURE);
	}

	/* First call of `mem_alloc()`. */
	if (!(flags & FLAG_IS_INIT))
		_mem_init();

	assert(sz);

	s = _mem_block_first_empty_get(file, line);
	p = malloc(sz);
	assert(p);
	s->ptr	= p;
	s->sz	= sz;
	s->file = file;
	s->line = line;

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
