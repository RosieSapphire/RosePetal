#include "opengl.h"

#include <assert.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define OGL_ERRBUF_SZ 512u

/***********
 * GENERAL *
 ***********/

void opengl_init(GLADloadproc load_proc)
{
	s32 r = INT32_MAX;

	r = gladLoadGLLoader(load_proc);
	assert(r);
}

void opengl_screen_clear(const u8 r, const u8 g, const u8 b)
{
	const float rf = (float)r / 255.f;
	const float gf = (float)g / 255.f;
	const float bf = (float)b / 255.f;

	glClearColor(rf, gf, bf, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void opengl_setup_3d(const u16 width, const u16 height)
{
	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
}

/**********
 * MODELS *
 **********/

void opengl_model_generate_triangle(struct ogl_model *m)
{
	static const struct ogl_vertex tri_verts[3] = {
		{
				{ 0.0f, 0.5f, 0.f },	/* Position */
				{ 0.f, 0.f, -1.f },	/* Normal */
				{ 1.f, 0.f, 0.f, 1.f }, /* Color */
		},
		{
				{ -0.5f, -0.5f, 0.f },	/* Position */
				{ 0.f, 0.f, -1.f },	/* Normal */
				{ 0.f, 1.f, 0.f, 1.f }, /* Color */
		},
		{
				{ 0.5f, -0.5f, 0.f },	/* Position */
				{ 0.f, 0.f, -1.f },	/* Normal */
				{ 0.f, 0.f, 1.f, 1.f }, /* Color */
		}
	};

	static const u32 tri_indis[3] = { 0, 1, 2 };

	size_t verts_sz = SIZE_MAX;
	size_t indis_sz = SIZE_MAX;

	/* Copy over triangle vertex and index data */
	m->verts_cnt = 3;
	m->indis_cnt = 3;

	verts_sz = sizeof(*m->verts_arr) * m->verts_cnt;
	indis_sz = sizeof(*m->indis_arr) * m->indis_cnt;

	m->verts_arr = malloc(verts_sz);
	m->indis_arr = malloc(indis_sz);
	assert(m->verts_arr);
	assert(m->indis_arr);

	memcpy(m->verts_arr, tri_verts, verts_sz);
	memcpy(m->indis_arr, tri_indis, indis_sz);

	/* Set up the OpenGL vertex and index buffers */
	glGenVertexArrays(1, &m->vao);
	glGenBuffers(1, &m->vbo);
	glGenBuffers(1, &m->ebo);

	/* Array object */
	glBindVertexArray(m->vao);

	/* Vertex buffer */
	glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
	glBufferData(GL_ARRAY_BUFFER, verts_sz, m->verts_arr, GL_STATIC_DRAW);
	glVertexAttribPointer(0,
			      3,
			      GL_FLOAT,
			      GL_FALSE,
			      sizeof(struct ogl_vertex),
			      (void *)offsetof(struct ogl_vertex, pos));
	glVertexAttribPointer(1,
			      3,
			      GL_FLOAT,
			      GL_TRUE, /* FIXME: Is this a good idea? */
			      sizeof(struct ogl_vertex),
			      (void *)offsetof(struct ogl_vertex, nrm));
	glVertexAttribPointer(2,
			      4,
			      GL_FLOAT,
			      GL_FALSE,
			      sizeof(struct ogl_vertex),
			      (void *)offsetof(struct ogl_vertex, col));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Index buffer */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		     sizeof(m->indis_arr),
		     m->indis_arr,
		     GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

void opengl_model_render(const struct ogl_model *m, const u32 s)
{
	glBindVertexArray(m->vao);
	glUseProgram(s);
	glDrawElements(GL_TRIANGLES,
		       m->indis_cnt,
		       GL_UNSIGNED_INT,
		       m->indis_arr);
	glBindVertexArray(0);
}

void opengl_model_terminate(struct ogl_model *m)
{
	glDeleteBuffers(1, &m->ebo);
	glDeleteBuffers(1, &m->vbo);
	glDeleteVertexArrays(1, &m->vao);
	free(m->verts_arr);
	free(m->indis_arr);
	m->verts_arr = NULL;
	m->indis_arr = NULL;
	m->verts_cnt = 0;
	m->indis_cnt = 0;
}

/***********
 * SHADERS *
 ***********/

u32 opengl_shader_part_create(char **src, const u32 type)
{
	char log[OGL_ERRBUF_SZ] = "";
	u32  ret		= UINT32_MAX;
	s32  err		= INT32_MAX;

	ret = glCreateShader(type);
	glShaderSource(ret, 1, (const char *const *)src, NULL);
	glCompileShader(ret);
	glGetShaderiv(ret, GL_COMPILE_STATUS, &err);
	if (!err) {
		glGetShaderInfoLog(ret, OGL_ERRBUF_SZ, NULL, log);
		fprintf(stderr, "VERTEX SHADER ERROR: \"%s\"\n", log);
		glDeleteShader(ret);
		ret = 0;
	}

	return ret;
}

u32 opengl_shader_program_link(const u32 v, const u32 f)
{
	char log[OGL_ERRBUF_SZ] = "";
	u32  ret		= UINT32_MAX;
	s32  err		= INT32_MAX;

	ret = glCreateProgram();
	glAttachShader(ret, v);
	glAttachShader(ret, f);
	glLinkProgram(ret);
	glGetProgramiv(ret, GL_LINK_STATUS, &err);
	if (!err) {
		glGetProgramInfoLog(ret, OGL_ERRBUF_SZ, NULL, log);
		fprintf(stderr, "SHADER PROGRAM: \"%s\"\n", log);
		ret = 0;
	}

	return ret;
}

u32 opengl_shader_program_create(const char *vpath, const char *fpath)
{
	char *vshd_src = NULL;
	char *fshd_src = NULL;
	u32   vshd     = UINT32_MAX;
	u32   fshd     = UINT32_MAX;
	u32   ret      = UINT32_MAX;

	/* Read data from files */
	vshd_src = file_alloc_and_read_to_buffer(vpath);
	fshd_src = file_alloc_and_read_to_buffer(fpath);
	assert(vshd_src);
	assert(fshd_src);

	/* Create the individual shader parts */
	vshd = opengl_shader_part_create(&vshd_src, GL_VERTEX_SHADER);
	fshd = opengl_shader_part_create(&fshd_src, GL_FRAGMENT_SHADER);
	assert(vshd);
	assert(fshd);

	/* Free the source code buffers */
	free(vshd_src);
	free(fshd_src);
	vshd_src = NULL;
	fshd_src = NULL;

	/* Link them together into a program! */
	ret = opengl_shader_program_link(vshd, fshd);

	/* Destroy the two parts */
	glDeleteShader(vshd);
	glDeleteShader(fshd);

	/* There we go! ^w^ */
	return ret;
}

void opengl_shader_program_destroy(const u32 s)
{
	glDeleteProgram(s);
}
