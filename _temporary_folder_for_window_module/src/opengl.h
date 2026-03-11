#ifndef T3D_EDITOR_OPENGL_H
#define T3D_EDITOR_OPENGL_H

#include <glad/glad.h>

#include "int_def.h"

struct ogl_vertex {
	float pos[3];
	float nrm[3];
	float col[4];
};

struct ogl_model {
	u32		   verts_cnt;
	u32		   indis_cnt;
	struct ogl_vertex *verts_arr;
	u32		  *indis_arr;
	u32		   vao;
	u32		   vbo;
	u32		   ebo;
};

extern void opengl_init(GLADloadproc load_proc);
extern void opengl_screen_clear(const u8 r, const u8 g, const u8 b);
extern void opengl_setup_3d(const u16 width, const u16 height);

extern void opengl_model_generate_triangle(struct ogl_model *m);
extern void opengl_model_render(const struct ogl_model *m, const u32 s);
extern void opengl_model_terminate(struct ogl_model *m);

extern u32  opengl_shader_part_create(char **src, const u32 type);
extern u32  opengl_shader_program_link(const u32 v, const u32 f);
extern u32  opengl_shader_program_create(const char *vpath, const char *fpath);
extern void opengl_shader_program_destroy(const u32 s);

#endif /* #ifndef T3D_EDITOR_OPENGL_H */
