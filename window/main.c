#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "rose_memory_wrapper.h"

enum {
        AT_NONE = 0,
        AT_CAMERA,
        AT_MODEL,
        ACTOR_TYPE_COUNT
};

typedef ru32 actor_type_e;

typedef struct {
        rf32 v[3];
} rv3f;

struct vertex {
        rv3f pos;
};

struct face {
        struct vertex verts[3];
};

struct model {
        struct face *faces;
        ru64         faces_cnt;
};

struct actor {
        struct model *mdl;
        rv3f          pos;
        actor_type_e  type;
};

static const struct face copy_faces[2] = {
        { /* face[0] */
                { /* verts */
                        { /* verts[0] */
                                { /* pos */
                                        0.f, 0.f, 0.f
                                }
                        },
                        { /* verts[1] */
                                { /* pos */
                                        1.f, 0.f, 0.f
                                }
                        },
                        { /* verts[2] */
                                { /* pos */
                                        0.f, 1.f, 0.f
                                }
                        }
                }
        },
        { /* face[1] */
                { /* verts */
                        { /* verts[0] */
                                { /* pos */
                                        0.f, 1.f, 0.f
                                }
                        },
                        { /* verts[1] */
                                { /* pos */
                                        1.f, 0.f, 0.f
                                }
                        },
                        { /* verts[2] */
                                { /* pos */
                                        1.f, 1.f, 0.f
                                }
                        }
                }
        }
};

static rb32 model_face_arrays_are_equal(const ru64         cnt0,
                                        const struct face *arr0,
                                        const ru64         cnt1,
                                        const struct face *arr1)
{
        /*
         * Really, I could use either one of the sizes,
         * but since they are supposed to be equal before
         * even trying to compare, I may as well just pick
         * one and be fucken' done with it. lol
         */
        const ru64 cnt = cnt0;

        if (cnt0 != cnt1)
                return RB_FALSE;

        for (ru64 i = 0; i < cnt; ++i) {
                const struct face *a = arr0 + i;
                const struct face *b = arr1 + i;

                for (ru64 j = 0; j < 3; ++j) {
                        for (ru64 k = 0; k < 3; ++k) {
                                if (a->verts[j].pos.v[k] !=
                                    b->verts[j].pos.v[k]) {
                                        return RB_FALSE;
                                }
                        }
                }
        }

        return RB_TRUE;
}

static struct model *model_allocate(const ru64 faces_cnt,
                                    const struct face *faces)
{
        struct model *m        = NULL;
        rb32          is_equal = RB_FALSE;

        assert(faces_cnt);
        assert(faces);

        m = (struct model *)rmw_alloc(sizeof(*m));
        assert(m);

        m->faces_cnt = faces_cnt;
        m->faces     = (struct face *)rmw_alloc(sizeof(*faces) * faces_cnt);
        memcpy(m->faces, faces, sizeof(*faces) * faces_cnt);

        is_equal = model_face_arrays_are_equal(m->faces_cnt, m->faces,
                                               faces_cnt, faces);
        assert(is_equal);

        return m;
}

static void model_free(struct model *m)
{
        m->faces_cnt = 0;
        rmw_free(m->faces);
        m->faces = NULL;
}

static struct actor *actor_allocate(struct model *mdl)
{
        struct actor *a = NULL;

        a = (struct actor *)rmw_alloc(sizeof(*a));
        assert(a);

        a->mdl = mdl;
        assert(a->mdl);

        return a;
}

static void actor_free(struct actor *a)
{
        model_free(a->mdl);
        a->mdl = NULL;

        rmw_free(a);
}

static void actor_print_info(const struct actor *a, FILE *stream)
{
        fprintf(stream, "Actor (.mdl = <%p>):\n", (void *)a->mdl);

        fprintf(stream, "\tModel (.faces = <%p>, .faces_cnt = %lu):\n",
                (void *)a->mdl->faces, a->mdl->faces_cnt);
        for (ru32 i = 0; i < a->mdl->faces_cnt; ++i) {
                const struct face *f = a->mdl->faces + i;

                fprintf(stream, "\t\tFace %u:\n", i);
                for (ru32 j = 0; j < 3; ++j) {
                        const rf32 *p = f->verts[j].pos.v;

                        fprintf(stream,
                                "\t\t\tVertex %u: "
                                ".pos = (%f, %f, %f)\n",
                                j, p[0], p[1], p[2]);
                }
        }
}

int main(void)
{
        struct actor *actor = NULL;

        actor = actor_allocate(model_allocate(1u, copy_faces));
        assert(actor);
        assert(actor->mdl);
        assert(actor->mdl->faces);
        assert(actor->mdl->faces_cnt);

        actor_print_info(actor, stdout);

        actor_free(actor);
        actor = NULL;

        rmw_print_status();

        return 0;
}
