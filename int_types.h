#ifndef _INT_TYPES_H_
#define _INT_TYPES_H_

#define STATIC_ASSERT(_cond, _msg)                                             \
	extern char static_assert_fail_##_msg[(_cond) ? 1 : -1]

typedef unsigned char u8;
typedef signed char   s8;

typedef unsigned short u16;
typedef signed short   s16;

typedef unsigned int u32;
typedef signed int   s32;

#ifdef __WIN32__

/* Windows */
typedef unsigned long long u64;
typedef signed long long   s64;

#elif defined(__linux__) /* #ifdef __WIN32__ */

/* Linux */
#ifdef __x86_64__

/* 64-bit */
typedef unsigned long u64;
typedef signed long   s64;

#elif defined(__i386__) /* #ifdef __x86_64__ */

/* 32-bit */
typedef unsigned long long u64;
typedef signed long long   s64;

#else /* #ifdef __x86_64__ #elif defined(__i386__) */

/* Unsupported/Unknown Register Size */
#error "Machine is neither 32-bit nor 64-bit... the fuck?"

#endif /* #ifdef __x86_64__ #elif defined(__i386__) #else */

#else /* #ifdef __WIN32__ #elif defined(__linux__) */

/* Unsupported OS */
#error "Unsupported operating system"

#endif /* #ifdef __WIN32__ #elif defined(__linux__) #else */

STATIC_ASSERT(sizeof(u8) == 1, u8_must_be_1_bytes);
STATIC_ASSERT(sizeof(s8) == 1, s8_must_be_1_bytes);
STATIC_ASSERT(sizeof(u16) == 2, u16_must_be_2_bytes);
STATIC_ASSERT(sizeof(s16) == 2, s16_must_be_2_bytes);
STATIC_ASSERT(sizeof(u32) == 4, u32_must_be_4_bytes);
STATIC_ASSERT(sizeof(s32) == 4, s32_must_be_4_bytes);
STATIC_ASSERT(sizeof(u64) == 8, u64_must_be_8_bytes);
STATIC_ASSERT(sizeof(s64) == 8, s64_must_be_8_bytes);

#endif /* #ifndef _INT_TYPES_H_ */
