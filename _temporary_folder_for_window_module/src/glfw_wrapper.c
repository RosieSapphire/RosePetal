#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "glfw_wrapper.h"

static __inline void glfw_err_cbfn(int e, const char *d)
{
	fprintf(stderr, "GLFW ERROR (0x%.8X): \"%s\"\n", e, d);
	abort();
}

void glfw_init(void)
{
	s32 r = INT32_MAX;

	glfwSetErrorCallback((GLFWerrorfun)glfw_err_cbfn);

	r = glfwInit();
	assert(r);
}

GLFWwindow *glfw_window_create(const u16 width, const u16 height, const char *name)
{
	const GLFWvidmode *vidmode = NULL;
	GLFWmonitor	  *monitor = NULL;
	GLFWwindow	  *ret	   = NULL;

	/* Get the current video mode */
	monitor = glfwGetPrimaryMonitor();
	assert(monitor);

	vidmode = glfwGetVideoMode(monitor);
	assert(vidmode);

	/* Set Video Mode-Specific Window Hints */
	glfwWindowHint(GLFW_RED_BITS, vidmode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, vidmode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, vidmode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, vidmode->refreshRate);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	/* Set OpenGL-Specific Window Hints */
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

	/* Create the window itself */
	ret = glfwCreateWindow(width, height, name, NULL, NULL);
	assert(ret);

	/* Make it the current context */
	glfwMakeContextCurrent(ret);

	/* Center it */
	glfwSetWindowPos(ret,
			 (vidmode->width >> 1) - (width >> 1),
			 (vidmode->height >> 1) - (height >> 1));

	return ret;
}

void glfw_window_destroy(GLFWwindow **wnd)
{
	glfwDestroyWindow(*wnd);
	*wnd = NULL;
}
