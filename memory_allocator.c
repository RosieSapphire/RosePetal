#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include "memory_allocator.h"
#include "boolean.h"

/****************************************************
 * TODO:                                            *
 * > Implement file and line debugging              *
 * > Implement a free-list for double-free checking *
 * > Make this into a header-only library.          *
 *   Not ideal, obviously, but extremely portable.  *
 ****************************************************/

#undef mem_init
#undef mem_alloc
#undef mem_free
#undef mem_terminate

#ifdef MEMORY_ALLOCATOR_WRAP_STDLIB
#undef malloc
#undef free
#endif /* #ifdef MEMORY_ALLOCATOR_WRAP_STDLIB */

enum mem_verify_result {
	MVR_INVALID_RESULT = -6,
	MVR_FAIL_0_SLOTS_CNT,
	MVR_FAIL_INDEX_TOO_HIGH,
	MVR_FAIL_NULL_SLOTS_ARR,
	MVR_FAIL_PTR_NULL_WITH_SIZE,
	MVR_FAIL_PTR_NON_NULL_NO_SIZE,
	MVR_SUCCESS
};

struct block {
	void  *ptr;
	size_t sz;
};

static struct memory {
	struct block *arr;
	size_t	      cnt;
} memory;

static bool_t is_init = FALSE;

/*********************
 * PRIVATE FUNCTIONS *
 *********************/

static void _mem_debugf(const char *fmt, ...)
{
#ifdef ALLOCATOR_DEBUG
	va_list args;

	va_start(args, fmt);
	(void)fprintf(stderr, "ALLOCATOR: ");
	(void)vfprintf(stderr, fmt, args);
	va_end(args);
#else  /* #ifdef ALLOCATOR_DEBUG */
	(void)fmt;
#endif /* #ifdef ALLOCATOR_DEBUG #else */
}

static struct block *
_mem_block_get(const size_t i, const char *file, const int line)
{
	assert(memory.cnt);
	assert(memory.arr);
	assert(i < memory.cnt);

#ifdef ALLOCATOR_DEBUG_VERBOSE
	_mem_debugf("Got block at index %lu @ %s:%d\n", file, line);
#else  /* #ifdef ALLOCATOR_DEBUG_VERBOSE */
	(void)file;
	(void)line;
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE #else */

	return memory.arr + i;
}

static struct block *_mem_block_get_last(const char *file, const int line)
{
	assert(memory.cnt);
	assert(memory.arr);

	return _mem_block_get(memory.cnt - 1, file, line);
}

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
	_mem_debugf("Verifying Allocator <%p>'s slot %lu\n", ind);
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE */

	if (!memory.cnt)
		GOTO_END(MVR_FAIL_0_SLOTS_CNT);

	if (ind >= memory.cnt)
		GOTO_END(MVR_FAIL_INDEX_TOO_HIGH);

	if (!memory.arr)
		GOTO_END(MVR_FAIL_NULL_SLOTS_ARR);

	s = _mem_block_get(ind, file, line);

	/* If our pointer is NULL, the rest of our data better match! */
	if (!s->ptr) {
		if (!s->sz)
			GOTO_END(MVR_SUCCESS);

		GOTO_END(MVR_FAIL_PTR_NULL_WITH_SIZE);
	}

	/* Same for if it's non-NULL, just in the other direction. */
	if (!s->sz)
		GOTO_END(MVR_FAIL_PTR_NON_NULL_NO_SIZE);

	GOTO_END(MVR_SUCCESS);

mem_verify_end:
#ifdef ALLOCATOR_DEBUG_VERBOSE
	if (r == MVR_SUCCESS)
		_mem_debugf("Memory slot %lu is valid! %s:%d\n", file, line);
#else  /* #ifdef ALLOCATOR_DEBUG_VERBOSE */
	(void)file;
	(void)line;
#endif /* #ifdef ALLOCATOR_DEBUG_VERBOSE #else */

	return r;

#undef GOTO_END
}

static void __attribute__((noreturn))
_mem_block_error_exit(const enum mem_verify_result r,
		      const size_t		   i,
		      const char		  *file,
		      const int			   line)
{
#define MSG_ARR_LEN	     (-MVR_INVALID_RESULT + 1)
#define MVR_TO_MSG_INDEX(_r) ((size_t)(_r + MSG_ARR_LEN - 1))

	const char *msg_arr[MSG_ARR_LEN] = {
		"<INVALID RESULT>",
		"Allocator has 0 slots",
		"Index is too high",
		"Allocator slots array is NULL",
		"Has NULL pointer, but has >= 0 size",
		"Pointer is non-NULL, but size is 0",
		"Success"
	};
	const char *msg = NULL;

	assert(r >= MVR_INVALID_RESULT);
	assert(r <= MVR_SUCCESS);

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

static struct block *_mem_block_first_empty_get(const char *file,
						const int   line)
{
	struct block	      *s = NULL;
	enum mem_verify_result r = MVR_INVALID_RESULT;
	size_t		       i, newsz;

	/* Search in the already-existing array for a free slot. */
	for (i = 0ul; i < memory.cnt; ++i) {
		s = _mem_block_get(i, file, line);
		r = _mem_block_verify(i, file, line);
		if (r != MVR_SUCCESS)
			_mem_block_error_exit(r, i, file, line);

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

static bool_t block_is_child_of_allocator(const struct block *s,
					  const char	     *file,
					  const int	      line)
{
	size_t i;

	assert(memory.cnt);
	assert(memory.arr);
	assert(s);

	for (i = 0ul; i < memory.cnt; ++i) {
		const struct block *x = _mem_block_get(i, file, line);

		if (x == s)
			return TRUE;
	}

	return FALSE;
}

static size_t
_mem_block_get_index(const struct block *s, const char *file, const int line)
{
	assert(memory.cnt);
	assert(memory.arr);
	assert(s);

	if (!block_is_child_of_allocator(s, file, line)) {
		_mem_debugf("Memory slot <%p> is not a child of allocator",
			    (const void *)s);
		exit(EXIT_FAILURE);
	}

	return (size_t)(s - memory.arr);
}

static struct block *
_mem_block_from_pointer(const void *ptr, const char *file, const int line)
{
	size_t i;

	assert(memory.cnt && memory.arr);

	for (i = 0ul; i < memory.cnt; ++i) {
		struct block	      *s = _mem_block_get(i, file, line);
		enum mem_verify_result r = MVR_INVALID_RESULT;

		r = _mem_block_verify(i, file, line);
		if (r != MVR_SUCCESS)
			_mem_block_error_exit(r, i, file, line);

		if (s->ptr == ptr)
			return s;
	}

	_mem_debugf("Failed to get memory slot from pointer <%p>\n", ptr);
	exit(EXIT_FAILURE);
}

static void
_mem_block_pointer_free(struct block *s, const char *file, const int line)
{
	assert(s);
	assert(s->ptr);
	assert(s->sz);

	_mem_debugf("Freed pointer <%p> of size %lu at %s:%d\n",
		    s->ptr,
		    s->sz,
		    file,
		    line);

	free(s->ptr);
	s->ptr = NULL;
	s->sz  = 0ul;
}

/********************
 * PUBLIC FUNCTIONS *
 ********************/

void _mem_init_internal(const char *file, const int line)
{
	assert(!memory.cnt);
	assert(!memory.arr);
	assert(!is_init);
	is_init = TRUE;
	_mem_debugf("INITIALIZED SUCCESSFULLY @ %s:%d.\n", file, line);
}

void *_mem_alloc_internal(const size_t sz, const char *file, const int line)
{
	struct block *s = NULL;
	void	     *p = NULL;

	assert(is_init);

	assert(sz);

	s = _mem_block_first_empty_get(file, line);
	p = malloc(sz);
	assert(p);
	s->ptr = p;
	s->sz  = sz;

	_mem_debugf("Allocated <%p> of size %lu to slot %lu at %s:%d\n",
		    p,
		    sz,
		    _mem_block_get_index(s, file, line),
		    file,
		    line);

	return p;
}

void _mem_free_internal(void *ptr, const char *file, const int line)
{
	struct block *s;

	assert(is_init);

	assert(ptr);

	s = _mem_block_from_pointer(ptr, file, line);
	assert(s);
	assert(block_is_child_of_allocator(s, file, line));

	_mem_block_pointer_free(s, file, line);
}

void _mem_terminate_internal(const char *file, const int line)
{
	size_t i;

	assert(is_init);

	assert(file);
	assert(line >= 0);

	for (i = 0ul; i < memory.cnt; ++i) {
		struct block		    *s = _mem_block_get(i, file, line);
		const enum mem_verify_result r =
				_mem_block_verify(i, file, line);

		if (r != MVR_SUCCESS)
			_mem_block_error_exit(r, i, file, line);

		if (!s->ptr)
			continue;

		_mem_debugf("LEAKED POINTER <%p> of size %lu "
			    "allocated to slot %lu at %s:%lu\n",
			    s->ptr,
			    s->sz,
			    _mem_block_get_index(s, file, line),
			    "TODO",
			    69);
		_mem_block_pointer_free(s, file, line);
	}

	is_init = FALSE;

	_mem_debugf("TERMINATED SUCCESSFULLY @ %s:%d.\n", file, line);
}
