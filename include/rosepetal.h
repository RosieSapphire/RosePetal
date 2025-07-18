#pragma once

#include <stdio.h>

/* General Types */
typedef unsigned char  rp_u8_t;
typedef unsigned short rp_u16_t;
typedef unsigned int   rp_u32_t;
typedef unsigned long  rp_u64_t;
typedef signed char    rp_s8_t;
typedef signed short   rp_s16_t;
typedef signed int     rp_s32_t;
typedef float          rp_f32_t;
typedef double         rp_f64_t;
typedef int            rp_int_t;

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
#define rp_debug_printf(STREAM, FMT, ...) \
        rp_debug_printf_ex(__FILE__, __LINE__, STREAM, FMT, __VA_ARGS__)
#else
#define rp_debug_printf(STREAM, FMT, ...)
#endif
#pragma GCC diagnostic pop

/* Enums */
enum rp_deb_stream {
        RP_DEB_STREAM_INFO,
        RP_DEB_STREAM_WARNING,
        RP_DEB_STREAM_ERROR,
        RP_DEB_STREAM_CNT
};

/* Functions */
rp_bool_t rp_debug_init(FILE *info_stream,
                        FILE *warning_stream,
                        FILE *error_stream);
rp_bool_t rp_debug_printf_ex(const char *file, const int line,
                             const enum rp_deb_stream stream,
                             const char *fmt, ...);
rp_bool_t rp_debug_free(void);

/*********
 * TIMER *
 *********/

/* Functions */
rp_bool_t rp_timer_init(void);
rp_bool_t rp_timer_free(void);
