#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "rp_memory.h"
#include "rp_assert.h"

struct rp_mem_block {
        const char *tag;
        void       *ptr;
        ru64        sz;
};

static struct rp_mem_block rp_mem_blocks[RP_MEM_MAX_ALLOC_CNT] = { 0 };

/*
 * FIXME:
 * This is kinda fucking shit. It would be MUCH better to just
 * have a head pointer that wraps around to the beginning.
 */
static struct rp_mem_block *rp_mem_block_get_first_free(void)
{
        for (ru64 i = 0; i < RP_MEM_MAX_ALLOC_CNT; ++i) {
                struct rp_mem_block *check = rp_mem_blocks + i;

                if (check->ptr || check->sz || check->tag)
                        continue;

                return check;
        }

        RP_ASSERTF(0,
                   "No internal memory left to find a free block "
                   "(BR_MEM_MAX_ALLOC_CNT = %lu).",
                   RP_MEM_MAX_ALLOC_CNT);
        return NULL;
}

void *rp_mem_alloc(const ru64 sz, const char *tag)
{
        struct rp_mem_block *block = NULL;

        RP_ASSERTM(sz, "Trying to allocate a pointer with 0 size!");

        RP_ASSERTM(__builtin_popcount(RP_MEM_MAX_ALLOC_CNT) == 1,
                   "RP_MEM_MAX_ALLOC_CNT is not a power of 2!");

        block = rp_mem_block_get_first_free();
        RP_ASSERTM(block, "Failed to find first free slot; must be outta mem!");

        block->tag = tag;
        block->ptr = malloc(sz);
        block->sz  = sz;

#if RP_MEM_DEBUG_PRINTF
        printf("RP_MEM: [ALLOC] %lu bytes to <%p>",
               block->sz,
               block->ptr);
        if (block->tag)
                printf(" with tag \"%s\"", block->tag);
        printf("\n");
#endif /* #if RP_MEM_DEBUG_PRINTF */

        return block->ptr;
}

static struct rp_mem_block *rp_mem_block_find_by_ptr(const void *p)
{
        if (!p)
                return NULL;

        for (ru64 i = 0; i < RP_MEM_MAX_ALLOC_CNT; ++i) {
                struct rp_mem_block *cur = rp_mem_blocks + i;

                if (cur->ptr == p)
                        return cur;
        }

        /* Couldn't find it. Sadge. :c */
        return NULL;
}

static ru64 rp_mem_get_active_blocks_cnt(void)
{
        ru64 total = 0;

        for (ru64 i = 0; i < RP_MEM_MAX_ALLOC_CNT; ++i) {
                struct rp_mem_block *cur = rp_mem_blocks + i;

                if (!cur->ptr)
                        continue;

                if (!cur->sz)
                        return 0;

                ++total;
        }

        return total;
}

void rp_mem_free(void *p)
{
        struct rp_mem_block *b          = NULL;
        ru64                 active_cnt = 0xFFFFFFFFFFFFFFFF;

        active_cnt = rp_mem_get_active_blocks_cnt();
        RP_ASSERTM(p, "No pointer passed in to free");
        RP_ASSERTM(active_cnt, "There are no blocks active to even free");

        b = rp_mem_block_find_by_ptr(p);
        RP_ASSERTF(b, "Failed to find memory block with pointer <%p>", p);

        /* Paranoid */
        RP_ASSERTM(b->ptr == p, "Found the wrong pointer... some-fucking-how");

#if RP_MEM_DEBUG_PRINTF
        printf("RP_MEM: [FREE] %lu bytes from <%p>",
               b->sz,
               b->ptr);
        if (b->tag)
                printf(" with tag \"%s\"", b->tag);
        printf("\n");
#endif /* #if RP_MEM_DEBUG_PRINTF */

        free(b->ptr);
        b->ptr = NULL;

        b->sz = 0;
}

static ru64 *rp_mem_blocks_get_active_indis(ru64 *arr, const ru64 num)
{
        ru64 i, x;

        if (!arr || !num)
                return 0;

        RP_ASSERTF(num <= RP_MEM_MAX_ALLOC_CNT,
                   "Maximum blocks for internal heap "
                   "is %lu, but specified %lu\n",
                   RP_MEM_MAX_ALLOC_CNT,
                   num);

        i = 0;
        x = 0;
        while (i < num && x < RP_MEM_MAX_ALLOC_CNT) {
                struct rp_mem_block *cur = rp_mem_blocks + x;

                if (!cur->ptr) {
                        ++x;
                        continue;
                }

                if (!cur->sz)
                        return 0;

                arr[i] = x;

                ++i;
                ++x;
        }

        return arr;
}

void rp_mem_print_status(void)
{
        ru64  active_cnt   = 0xFFFFFFFFFFFFFFFF;
        ru64 *active_indis = NULL;

        printf("RP MEMORY STATUS:\n");

        active_cnt = rp_mem_get_active_blocks_cnt();
        if (!active_cnt) {
                printf("\tNo blocks left in use. Good job!\n");
                return;
        }

        active_indis = (ru64 *)malloc(sizeof(*active_indis * active_cnt));
        RP_ASSERTM(active_indis, "Failed to allocate memory for indices");

        active_indis = rp_mem_blocks_get_active_indis(active_indis, active_cnt);
        RP_ASSERTM(active_indis, "Failed to get active indices");

        for (ru64 i = 0; i < active_cnt; ++i) {
                const ru64                 ind = active_indis[i];
                const struct rp_mem_block *cur = rp_mem_blocks + ind;

                printf("\t%lu bytes active at <%p> (alloc index %lu)\n",
                       cur->sz,
                       cur->ptr,
                       ind);
        }

        free(active_indis);
}

extern void rp_mem_free_all(void)
{
        for (ru64 i = 0; i < RP_MEM_MAX_ALLOC_CNT; ++i) {
        }
}

#undef PTR_NO_INIT
#undef RP_ASSERT
