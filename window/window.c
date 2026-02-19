#include <GLFW/glfw3.h>

#include "rp_window.h"

#include "rp_assert.h"

static GLFWwindow *rp_window_ptr = NULL;

void rp_window_init(const ru16 width, const ru16 height, const char *name)
{
        GLFWmonitor       *mon = NULL;
        const GLFWvidmode *vm  = NULL;

        RP_ASSERTM(!rp_window_ptr, "Window is already initialized!");

        glfwInit();
        rp_window_ptr = glfwCreateWindow(width, height, name, NULL, NULL);

        mon = glfwGetPrimaryMonitor();
        RP_ASSERTM(mon, "Failed to get primary monitor");

        vm = glfwGetVideoMode(mon);
        RP_ASSERTM(vm, "Failed to get video mode from primary monitor");

        /* Center the window in the monitor view-space */
        glfwSetWindowPos(rp_window_ptr,
                         (vm->width / 2) - (width / 2),
                         (vm->height / 2) - (height / 2));
}

rb32 rp_window_is_open(void)
{
        return (rb32)(!glfwWindowShouldClose(rp_window_ptr));
}

void rp_window_poll(void)
{
        glfwPollEvents();
}

void rp_window_swap_buffers(void)
{
        glfwSwapBuffers(rp_window_ptr);
}

void rp_window_terminate(void)
{
        RP_ASSERTM(rp_window_ptr, "Window was never initialized!");

        glfwDestroyWindow(rp_window_ptr);
        glfwTerminate();
}
