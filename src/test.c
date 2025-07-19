#include <SDL2/SDL_timer.h>

#include "rosepetal.h"

enum {
        RET_OKAY,
        RET_RP_DEBUG_INIT_FAIL,
        RET_RP_DEBUG_FREE_FAIL,
        RET_RP_TIMER_INIT_FAIL,
        RET_RP_TIMER_FREE_FAIL,
        RET_CODE_CNT
};

int main(void)
{
        rp_f64_t time_prv, time_cur, time_accum;

        if (!rp_debug_init(RP_DEBUG_INIT_DEFAULT))
                return RET_RP_DEBUG_INIT_FAIL;

        if (!rp_timer_init())
                return RET_RP_TIMER_INIT_FAIL;

        time_prv = rp_timer_get_sec64();
        time_cur = time_prv;

        while (1) {
                static       rp_s32_t tick_cnt   = 0;
                static const rp_s32_t tickrate   = 4;
                static const rp_f64_t tick_delta = (1. / tickrate);

                time_prv    = time_cur;
                time_cur    = rp_timer_get_sec64();
                time_accum += (time_cur - time_prv);

                while (time_accum >= tick_delta) {
                        rp_debug_printf(RP_DBG_STREAM_INFO,
                                        "Tick %d.\n", ++tick_cnt);
                        time_accum -= tick_delta;
                }
        }

        if (!rp_timer_free())
                return RET_RP_TIMER_FREE_FAIL;

        if (!rp_debug_free())
                return RET_RP_DEBUG_FREE_FAIL;

        return RET_OKAY;
}
