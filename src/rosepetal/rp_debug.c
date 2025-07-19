#ifdef USE_DEBUGGER

#include <stdio.h>
#include <stdarg.h>

#include "rosepetal.h"

#define RP_DBG_ARG

#define DBG_STREAM_INFO_FALLBACK    stdout
#define DBG_STREAM_WARNING_FALLBACK stdout
#define DBG_STREAM_ERROR_FALLBACK   stderr

static FILE     *rp_debug_streams[RP_DBG_STREAM_CNT];
static rp_bool_t rp_debug_is_init = RP_FALSE;

rp_bool_t rp_debug_init(RP_DBG_ARG FILE *info_stream,
                        RP_DBG_ARG FILE *warning_stream,
                        RP_DBG_ARG FILE *error_stream)
{
        if (rp_debug_is_init) {
                fprintf(stderr, "ERROR: Trying to call `rp_debug_init()` "
                                "when it's already initialized.\n");
                return RP_FALSE;
        }

        rp_debug_streams[RP_DBG_STREAM_INFO]    = info_stream;
        rp_debug_streams[RP_DBG_STREAM_WARNING] = warning_stream;
        rp_debug_streams[RP_DBG_STREAM_ERROR]   = error_stream;
        rp_debug_is_init                        = RP_TRUE;

        return RP_TRUE;
}

rp_bool_t rp_debugf_ex(RP_DBG_ARG const char *file,
                       RP_DBG_ARG const int line,
                       RP_DBG_ARG const enum rp_deb_stream stream,
                       RP_DBG_ARG const char *fmt, ...)
{
        va_list args;
        FILE *strm;
        rp_s32_t stat;
        const char *strm_prefix[RP_DBG_STREAM_CNT] = {
                "\x1b[0;36mINFO   \x1b[0m",
                "\x1b[0;33mWARNING\x1b[0m",
                "\x1b[0;31mERROR  \x1b[0m"
        };

        if (!rp_debug_is_init) {
                fprintf(DBG_STREAM_ERROR_FALLBACK,
                                "ERROR: Rose Petal's debug module has not been"
                                " initialized. Make sure you've called "
                                "`rp_debug_init()` before trying to use the "
                                "`rp_debugf()` function. (%s:%d)\n",
                        file, line);
                return RP_FALSE;
        }

        strm = rp_debug_streams[stream];
        if (!strm) {
                fprintf(DBG_STREAM_ERROR_FALLBACK,
                        "`rp_debugf()`: Trying to "
                        "print to invalid steam.\n");
                return RP_FALSE;
        }

        stat = fprintf(strm, "%s %s:%d : ", strm_prefix[stream], file, line);
        if (!stat) {
                fprintf(DBG_STREAM_ERROR_FALLBACK,
                        "`rp_debugf()`: Failed to run "
                        "`fprintf()` error_code=%d.\n", stat);
                return RP_FALSE;
        }

        va_start(args, fmt);

        stat = vfprintf(strm, fmt, args);
        if (!stat) {
                fprintf(DBG_STREAM_ERROR_FALLBACK,
                        "`rp_debugf()`: Failed to run "
                        "`vfprintf()` error_code=%d.\n", stat);
                return RP_FALSE;
        }

        va_end(args);

        return RP_TRUE;
}

rp_bool_t rp_debug_free(void)
{
        rp_u8_t i;

        if (!rp_debug_is_init) {
                fprintf(stderr, "ERROR: Trying to call `rp_debug_free()` "
                                "when it's already freed.\n");
                return RP_FALSE;
        }

        for (i = 0; i < RP_DBG_STREAM_CNT; ++i)
                rp_debug_streams[i] = NULL;

        rp_debug_is_init = RP_FALSE;

        return RP_TRUE;
}

#else /* USE_DEBUGGER */

#include "rosepetal.h"

#define RP_DBG_ARG __attribute__((unused))

rp_bool_t rp_debug_init(RP_DBG_ARG FILE *info_stream,
                        RP_DBG_ARG FILE *warning_stream,
                        RP_DBG_ARG FILE *error_stream)
{
        return RP_TRUE;
}

rp_bool_t rp_debugf_ex(RP_DBG_ARG const char *file,
                       RP_DBG_ARG const int line,
                       RP_DBG_ARG const enum rp_deb_stream stream,
                       RP_DBG_ARG const char *fmt, ...)
{
        return RP_TRUE;
}

rp_bool_t rp_debug_free(void)
{
        return RP_TRUE;
}

#endif /* USE_DEBUGGER */
