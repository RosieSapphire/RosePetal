#ifndef ROSE_PETAL_MEMORY_H
#define ROSE_PETAL_MEMORY_H

#include "rp_types.h"

/*
 * Allocate memory through RMW.
 * You can add an optional tag to the allocation
 * to give it a more recognizable name upon calling
 * `rp_mem_print_status()`. Although, this can also
 * just be NULL and it'll just print the memory address.
 *
 * NOTE: `const char *tag` _MUST NOT_ BE ALLOCATED ON THE HEAP!
 */
extern void *rp_mem_alloc(const ru64 sz, const char *tag);

/*
 * Free memory through RMW.
 * Not that this does not free the allocation
 * containers used internally with
 * Rose Memory Wrapper; that would require a
 * call to `rp_mem_free_all()` to do so.
 */
extern void rp_mem_free(void *p);

/*
 * Print to `stdout` the current status of the heap.
 * Essentially, how many allocations and frees there
 * were, as well as how many bytes were managed across
 * both operations, as well as a net total left allocated
 * and which blocks they're in!
 */
extern void rp_mem_print_status(void);

/*
 * Frees all memory allocated internally through
 * calls to `rp_mem_alloc()`, and ALSO frees the array
 * used to keep track of all said allocations.
 */
extern void rp_mem_free_all(void);

#endif /* #ifndef ROSE_PETAL_MEMORY_H */
