#include "opengl.h"
#include "glfw_wrapper.h"

#define MAX_VERTEX_BUFFER (1u << 19u)  /* 512 KB */
#define MAX_ELEMENT_BUFFER (1u << 17u) /* 128 KB */
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"

#define WND_WID 1280u
#define WND_HEI 720u
#define WND_NAME "T3D Model Viewer"

static GLFWwindow	 *window	 = NULL;
static struct nk_context *nk_ctx	 = NULL;
static struct nk_glfw	  nk_glfw	 = { 0 };
static u32		  shader_program = UINT32_MAX;
static struct ogl_model	  model_test	 = { 0 };

/*********************
 * NUKLEAR FUNCTIONS *
 *********************/
static struct nk_context *nuklear_init(struct nk_glfw *glfw, GLFWwindow *wnd)
{
	struct nk_context    *c	 = NULL;
	struct nk_font_atlas *fa = NULL;

	c = nk_glfw3_init(glfw, wnd, NK_GLFW3_INSTALL_CALLBACKS);
	assert(c);

	nk_glfw3_font_stash_begin(&nk_glfw, &fa);
	assert(fa);
	nk_glfw3_font_stash_end(&nk_glfw);

	return c;
}

static __inline void nuklear_terminate(struct nk_context **ctx)
{
	nk_free(*ctx);
	*ctx = NULL;
}

int main(void)
{
	/********
	 * INIT *
	 ********/
	glfw_init();
	window = glfw_window_create(WND_WID, WND_HEI, WND_NAME);
	assert(window);

	opengl_init((GLADloadproc)glfwGetProcAddress);
	opengl_model_generate_triangle(&model_test);
	shader_program = opengl_shader_program_create("res/vert.glsl",
						      "res/frag.glsl");
	assert(shader_program != 0);

	nk_ctx = nuklear_init(&nk_glfw, window);
	assert(nk_ctx);

	/*******
	 * RUN *
	 *******/
	while (!glfwWindowShouldClose(window)) {
		/*
		 * Update
		 */
		glfwPollEvents();
		nk_glfw3_new_frame(&nk_glfw);
		if (nk_begin(nk_ctx,
			     "Test",
			     nk_rect(0, 0, WND_WID >> 2, WND_HEI >> 2),
			     0)) {
			nk_layout_row_dynamic(nk_ctx, 120, 1);
			nk_label(nk_ctx, "Hello, Bitches!", NK_TEXT_CENTERED);

			nk_layout_row_static(nk_ctx, 30, 80, 1);
			if (nk_button_label(nk_ctx, "button")) {
				static int cnt = 0;

				printf("Button Pressed %d times!\n", ++cnt);
			}
		}
		nk_end(nk_ctx);

		/*
		 * Rendering
		 */
		opengl_screen_clear(0x05, 0x0A, 0x14);

		/* 3D */
		opengl_setup_3d(WND_WID, WND_HEI);
		opengl_model_render(&model_test, shader_program);

		/* UI */
		nk_glfw3_render(&nk_glfw,
				NK_ANTI_ALIASING_ON,
				MAX_VERTEX_BUFFER,
				MAX_ELEMENT_BUFFER);
		glfwSwapBuffers(window);
	}

	/*************
	 * TERMINATE *
	 *************/
	nuklear_terminate(&nk_ctx);

	opengl_shader_program_destroy(shader_program);
	opengl_model_terminate(&model_test);

	glfw_window_destroy(&window);
	glfwTerminate();

	return 0;
}
