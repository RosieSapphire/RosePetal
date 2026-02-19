#ifndef ROSE_PETAL_WINDOW_H
#define ROSE_PETAL_WINDOW_H

#include "rp_types.h"

extern void
rp_window_init(const ru16 width, const ru16 height, const char *name);
extern void rp_window_poll(void);
extern void rp_window_swap_buffers(void);
extern rb32 rp_window_is_open(void);
extern void rp_window_terminate(void);

#endif /* #ifndef ROSE_PETAL_WINDOW_H */
