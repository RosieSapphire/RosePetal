#ifndef T3D_EDITOR_GLFW_WRAPPER_H
#define T3D_EDITOR_GLFW_WRAPPER_H

#include <GLFW/glfw3.h>

#include "int_def.h"

extern void glfw_init(void);
extern GLFWwindow *
glfw_window_create(const u16 width, const u16 height, const char *name);
extern void glfw_window_destroy(GLFWwindow **wnd);

#endif /* #ifndef T3D_EDITOR_GLFW_WRAPPER_H */
