#ifndef _RP_TYPES_H_
#define _RP_TYPES_H_

#include <stdint.h>

#include "rp_assert.h"

typedef uint8_t u8;
typedef int8_t s8;

typedef uint16_t u16;
typedef int16_t s16;

typedef uint32_t u32;
typedef int32_t s32;

typedef uint64_t u64;
typedef int64_t s64;

typedef float  f32;
typedef double f64;

/* Boolean */
#ifdef FALSE
#error "\"FALSE\" is already defined!"
#endif /* #ifdef FALSE */

#ifdef TRUE
#error "\"TRUE\" is already defined!"
#endif /* #ifdef TRUE */

#define FALSE 0
#define TRUE  1

typedef unsigned char bool_t;

RP_STATIC_ASSERT(sizeof(bool_t) == 1, bool_t_must_be_1_bytes);
RP_STATIC_ASSERT(sizeof(u8) == 1, u8_must_be_1_bytes);
RP_STATIC_ASSERT(sizeof(s8) == 1, s8_must_be_1_bytes);
RP_STATIC_ASSERT(sizeof(u16) == 2, u16_must_be_2_bytes);
RP_STATIC_ASSERT(sizeof(s16) == 2, s16_must_be_2_bytes);
RP_STATIC_ASSERT(sizeof(u32) == 4, u32_must_be_4_bytes);
RP_STATIC_ASSERT(sizeof(s32) == 4, s32_must_be_4_bytes);
RP_STATIC_ASSERT(sizeof(f32) == 4, f32_must_be_4_bytes);
RP_STATIC_ASSERT(sizeof(u64) == 8, u64_must_be_8_bytes);
RP_STATIC_ASSERT(sizeof(s64) == 8, s64_must_be_8_bytes);
RP_STATIC_ASSERT(sizeof(f64) == 8, f64_must_be_4_bytes);

#endif /* #ifndef _RP_TYPES_H_ */
