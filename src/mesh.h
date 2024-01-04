#ifndef _MESH_H
#define _MESH_H

#include <stddef.h>

#include <cglm/struct.h>
#include <glad/glad.h>

#include "tools.h"

enum attr_ty {
	ATTR_TY_I32,
	ATTR_TY_U32,
	ATTR_TY_F32,
	ATTR_TY_MAT3,
	ATTR_TY_MAT4,
};
struct vtx_attr {
	i32 len;
	enum attr_ty type;
	u32 stride, offset;
	const char *name;
};
#define _GEN_VTX_ATTR(s, f, n) .stride = sizeof(s), .offset = offsetof(s, f), .name = n,
#define GEN_VTX_ATTR(s, f, n) (_Generic((((s){0}).f), \
	i32 : (struct vtx_attr){ .len = 1, .type = ATTR_TY_I32, _GEN_VTX_ATTR(s,f,n) }, \
	u32 : (struct vtx_attr){ .len = 1, .type = ATTR_TY_U32, _GEN_VTX_ATTR(s,f,n) }, \
	f32 : (struct vtx_attr){ .len = 1, .type = ATTR_TY_F32, _GEN_VTX_ATTR(s,f,n) }, \
	vec2s : (struct vtx_attr){ .len = 2, .type = ATTR_TY_F32, _GEN_VTX_ATTR(s,f,n) }, \
	vec3s : (struct vtx_attr){ .len = 3, .type = ATTR_TY_F32, _GEN_VTX_ATTR(s,f,n) }, \
	vec4s : (struct vtx_attr){ .len = 4, .type = ATTR_TY_F32, _GEN_VTX_ATTR(s,f,n) }, \
	ivec2s : (struct vtx_attr){ .len = 2, .type = ATTR_TY_I32, _GEN_VTX_ATTR(s,f,n) }, \
	ivec3s : (struct vtx_attr){ .len = 3, .type = ATTR_TY_I32, _GEN_VTX_ATTR(s,f,n) }, \
	ivec4s : (struct vtx_attr){ .len = 4, .type = ATTR_TY_I32, _GEN_VTX_ATTR(s,f,n) }, \
	mat3s : (struct vtx_attr){ .len = 3, .type = ATTR_TY_MAT3, _GEN_VTX_ATTR(s,f,n) }, \
	mat4s : (struct vtx_attr){ .len = 4, .type = ATTR_TY_MAT4, _GEN_VTX_ATTR(s,f,n) } \
))
#define VTX_ATTR_END ((struct vtx_attr){0})

struct mesh {
	GLuint vao, vbo, ebo, ibo;
	bool use_ibo, use_ebo;
};

struct mesh mesh_init(const struct vtx_attr *vtx_attrs, bool use_ebo, const struct vtx_attr *ibo_attrs);
void mesh_deinit(struct mesh *self);
void mesh_upload_verts(struct mesh *self, const void *data, usize size);
void mesh_upload_indexes(struct mesh *self, const u32 *indexes, usize size);
void mesh_upload_instances(struct mesh *self, const void *data, usize size);
u32 vtx_attr_num_locations(const struct vtx_attr *attr);
bool vtx_attr_is_end(const struct vtx_attr *attr);

struct quad_vtx {
	vec3s pos;
	vec3s norm;
	vec3s uv;
};
struct quad_inst {
	mat4s world;
	mat3s texmat;
};

const struct vtx_attr *vtx_attrs_quad(void);
const struct vtx_attr *ibo_attrs_quad(void);
void quad_upload_vtxs(struct mesh *mesh);

#endif

