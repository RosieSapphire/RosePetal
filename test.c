#include <assert.h>
#include <stdio.h>

#include "types.h"
#include "random.h"

#define MEMORY_ALLOCATOR_WRAP_STDLIB
#define MEMORY_ALLOCATOR_IMPLEMENTATION
#include "memory_allocator.h"

#define PTR_TEST_CNT 50ul

/*
 * Randomly frees only some of the blocks to simulate a lazy-ass programmer
 */
#if 1
#define SIMULATE_LEAK
#endif

static u32 *ptr_test[PTR_TEST_CNT] = { NULL };

int main(void)
{
	size_t i;

	mem_register_exit_callback();

	random_seed(UINT32_MAX);

	/* Allocate a bunch of memory */
	for (i = 0ul; i < PTR_TEST_CNT; ++i) {
		size_t j;
		u8     num;

		do {
			num = random_u8();
		} while (!num);

		ptr_test[i] = (u32 *)malloc(sizeof(**ptr_test) * num);
		assert(ptr_test[i]);

		/* 2/3 of the time, just continue like normal */
		if (random_u32() % 3) {
			for (j = 0ul; j < num; ++j)
				*(ptr_test[i]) = random_u32();

			continue;
		}

		/*
		 * The other 1/3 of the time, go through all that's currently
		 * allocated, and randomly (another 50%) free those pointers.
		 */
		for (j = 0ul; j <= i; ++j) {
			if (random_bool_50_percent())
				continue;

			if (!ptr_test[j])
				continue;

			free(ptr_test[j]);
			ptr_test[j] = NULL;
		}
	}

	/* Free all of it */
#ifndef SIMULATE_LEAK
	for (i = 0ul; i < PTR_TEST_CNT; ++i) {
		if (!ptr_test[i])
			continue;

		free(ptr_test[i]);
		ptr_test[i] = NULL;
	}
#endif /* #ifndef SIMULATE_LEAK */

	return 0;
}
