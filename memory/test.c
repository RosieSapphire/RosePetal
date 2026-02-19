#include <stdio.h>

#include "rp_assert.h"
#include "rp_memory.h"

int main(void)
{
        ru32 *test0 = (ru32 *)0xCDCDCDCDCDCDCDCD;
        ru8  *test1 = (ru8 *)0xCDCDCDCDCDCDCDCD;

        printf("Beginning Rose Memory Wrapper Test!\n");

        test0 = (ru32 *)rp_mem_alloc(sizeof(*test0) * 5, "Test 0");
        RP_ASSERTM(test0, "Failed to allocate Test 0");

        test1 = (ru8 *)rp_mem_alloc(sizeof(*test1) * 5, "Test 1");
        RP_ASSERTM(test1, "Failed to allocate Test 1");

        for (ru64 i = 0; i < 5; ++i) {
                test0[i] = (ru32)(1u << (27 + i));
                test1[i] = (ru8)(1u << (3 + i));
        }

        printf("Set Test 0's allocated values to: [\n");
        for (int i = 0; i < 5; ++i)
                printf("\t%u,\n", test0[i]);
        printf("]\n");

        rp_mem_free((void *)test0);
        test0 = NULL;

        printf("Set Test 1's allocated values to: [\n");
        for (int i = 0; i < 5; ++i)
                printf("\t%u,\n", test1[i]);
        printf("]\n");

        rp_mem_free(test1);
        test1 = NULL;

        rp_mem_print_status();

        return 0;
}
