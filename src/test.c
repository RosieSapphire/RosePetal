#include "rosepetal.h"

enum {
        RET_OKAY,
        RET_RP_DEBUG_INIT_FAIL,
        RET_CODE_CNT
};

int main(void)
{
        if (!rp_debug_init(RP_DEBUG_INIT_DEFAULT))
                return RET_RP_DEBUG_INIT_FAIL;

        rp_debug_printf(RP_DEB_STREAM_INFO,
                        "This is a test info. %d\n",
                        69);
        rp_debug_printf(RP_DEB_STREAM_WARNING,
                        "This is a test warning. %d\n",
                        420);
        rp_debug_printf(RP_DEB_STREAM_ERROR,
                        "This is a test error. %d\n",
                        42069);

        if (!rp_debug_free())
                return RET_RP_DEBUG_INIT_FAIL;

        return RET_OKAY;
}
