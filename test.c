/* rp_random.h */
#define RP_RANDOM_TEST
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
