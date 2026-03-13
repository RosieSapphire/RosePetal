#ifndef _RP_RANDOM_H_
#define _RP_RANDOM_H_

/*
 * This is the random number generator submodule of the Rose Petal library.
 * Most of it is honestly just helper functions for generating ranges and
 * shit, and I may add or remove to it as I see fit depending on whether
 * or not I even use half the stuff. Either way, it's just nice to have! :D
 *
 * TODO: Make the functions follow the `_*_internal()` pattern so
 *       that debugging information can be passed through to them.
 *
 * MACRO DEFINE LIST:
 *
 * RP_RANDOM_IMPLEMENTATION:
 *	This macro is required for all the function declarations to be
 *	accessible. Make sure you define it before including this file
 *	anywhere you want the code to be defined for linkage.
 *
 *	IMPORTANT: Make sure you only ever define this macro in
 *	           ONE C file that gets linked with any given
 *	           program, otherwise the linker will bitch at
 *	           you for a multiple definition thingy-majiggy.
 *
 *	NOTE: All the below macros assume this one is defined.
 *
 * RP_RANDOM_LOG:
 *	Prints to `stdout` whenever a random function is called to let
 *	the user know what function was called and what number it gave.
 *
 * RP_RANDOM_TEST:
 *	This defines the function definiton for testing the random module.
 *	This is only really used in the internal `test.c` file, but you
 *	can define it if you really wanna use it.
 *
 *	NOTE: All test functions in this library are static,
 *	      meaning that they can only be defined once, although
 *	      to be fair, you can only define the implementation once
 *	      anyway, so it doesn't really matter all that much.
 */

/* Macro parity */
#if defined(RP_RANDOM_TEST) && !defined(RP_RANDOM_IMPLEMENTATION)
#error "`RP_RANDOM_TEST` cannot be defined unless `RP_RANDOM_IMPLEMENTATION` also is, otherwise the function will simply not exist."
#endif /* #if defined(RP_RANDOM_TEST) && !defined(RP_RANDOM_IMPLEMENTATION) */

#include <stdint.h>

#include "rp_types.h"

RP_STATIC_ASSERT(RAND_MAX == INT32_MAX, RAND_MAX_must_equal_INT32_MAX);

/********************************
 * PUBLIC FUNCTION DECLARATIONS *
 ********************************/

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
extern u32 rp_random_seed(const u32 seed);

/*
 * Generates a random number from 0 to 4294967295
 */
extern u32 rp_random_u32(void);

/*
 * Generates a random unsigned 32-bit number between a lower and upper bound
 *
 * NOTE: The upper bound is exclusive, meaning if you do something
 * like `.lo = 10, .hi = 50`, you will get numbers from [10 -> 49].
 */
extern u32 rp_random_u32_range(const u32 lo, const u32 hi);

/*
 * Generates a random number from 0 to 2147483647
 */
extern s32 rp_random_s32(void);

/*
 * Generates a random signed 32-bit number between a lower and upper bound
 *
 * NOTE: The upper bound is exclusive, meaning if you do something
 * like `.lo = -10, .hi = 10`, you will get numbers from [-10 -> 9].
 */
extern s32 rp_random_s32_range(const s32 lo, const s32 hi);

/*
 * Generates a random number from 0 to 65535
 */
extern u16 rp_random_u16(void);

/*
 * Generates a random number from -32768 to 32767
 */
extern s16 rp_random_s16(void);

/*
 * Generates a random number from 0 to 255
 */
extern u8 rp_random_u8(void);

/*
 * Generates a random number from -128 to 127
 */
extern s8 rp_random_s8(void);

/*
 * Generates a random boolean with a 50% chance for either TRUE or FALSE
 */
extern bool_t rp_random_bool_50_percent(void);

/*
 * Generates a floating point number from 0 to 1
 *
 * It can be scaled with a magnitude, but just use `1.f` for default.
 */
extern f32 rp_random_f32_zo(const f32 mag);

/*
 * Generates a floating point number from -1 to 1
 *
 * It can be scaled with a magnitude, but just use `1.f` for default.
 */
extern f32 rp_random_f32_no(const f32 mag);

/*******************************
 * PUBLIC FUNCTION DEFINITIONS *
 *******************************/

#ifdef RP_RANDOM_IMPLEMENTATION

#include <time.h>

#ifdef rnd_debugf
#error "`rnd_debugf()` is already defined elsewhere!"
#endif /* #ifdef rnd_debugf */

#ifdef RP_RANDOM_LOG
#define rnd_debugf(...) printf(__VA_ARGS__)
#else /* #ifdef RP_RANDOM_LOG */
#define rnd_debugf(...) ((void)0)
#endif /* #ifdef RP_RANDOM_LOG #else */

u32 rp_random_seed(const u32 seed)
{
	const u32 s = (seed == UINT32_MAX) ? (u32)time(NULL) : (u32)seed;

	srand(s);
	rnd_debugf("RNG: rp_random_seed(%u) -> %u\n", seed, s);

	return s;
}

u32 rp_random_u32(void)
{
	const u32 r = (((u32)rand() & 0xFFFF) << 0u) |
		      (((u32)rand() & 0xFFFF) << 16u);

	rnd_debugf("rp_random_u32() -> %u\n", r);

	return r;
}

u32 rp_random_u32_range(const u32 lo, const u32 hi)
{
	u32 r;

	rp_assertf(hi > lo,
		   "rp_random_u32_range(): High must be "
		   "greater than low [.lo = %u, .hi = %u]",
		   lo,
		   hi);

	r = (rp_random_u32() % (hi - lo)) + lo;
	rnd_debugf("rp_random_u32_range(lo:%u, hi:%u) -> %u\n", lo, hi, r);

	return r;
}

s32 rp_random_s32(void)
{
	const s32 r = (s32)rp_random_u32();

	rnd_debugf("rp_random_s32() -> %d\n", r);

	return r;
}

s32 rp_random_s32_range(const s32 lo, const s32 hi)
{
	s64 t;
	s32 r;

	rp_assertf(hi > lo,
		   "rp_random_s32_range(): High must be "
		   "greater than low [.lo = %u, .hi = %u]",
		   lo,
		   hi);

	t = (s64)(rp_random_u32() % (u64)((s64)hi - (s64)lo)) + (s64)lo;

	rp_assertf(t == (s32)t,
		   "Conversion error when down-casting `t` from %lu to %d",
		   t,
		   (s32)t);

	r = (s32)t;
	rnd_debugf("rp_random_s32_range(lo:%d, hi:%d) -> %d\n", lo, hi, r);

	return r;
}

u16 rp_random_u16(void)
{
	const u16 r = (u16)(rp_random_u32() & 0xFFFF);

	rnd_debugf("rp_random_u16() -> %u\n", r);

	return r;
}

s16 rp_random_s16(void)
{
	const s16 r = (s16)(rp_random_u32() & 0xFFFF);

	rnd_debugf("rp_random_s16() -> %d\n", r);

	return r;
}

u8 rp_random_u8(void)
{
	const u8 r = (u8)(rp_random_u32() & 0xFF);

	rnd_debugf("rp_random_u8() -> %u\n", r);

	return r;
}

s8 rp_random_s8(void)
{
	const s8 r = (s8)(rp_random_u32() & 0xFF);

	rnd_debugf("rp_random_u8() -> %d\n", r);

	return r;
}

bool_t rp_random_bool_50_percent(void)
{
	const bool_t r = (bool_t)(rand() & 1);

	rnd_debugf("rp_random_bool_50_percent() -> %5s\n", r ? "TRUE" : "FALSE");

	return r;
}

f32 rp_random_f32_zo(const f32 mag)
{
	const f32 r = mag * ((f32)rand() / (f32)RAND_MAX);

	rnd_debugf("rp_random_f32_zo(mag:%f) -> %f\n", (f64)mag, (f64)r);

	return r;
}

f32 rp_random_f32_no(const f32 mag)
{
	const f32 r = mag * (((f32)rand() / (f32)(RAND_MAX >> 1)) - 1.f);

	rnd_debugf("rp_random_f32_no(mag:%f) -> %f\n", (f64)mag, (f64)r);

	return r;
}

#ifdef RP_RANDOM_TEST

static void rp_random_test(void)
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

#endif /* #ifdef RP_RANDOM_IMPLEMENTATION */

#endif /* #ifndef _RP_RANDOM_H_ */
