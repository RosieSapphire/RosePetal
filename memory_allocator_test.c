#include <stdint.h>
#include <stdlib.h>

#include "int_types.h"

#define MEMORY_ALLOCATOR_WRAP_STDLIB
#include "memory_allocator.h"

int main(void)
{
	u32 *a, *b, *c;

	mem_register_exit_callback();

	a = (u32 *)malloc(sizeof(*a) * 5ul);
	b = (u32 *)malloc(sizeof(*b) * 63ul);
	c = (u32 *)malloc(sizeof(*c) * 4096ul);
#if 1
	mem_free(c);
	mem_free(b);
	mem_free(a);
#endif

	/* TODO: Register terminate `atexit()`. */

	return 0;
}
