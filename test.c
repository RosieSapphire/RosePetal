/*
 * TODO: Add a separate `rp_log.h` file that can be toggled with
 *       `RP_LOG_ENABLE`. Module-specific macros like `RP_MEMORY_LOG`
 *       and `RP_RANDOM_LOG` will turn on and off this macro for
 *       those specific modules depending on what is necessary.
 */

/* rp_random.h */
#define RP_RANDOM_TEST
#define RP_RANDOM_IMPLEMENTATION
#include "rp_random.h"

/* rp_memory.h */
#define RP_MEMORY_LOG
#define RP_MEMORY_TEST
#define RP_MEMORY_TEST_ALLOC_CNT 16ul
#define RP_MEMORY_IMPLEMENTATION
#include "rp_memory.h"

int main(void)
{
	rp_random_test();
	rp_memory_test();
	return 0;
}
