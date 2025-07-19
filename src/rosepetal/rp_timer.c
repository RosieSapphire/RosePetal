
#include <time.h>
#include <sys/time.h>

#include "rosepetal.h"

/*
 * Timespec stuff from CSTD>=C99. Since we're using C89, we don't
 * have this, but I'd still like to have time in nanoseconds. :D
 */
#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC 1

#include <bits/types.h>
#include <bits/endian.h>
#include <bits/types/time_t.h>

/* POSIX.1b structure for a time value.  This is like a `struct timeval' but
   has nanoseconds instead of microseconds.  */
struct timespec
{
#ifdef __USE_TIME_BITS64
        __time64_t tv_sec;
#else /* __USE_TIME_BITS64 */
        __time_t tv_sec;
#endif /* __USE_TIME_BITS64 */
#if __WORDSIZE == 64 \
        || (defined __SYSCALL_WORDSIZE && __SYSCALL_WORDSIZE == 64) \
        || (__TIMESIZE == 32 && !defined __USE_TIME_BITS64)
        __syscall_slong_t tv_nsec;
#else /* __WORDSIZE == 64 */
# if __BYTE_ORDER == __BIG_ENDIAN
        int: 32;           /* Padding.  */
        long int tv_nsec;  /* Nanoseconds.  */
# else /* __BYTE_ORDER == __BIG_ENDIAN */
        long int tv_nsec;  /* Nanoseconds.  */
        int: 32;           /* Padding.  */
# endif /* __BYTE_ORDER == __BIG_ENDIAN */
#endif /* __WORDSIZE == 64 */
};

#endif /* _STRUCT_TIMESPEC */

/* Extern function for `clock_gettime()` in >= C99. */
extern int clock_gettime(__clockid_t clockid, struct timespec *tp);

/* Choose which clock type we want depending on whether we're paranoid. */
#ifdef BE_PARANOID
#define CLOCK_MONOTONIC_RAW 0x4 /* Ripped from POSIX */
#define RP_CLOCK_TYPE       CLOCK_MONOTONIC_RAW
#else
#define CLOCK_MONOTONIC 0x1 /* Also ripped from POSIX */
#define RP_CLOCK_TYPE   CLOCK_MONOTONIC
#endif

static rp_u64_t rp_timer_nsec_since_init;
static rp_bool_t rp_timer_is_init = RP_FALSE;

rp_bool_t rp_timer_init(void)
{
        if (rp_timer_is_init) {
                rp_debug_printf(RP_DBG_STREAM_WARNING,
                                "Trying to init timer subsystem "
                                "even though it is already active.\n", NULL);
                return RP_FALSE;
        }

        rp_timer_nsec_since_init = rp_timer_get_nsec_raw();
        rp_timer_is_init         = RP_TRUE;

        return RP_TRUE;
}

rp_u64_t rp_timer_get_nsec_raw(void)
{
        rp_u64_t ticks;
        struct timespec ts;

        clock_gettime(RP_CLOCK_TYPE, &ts);
        ticks = (ts.tv_sec * RP_NSEC_PER_SEC) + ts.tv_nsec;

        return ticks;
}

rp_u64_t rp_timer_get_nsec(void)
{
        return rp_timer_get_nsec_raw() - rp_timer_nsec_since_init;
}

rp_f64_t rp_timer_get_sec64(void)
{
        return (rp_f64_t)rp_timer_get_nsec() / (rp_f64_t)RP_NSEC_PER_SEC;
}

rp_f32_t rp_timer_get_sec(void)
{
        return (rp_f32_t)rp_timer_get_sec64();
}

rp_bool_t rp_timer_free(void)
{
        if (!rp_timer_is_init) {
                rp_debug_printf(RP_DBG_STREAM_WARNING,
                                "Trying to free timer subsystem "
                                "even though it is already freed.\n", NULL);
                return RP_FALSE;
        }

        rp_timer_is_init = RP_FALSE;

        return RP_TRUE;
}
