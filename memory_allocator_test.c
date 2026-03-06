#include <stdint.h>

#include "int_types.h"
#include "memory_allocator.h"

int main(void)
{
	u32 *a, *b, *c;

	mem_init();

	a = (u32 *)mem_alloc(sizeof(*a) * 5ul);
	b = (u32 *)mem_alloc(sizeof(*b) * 63ul);
	c = (u32 *)mem_alloc(sizeof(*c) * 4096ul);
	mem_free(c);
	/* mem_free(b); */
	mem_free(a);

	mem_terminate();

	return 0;
}
