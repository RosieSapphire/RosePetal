#include <stdio.h>
#include <stdarg.h>

#include "rosepetal.h"

#define DEB_STREAM_INFO_FALLBACK    stdout
#define DEB_STREAM_WARNING_FALLBACK stdout
#define DEB_STREAM_ERROR_FALLBACK   stderr

static FILE     *debug_streams[RP_DEB_STREAM_CNT];
static rp_bool_t debug_is_init = RP_FALSE;

rp_bool_t rp_debug_init(FILE *info_stream,
                        FILE *warning_stream,
                        FILE *error_stream)
{
        if (debug_is_init) {
                fprintf(stderr, "ERROR: Trying to call `rp_debug_init()` "
                                "when it's already initialized.\n");
                return RP_FALSE;
        }

        debug_streams[RP_DEB_STREAM_INFO]    = info_stream;
        debug_streams[RP_DEB_STREAM_WARNING] = warning_stream;
        debug_streams[RP_DEB_STREAM_ERROR]   = error_stream;
        debug_is_init                        = RP_TRUE;

        return RP_TRUE;
}

rp_bool_t rp_debug_printf(const enum rp_deb_stream stream, const char *fmt, ...)
{
        va_list args;
        FILE *strm;
        rp_int_t stat;
        const char *strm_prefix[RP_DEB_STREAM_CNT] = {
                "\x1b[0;36mINFO:    \x1b[0m",
                "\x1b[0;33mWARNING: \x1b[0m",
                "\x1b[0;31mERROR:   \x1b[0m"
        };

        strm = debug_streams[stream];
        if (!strm) {
                fprintf(DEB_STREAM_ERROR_FALLBACK,
                        "`rp_debug_printf()`: Trying to "
                        "print to invalid steam.\n");
                return RP_FALSE;
        }

        stat = fprintf(strm, "%s", strm_prefix[stream]);
        if (!stat) {
                fprintf(DEB_STREAM_ERROR_FALLBACK,
                        "`rp_debug_printf()`: Failed to run "
                        "`fprintf()` error_code=%d.\n", stat);
                return RP_FALSE;
        }

        va_start(args, fmt);

        stat = vfprintf(strm, fmt, args);
        if (!stat) {
                fprintf(DEB_STREAM_ERROR_FALLBACK,
                        "`rp_debug_printf()`: Failed to run "
                        "`vfprintf()` error_code=%d.\n", stat);
                return RP_FALSE;
        }

        va_end(args);

        return RP_TRUE;
}

rp_bool_t rp_debug_free(void)
{
        rp_u8_t i;

        if (!debug_is_init) {
                fprintf(stderr, "ERROR: Trying to call `rp_debug_free()` "
                                "when it's already freed.\n");
                return RP_FALSE;
        }

        for (i = 0; i < RP_DEB_STREAM_CNT; ++i)
                debug_streams[i] = NULL;

        debug_is_init = RP_FALSE;

        return RP_TRUE;
}
