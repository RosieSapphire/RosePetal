#ifndef _RP_RANDOM_H_
#define _RP_RANDOM_H_

/*
 * This is the random number generator submodule of the Rose Petal library.
 * Most of it is honestly just helper functions for generating ranges and
 * shit, and I may add or remove to it as I see fit depending on whether
 * or not I even use half the stuff. Either way, it's just nice to have! :D
 */

#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "rp_types.h"

RP_STATIC_ASSERT(RAND_MAX == INT32_MAX, RAND_MAX_must_equal_INT32_MAX);

#ifdef assertf
#error "The \"assertf\" macro is already defined elsewhere!"
#endif /* #ifdef assertf */

#ifdef RP_RANDOM_LOG
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#endif /* #ifdef RP_RANDOM_LOG */

static __inline void _assertf_internal(const bool_t cond,
				       const char  *cond_str,
				       const char  *file,
				       const int    line,
				       const char  *func,
				       const char  *fmt,
				       ...)
{
#ifdef RP_RANDOM_LOG
	va_list args;

	if (cond)
		return;

	va_start(args, fmt);
	fprintf(stderr,
		"RANDOM: ASSERTION (%s) @ %s:%d in %s FAILED: ",
		cond_str,
		strrchr(file, '/') ? strrchr(file, '/') + 1 : file,
		line,
		func);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
#else  /* #ifdef RP_RANDOM_LOG */
	(void)cond;
	(void)cond_str;
	(void)file;
	(void)line;
	(void)func;
	(void)fmt;
#endif /* #ifdef RP_RANDOM_LOG #else */
}

#ifdef RP_RANDOM_LOG
#define assertf(_cond, _fmt, ...)                                              \
	_assertf_internal(_cond,                                               \
			  #_cond,                                              \
			  __FILE__,                                            \
			  __LINE__,                                            \
			  __func__,                                            \
			  _fmt,                                                \
			  __VA_ARGS__)
#else /* #ifdef RP_RANDOM_LOG */
#define assertf(_cond, _fmt, ...)                                              \
	do {                                                                   \
		(void)(_cond);                                                 \
		(void)(_fmt);                                                  \
	} while (0)
#endif /* #ifdef RP_RANDOM_LOG #else */

/*
 * You can seed the randomness with a custom value, or
 * just pass `UINT32_MAX` for it to use `time(NULL)`, which will
 * use the current Unix time that the function was called.
 *
 * It's a little jank as it requires `<limits.h>` to be included
 * and it makes it so you aren't able to pass in UINT32_MAX as
 * an actual seed value, but it should be fine enough for this.
 *
 * Returns the seed if the input wasn't `UINT32_MAX`.
 */
static __inline u32 rp_random_seed(const u32 seed)
{
	const u32 s = (seed == UINT32_MAX) ? (u32)time(NULL) : (u32)seed;

	srand(s);

	return s;
}

/*
 * Generates a random number from 0 to 4294967295
 */
static __inline u32 rp_random_u32(void)
{
	return (((u32)rand() & 0xFFFF) << 0u) | (((u32)rand() & 0xFFFF) << 16u);
}

/*
 * Generates a random unsigned 32-bit number between a lower and upper bound
 *
 * NOTE: The upper bound is exclusive, meaning if you do something
 * like `.lo = 10, .hi = 50`, you will get numbers from [10 -> 49].
 */
static __inline u32 rp_random_u32_range(const u32 lo, const u32 hi)
{
	assertf(hi > lo,
		"rp_random_u32_range(): High must be "
		"greater than low [.lo = %u, .hi = %u]",
		lo,
		hi);
	return (rp_random_u32() % (hi - lo)) + lo;
}

/*
 * Generates a random number from 0 to 2147483647
 */
static __inline s32 rp_random_s32(void)
{
	return (s32)rp_random_u32();
}

/*
 * Generates a random signed 32-bit number between a lower and upper bound
 *
 * NOTE: The upper bound is exclusive, meaning if you do something
 * like `.lo = -10, .hi = 10`, you will get numbers from [-10 -> 9].
 */
static __inline s32 rp_random_s32_range(const s32 lo, const s32 hi)
{
	s64 t;

	assertf(hi > lo,
		"rp_random_s32_range(): High must be "
		"greater than low [.lo = %u, .hi = %u]",
		lo,
		hi);

	t = (s64)(rp_random_u32() % (u64)((s64)hi - (s64)lo)) + (s64)lo;

	assertf(t == (s32)t,
		"Conversion error when down-casting `t` from %lu to %d",
		t,
		(s32)t);

	return (s32)t;
}

/*
 * Generates a random number from 0 to 65535
 */
static __inline u16 rp_random_u16(void)
{
	return (u16)(rp_random_u32() & 0xFFFF);
}

/*
 * Generates a random number from -32768 to 32767
 */
static __inline s16 rp_random_s16(void)
{
	return (s16)(rp_random_u32() & 0xFFFF);
}

/*
 * Generates a random number from 0 to 255
 */
static __inline u8 rp_random_u8(void)
{
	return (u8)(rp_random_u32() & 0xFF);
}

/*
 * Generates a random number from -128 to 127
 */
static __inline s8 rp_random_s8(void)
{
	return (s8)(rp_random_u32() & 0xFF);
}

/*
 * Generates a random boolean with a 50% chance for either TRUE or FALSE
 */
static __inline bool_t rp_random_bool_50_percent(void)
{
	return (bool_t)(rand() & 1);
}

/*
 * Generates a floating point number from 0 to 1
 *
 * It can be scaled with a magnitude, but just use `1.f` for default.
 */
static __inline f32 rp_random_f32_zo(const f32 mag)
{
	return mag * ((f32)rand() / (f32)RAND_MAX);
}

/*
 * Generates a floating point number from -1 to 1
 *
 * It can be scaled with a magnitude, but just use `1.f` for default.
 */
static __inline f32 rp_random_f32_no(const f32 mag)
{
	return mag * (((f32)rand() / (f32)(RAND_MAX >> 1)) - 1.f);
}

#ifdef RP_RANDOM_TEST
#include <stdio.h>

static __inline void rp_random_test(void)
{
	s32 s32_min, s32_max;
	u32 u32_min, u32_max;

	rp_random_seed(UINT32_MAX);

	s32_min = rp_random_s32();
	do {
		s32_max = rp_random_s32();
	} while (s32_max <= s32_min);

	u32_min = rp_random_u32();
	do {
		u32_max = rp_random_u32();
	} while (u32_max <= u32_min);

	printf("\n[TEST] rp_random.h:\n\n");
	printf("S32: %d\n", rp_random_s32());
	printf("U32: %u\n", rp_random_u32());
	printf("S16: %d\n", rp_random_s16());
	printf("U16: %u\n", rp_random_u16());
	printf("S8: %d\n", rp_random_s8());
	printf("U8: %u\n", rp_random_u8());
	printf("S32 [%d -> %d]: %d\n",
	       s32_min,
	       s32_max,
	       rp_random_s32_range(s32_min, s32_max));
	printf("U32 [%u -> %u]: %u\n",
	       u32_min,
	       u32_max,
	       rp_random_u32_range(u32_min, u32_max));
	printf("F32 [0 -> 1]: %f\n", (f64)rp_random_f32_zo(1.f));
	printf("F32 [-1 -> 1]: %f\n", (f64)rp_random_f32_no(1.f));
	printf("F32 [ 0 -> 4096]: %f\n", (f64)rp_random_f32_zo(4096.f));
	printf("F32 [-4096 -> 4096]: %f\n", (f64)rp_random_f32_no(4096.f));
}
#endif /* #ifdef RP_RANDOM_TEST */

#endif /* #ifndef _RP_RANDOM_H_ */
