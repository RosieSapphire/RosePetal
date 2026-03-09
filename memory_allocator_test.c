#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "types.h"

#define MEMORY_ALLOCATOR_WRAP_STDLIB
#include "memory_allocator.h"

#define PTR_TEST_CNT 50ul

/*
 * Randomly frees only some of the blocks to simulate a lazy-ass programmer
 */
/* #define FREE_PARTIAL */

static u32 *ptr_test[PTR_TEST_CNT] = { NULL };

static __inline u8 rand_u8_nz(void)
{
	u8 r = UINT8_MAX;

	do {
		r = rand() & UINT8_MAX;
	} while (!r);

	return r;
}

static __inline u32 rand_u32(void)
{
	return (u32)(rand() & UINT32_MAX);
}

int main(void)
{
	size_t i;

	mem_register_exit_callback();

	srand(time(NULL));

	/* Allocate a bunch of memory */
	for (i = 0ul; i < PTR_TEST_CNT; ++i) {
		size_t j;
		const u8 num = rand_u8_nz();

		ptr_test[i] = malloc(sizeof(**ptr_test) * num);
		assert(ptr_test[i]);
		for (j = 0ul; j < num; ++j)
			*ptr_test[i] = rand_u32();
	}

	/* Free all of it */
	for (i = 0ul; i < PTR_TEST_CNT; ++i) {
#ifdef FREE_PARTIAL
		/* 50/50 chance it'll skip freeing */
		if (rand() & 1)
			continue;
#endif /* #ifdef FREE_PARTIAL */
		free(ptr_test[i]);
		ptr_test[i] = NULL;
	}

	return 0;
}
