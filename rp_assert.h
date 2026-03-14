#ifndef _RP_ASSERT_H_
#define _RP_ASSERT_H_

/*
 * Check for previous macro definitions
 */
#ifdef RP_STATIC_ASSERT
#error "The `RP_STATIC_ASSERT` macro was already defined elsewhere!"
#endif /* #ifndef RP_STATIC_ASSERT */

#ifdef rp_assertf
#error "The `rp_assertf` macro is already defined elsewhere!"
#endif /* #ifdef rp_assertf */

#ifdef rp_assertf_ex
#error "The `rp_assertf_ex` macro is already defined elsewhere!"
#endif /* #ifdef rp_assertf_ex */

#define RP_STATIC_ASSERT(_cond, _msg)                                          \
	extern char rp_static_assert_fail_##_msg[(_cond) ? 1 : -1]

/*
 * Check if debug mode is enabled or not
 */
#if defined(_DEBUG) && !defined(NDEBUG)
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static __inline void _rp_assertf_internal(const int   cond,
					  const char *cond_str,
					  const char *file,
					  const int   line,
					  const char *func,
					  const char *fmt,
					  ...)
{
	va_list args;

	if (cond)
		return;

	va_start(args, fmt);
	fprintf(stderr,
		"ASSERTION (%s) @ %s:%d in %s FAILED: \n\t",
		cond_str,
		strrchr(file, '/') ? strrchr(file, '/') + 1 : file,
		line,
		func);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}

#define rp_assertf(_cond, ...)                                                 \
	_rp_assertf_internal(!!(_cond),                                        \
			     #_cond,                                           \
			     __FILE__,                                         \
			     __LINE__,                                         \
			     __func__,                                         \
			     __VA_ARGS__)
#define rp_assertf_ex(_cond, _file, _line, ...)                                \
	_rp_assertf_internal(!!(_cond),                                        \
			     #_cond,                                           \
			     _file,                                            \
			     _line,                                            \
			     __func__,                                         \
			     __VA_ARGS__)
#else /* #if defined(_DEBUG) && !defined(NDEBUG) */
#define rp_assertf(_cond, ...) (void)(_cond)
#define rp_assertf_ex(_cond, _file, _line, ...)                                \
	do {                                                                   \
		(void)(_cond);                                                 \
		(void)(_file);                                                 \
		(void)(_line);                                                 \
	} while (0)
#endif /* #if defined(_DEBUG) && !defined(NDEBUG) #else */

#endif /* #ifndef _RP_ASSERT_H_ */
