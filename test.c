#include <assert.h>
#include <stdio.h>
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
#define FREE_PARTIAL

static u32 *ptr_test[PTR_TEST_CNT] = { NULL };

static __inline u8 rand_u8_nz(void)
{
	u8 r = UINT8_MAX;

	do {
		r = rand() & UINT8_MAX;
	} while (!r);

	return r;
}

static __inline s32 rand_s32(void)
{
	return (s32)(rand() & INT32_MAX);
}

int main(void)
{
	size_t i;

	mem_register_exit_callback();

	srand((u32)time(NULL));

	/* Allocate a bunch of memory */
	for (i = 0ul; i < PTR_TEST_CNT; ++i) {
		size_t j;
		const u8 num = rand_u8_nz();

		ptr_test[i] = (u32 *)malloc(sizeof(**ptr_test) * num);
		assert(ptr_test[i]);

		/* If 1/2 chance succeeds, just continue like normal */
		if (rand() & 1) {
			for (j = 0ul; j < num; ++j)
				*(ptr_test[i]) = (u32)rand_s32();

			continue;
		}

		/*
		 * If it fails, go through all that's currently
		 * allocated, and randomly (50/50) free those pointers.
		 */
		for (j = 0ul; j <= i; ++j) {
			if (rand() & 1)
				continue;

			if (!ptr_test[j])
				continue;

			free(ptr_test[j]);
			ptr_test[j] = NULL;
		}
	}

	/* Free all of it */
	for (i = 0ul; i < PTR_TEST_CNT; ++i) {
		if (!ptr_test[i])
			continue;

#ifdef FREE_PARTIAL
		/* 7/8 chance it'll skip freeing */
		if (rand() & 7)
			continue;
#endif /* #ifdef FREE_PARTIAL */

		free(ptr_test[i]);
		ptr_test[i] = NULL;
	}

	return 0;
}
