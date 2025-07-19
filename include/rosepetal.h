#pragma once

#include <stdio.h>

/* General Defines */
#define RP_NSEC_PER_SEC 1000000000L

#ifdef M_PIf
#define RP_PIf M_PIf
#else /* M_PIf */
#define RP_PIf 3.1415927410125732421875f
#endif /* M_PIf */

#define RP_DEG_TO_RAD(D) (D *  0.01745329300562540690f)
#define RP_RAD_TO_DEG(R) (R * 57.29577791868204673438f)

/* General Types */
typedef unsigned char  rp_u8_t;
typedef unsigned short rp_u16_t;
typedef unsigned int   rp_u32_t;
typedef unsigned long  rp_u64_t;
typedef signed char    rp_s8_t;
typedef signed short   rp_s16_t;
typedef signed int     rp_s32_t;
typedef signed long    rp_s64_t;
typedef float          rp_f32_t;
typedef double         rp_f64_t;

typedef rp_u8_t        rp_bool_t;
enum { RP_FALSE, RP_TRUE };

/************
 * DEBUGGER *
 ************/

/* Macros */
#define RP_DEBUG_INIT_DEFAULT stdout, stdout, stderr

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#ifdef USE_DEBUGGER
#define rp_debugf(STREAM, FMT, ...) \
        rp_debugf_ex(__FILE__, __LINE__, STREAM, FMT, __VA_ARGS__)
#else
#define rp_debugf(STREAM, FMT, ...)
#endif
#pragma GCC diagnostic pop

/* Enums */
enum rp_deb_stream {
        RP_DBG_STREAM_INFO,
        RP_DBG_STREAM_WARNING,
        RP_DBG_STREAM_ERROR,
        RP_DBG_STREAM_CNT
};

/* Functions */
rp_bool_t rp_debug_init(FILE *info_stream,
                        FILE *warning_stream,
                        FILE *error_stream);
rp_bool_t rp_debugf_ex(const char *file, const int line,
                       const enum rp_deb_stream stream,
                       const char *fmt, ...);
rp_bool_t rp_debug_free(void);

/*********
 * TIMER *
 *********/

/* Functions */
rp_bool_t rp_timer_init(void);
rp_u64_t rp_timer_get_nsec_raw(void);
rp_u64_t rp_timer_get_nsec(void);
rp_f64_t rp_timer_get_sec64(void);
rp_f32_t rp_timer_get_sec(void);
rp_bool_t rp_timer_free(void);
