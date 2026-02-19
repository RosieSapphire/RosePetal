#ifndef ROSE_PETAL_TYPES_H
#define ROSE_PETAL_TYPES_H

/* First, static assertions */
#define assert_uchar   unsigned char
#define assert_ushort  unsigned short
#define assert_uint    unsigned int
#define assert_ulong   unsigned long
#define assert_schar   signed char
#define assert_sshort  signed short
#define assert_sint    signed int
#define assert_slong   signed long
#define assert_float32 float
#define assert_float64 double

#define RIT_STATIC_ASSERT_SIZE(_type, _expected) \
        typedef char A_##_type[sizeof(_type) == (_expected) ? 1 : -1]

RIT_STATIC_ASSERT_SIZE(assert_uchar, 1);
RIT_STATIC_ASSERT_SIZE(assert_ushort, 2);
RIT_STATIC_ASSERT_SIZE(assert_uint, 4);
RIT_STATIC_ASSERT_SIZE(assert_ulong, 8);
RIT_STATIC_ASSERT_SIZE(assert_schar, 1);
RIT_STATIC_ASSERT_SIZE(assert_sshort, 2);
RIT_STATIC_ASSERT_SIZE(assert_sint, 4);
RIT_STATIC_ASSERT_SIZE(assert_slong, 8);
RIT_STATIC_ASSERT_SIZE(assert_float32, 4);
RIT_STATIC_ASSERT_SIZE(assert_float64, 8);

#undef RIT_STATIC_ASSERT_SIZE
#undef assert_uchar
#undef assert_ushort
#undef assert_uint
#undef assert_ulong
#undef assert_schar
#undef assert_sshort
#undef assert_sint
#undef assert_slong
#undef assert_float32
#undef assert_float64

/* THEN the typedefs */
typedef unsigned char ru8;
typedef unsigned short ru16;
typedef unsigned int ru32;
typedef unsigned long ru64;

typedef signed char rs8;
typedef signed short rs16;
typedef signed int rs32;
typedef signed long rs64;

typedef float rf32;
typedef double rf64;

typedef ru8 rb8;
typedef ru32 rb32;

#define RB_FALSE 0
#define RB_TRUE  1

#endif /* #ifndef ROSE_PETAL_TYPES_H */
