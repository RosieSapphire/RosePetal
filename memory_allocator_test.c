#include <stdint.h>
#include <stdlib.h>

#include "int_types.h"

#define MEMORY_ALLOCATOR_WRAP_STDLIB
#include "memory_allocator.h"

#define PTR_TEST_CNT 50u

int main(void)
{
	mem_register_exit_callback();

	return 0;
}
