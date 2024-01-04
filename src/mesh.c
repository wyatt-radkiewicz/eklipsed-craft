#include "mesh.h"

static GLenum _vtx_attr_get_type(const struct vtx_attr *attr) {
	switch (attr->type) {
	case ATTR_TY_I32: return GL_INT;
	case ATTR_TY_U32: return GL_UNSIGNED_INT;
	default: return GL_FLOAT;
	}
}

static const struct vtx_attr *apply_vtx_attr(u32 *loc, const struct vtx_attr *attr, i32 divisor) {
	for (i32 i = 0; i < vtx_attr_num_locations(attr); i++) {
		glVertexAttribPointer(
			*loc,
			attr->len,
			_vtx_attr_get_type(attr),
			GL_FALSE,
			attr->stride,
			(void *)(usize)(attr->offset + (i * attr->len * 4))
		);
		glEnableVertexAttribArray(*loc);
		glVertexAttribDivisor(*loc, divisor);
		(*loc)++;
	}
	return attr + 1;
}

struct mesh mesh_init(const struct vtx_attr *vtx_attrs, bool use_ebo, const struct vtx_attr *ibo_attrs) {
	struct mesh mesh = {
		.use_ebo = use_ebo,
		.use_ibo = ibo_attrs,
	};
	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);
	if (mesh.use_ibo) glGenBuffers(1, &mesh.ibo);
	if (mesh.use_ebo) glGenBuffers(1, &mesh.ebo);

	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	if (mesh.use_ebo) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
	u32 loc = 0;
	while (!vtx_attr_is_end(vtx_attrs = apply_vtx_attr(&loc, vtx_attrs, 0)));
	if (!mesh.use_ibo) goto ret;
	glBindBuffer(GL_ARRAY_BUFFER, mesh.ibo);
	while (!vtx_attr_is_end(ibo_attrs = apply_vtx_attr(&loc, ibo_attrs, 1)));
ret:
	glBindVertexArray(0);
	return mesh;
}
void mesh_deinit(struct mesh *self) {
	if (self->use_ebo) glDeleteBuffers(1, &self->ebo);
	if (self->use_ibo) glDeleteBuffers(1, &self->ibo);
	glDeleteBuffers(1, &self->vbo);
	glDeleteVertexArrays(1, &self->vao);
}
void mesh_upload_verts(struct mesh *self, const void *data, usize size) {
	glBindBuffer(GL_ARRAY_BUFFER, self->vbo);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}
void mesh_upload_indexes(struct mesh *self, const u32 *indexes, usize size) {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indexes, GL_STATIC_DRAW);
}
void mesh_upload_instances(struct mesh *self, const void *data, usize size) {
	glBindBuffer(GL_ARRAY_BUFFER, self->ibo);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}
u32 vtx_attr_num_locations(const struct vtx_attr *attr) {
	return (u32[]){
		1, 1, 1, // i32, u32, f32
		3, 4, // mat3, mat4
	}[attr->type];
}
bool vtx_attr_is_end(const struct vtx_attr *attr) {
	return !attr || !attr->len;
}

const struct vtx_attr *vtx_attrs_basic(void) {
	static const struct vtx_attr attrs[] = {
		GEN_VTX_ATTR(struct basic_vtx, pos, "apos"),
		GEN_VTX_ATTR(struct basic_vtx, norm, "anorm"),
		GEN_VTX_ATTR(struct basic_vtx, uv, "auv"),
		VTX_ATTR_END
	};
	return attrs;
}
const struct vtx_attr *ibo_attrs_quad(void) {
	static const struct vtx_attr attrs[] = {
		GEN_VTX_ATTR(struct quad_inst, world, "iworld"),
		GEN_VTX_ATTR(struct quad_inst, uv, "iuv"),
		VTX_ATTR_END
	};
	return attrs;
}
void quad_upload_vtxs(struct mesh *mesh) {
	static const struct basic_vtx vtxs[] = {
		// Top-left
		{
			.pos =  (vec3s){ .x = -0.5f, .y = 0.5f, .z = 0.0f },
			.norm = (vec3s){ .x = 0.0f,	 .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 0.0f,  .y = 1.0f },
		},
		// Top-right
		{
			.pos =  (vec3s){ .x = 0.5f, .y = 0.5f, .z = 0.0f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 1.0f, .y = 1.0f },
		},
		// Bottom-left
		{
			.pos =  (vec3s){ .x = -0.5f, .y = -0.5f, .z = 0.0f },
			.norm = (vec3s){ .x = 0.0f,  .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 0.0f,  .y = 0.0f },
		},
		// Bottom-right
		{
			.pos =  (vec3s){ .x = 0.5f, .y = -0.5f, .z = 0.0f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 1.0f, .y = 0.0f },
		},
	};
	
	static const u32 indexes[] = {
		0, 2, 1,
		1, 2, 3,
	};

	mesh_upload_verts(mesh, vtxs, sizeof(vtxs));
	mesh_upload_indexes(mesh, indexes, sizeof(indexes));
}

const struct vtx_attr *vtx_attrs_cube(void) {
	static const struct vtx_attr attrs[] = {
		GEN_VTX_ATTR(struct cube_vtx, pos, "apos"),
		GEN_VTX_ATTR(struct cube_vtx, norm, "anorm"),
		GEN_VTX_ATTR(struct cube_vtx, uv, "auv"),
		GEN_VTX_ATTR(struct cube_vtx, face, "afaceid"),
		VTX_ATTR_END
	};
	return attrs;
}
const struct vtx_attr *ibo_attrs_cube(void) {
	static const struct vtx_attr attrs[] = {
		GEN_VTX_ATTR(struct cube_inst, world, "iworld"),
		GEN_VTX_ATTR(struct cube_inst, pz, "ipz"),
		GEN_VTX_ATTR(struct cube_inst, nz, "inz"),
		GEN_VTX_ATTR(struct cube_inst, py, "ipy"),
		GEN_VTX_ATTR(struct cube_inst, ny, "iny"),
		GEN_VTX_ATTR(struct cube_inst, px, "ipx"),
		GEN_VTX_ATTR(struct cube_inst, nx, "inx"),
		VTX_ATTR_END
	};
	return attrs;
}
void cube_upload_vtxs(struct mesh *mesh) {
	static struct cube_vtx vtxs[] = {
		// PZ Top-left
		{
			.pos =  (vec3s){ .x = -0.5f, .y = 0.5f, .z = 0.5f },
			.norm = (vec3s){ .x = 0.0f,	 .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 0.0f,  .y = 1.0f },
			.face = FACE_PZ,
		},
		// PZ Top-right
		{
			.pos =  (vec3s){ .x = 0.5f, .y = 0.5f, .z = 0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 1.0f, .y = 1.0f },
			.face = FACE_PZ,
		},
		// PZ Bottom-left
		{
			.pos =  (vec3s){ .x = -0.5f, .y = -0.5f, .z = 0.5f },
			.norm = (vec3s){ .x = 0.0f,  .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 0.0f,  .y = 0.0f },
			.face = FACE_PZ,
		},
		// PZ Bottom-right
		{
			.pos =  (vec3s){ .x = 0.5f, .y = -0.5f, .z = 0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 1.0f, .y = 0.0f },
			.face = FACE_PZ,
		},

		// NZ Top-right
		{
			.pos =  (vec3s){ .x = 0.5f, .y = 0.5f, .z = -0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = -1.0f },
			.uv =   (vec2s){ .x = 1.0f, .y = 1.0f },
			.face = FACE_NZ,
		},
		// NZ Top-left
		{
			.pos =  (vec3s){ .x = -0.5f, .y = 0.5f, .z = -0.5f },
			.norm = (vec3s){ .x = 0.0f,	 .y = 0.0f, .z = -1.0f },
			.uv =   (vec2s){ .x = 0.0f,  .y = 1.0f },
			.face = FACE_NZ,
		},
		// NZ Bottom-right
		{
			.pos =  (vec3s){ .x = 0.5f, .y = -0.5f, .z = -0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = -1.0f },
			.uv =   (vec2s){ .x = 1.0f, .y = 0.0f },
			.face = FACE_NZ,
		},
		// NZ Bottom-left
		{
			.pos =  (vec3s){ .x = -0.5f, .y = -0.5f, .z = -0.5f },
			.norm = (vec3s){ .x = 0.0f,  .y = 0.0f, .z = -1.0f },
			.uv =   (vec2s){ .x = 0.0f,  .y = 0.0f },
			.face = FACE_NZ,
		},

		// PY Top-left
		{
			.pos =  (vec3s){ .x = -0.5f, .y = 0.5f, .z = -0.5f },
			.norm = (vec3s){ .x = 0.0f,	 .y = 0.0f, .z = -1.0f },
			.uv =   (vec2s){ .x = 0.0f,  .y = 0.0f },
			.face = FACE_PY,
		},
		// PY Top-right
		{
			.pos =  (vec3s){ .x = 0.5f, .y = 0.5f, .z = -0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = -1.0f },
			.uv =   (vec2s){ .x = 1.0f, .y = 0.0f },
			.face = FACE_PY,
		},
		// PY Bottom-left
		{
			.pos =  (vec3s){ .x = -0.5f, .y = 0.5f, .z = 0.5f },
			.norm = (vec3s){ .x = 0.0f,	 .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 0.0f,  .y = 1.0f },
			.face = FACE_PY,
		},
		// PY Bottom-right
		{
			.pos =  (vec3s){ .x = 0.5f, .y = 0.5f, .z = 0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 1.0f, .y = 1.0f },
			.face = FACE_PY,
		},

		// NY Top-right
		{
			.pos =  (vec3s){ .x = 0.5f, .y = -0.5f, .z = -0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = -1.0f },
			.uv =   (vec2s){ .x = 1.0f, .y = 0.0f },
			.face = FACE_NY,
		},
		// NY Top-left
		{
			.pos =  (vec3s){ .x = -0.5f, .y = -0.5f, .z = -0.5f },
			.norm = (vec3s){ .x = 0.0f,	 .y = 0.0f, .z = -1.0f },
			.uv =   (vec2s){ .x = 0.0f,  .y = 0.0f },
			.face = FACE_NY,
		},
		// NY Bottom-right
		{
			.pos =  (vec3s){ .x = 0.5f, .y = -0.5f, .z = 0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 1.0f, .y = 1.0f },
			.face = FACE_NY,
		},
		// NY Bottom-left
		{
			.pos =  (vec3s){ .x = -0.5f, .y = -0.5f, .z = 0.5f },
			.norm = (vec3s){ .x = 0.0f,	 .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 0.0f,  .y = 1.0f },
			.face = FACE_NY,
		},

		// PX Top-right
		{
			.pos =  (vec3s){ .x = 0.5f, .y = 0.5f, .z = 0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 1.0f, .y = 1.0f },
			.face = FACE_PX,
		},
		// PX Top-right
		{
			.pos =  (vec3s){ .x = 0.5f, .y = 0.5f, .z = -0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = -1.0f },
			.uv =   (vec2s){ .x = 0.0f, .y = 1.0f },
			.face = FACE_PX,
		},
		// PX Bottom-right
		{
			.pos =  (vec3s){ .x = 0.5f, .y = -0.5f, .z = 0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 1.0f, .y = 0.0f },
			.face = FACE_PX,
		},
		// PX Bottom-right
		{
			.pos =  (vec3s){ .x = 0.5f, .y = -0.5f, .z = -0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = -1.0f },
			.uv =   (vec2s){ .x = 0.0f, .y = 0.0f },
			.face = FACE_PX,
		},

		// NX Top-right
		{
			.pos =  (vec3s){ .x = -0.5f, .y = 0.5f, .z = -0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = -1.0f },
			.uv =   (vec2s){ .x = 0.0f, .y = 1.0f },
			.face = FACE_NX,
		},
		// NX Top-right
		{
			.pos =  (vec3s){ .x = -0.5f, .y = 0.5f, .z = 0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 1.0f, .y = 1.0f },
			.face = FACE_NX,
		},
		// NX Bottom-right
		{
			.pos =  (vec3s){ .x = -0.5f, .y = -0.5f, .z = -0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = -1.0f },
			.uv =   (vec2s){ .x = 0.0f, .y = 0.0f },
			.face = FACE_NX,
		},
		// NX Bottom-right
		{
			.pos =  (vec3s){ .x = -0.5f, .y = -0.5f, .z = 0.5f },
			.norm = (vec3s){ .x = 0.0f, .y = 0.0f, .z = 1.0f },
			.uv =   (vec2s){ .x = 1.0f, .y = 0.0f },
			.face = FACE_NX,
		},
	};
	
	static const u32 indexes[] = {
		// PZ
		0, 2, 1,
		1, 2, 3,
		// NZ
		4+0, 4+2, 4+1,
		4+1, 4+2, 4+3,
		// PY
		4*2+0, 4*2+2, 4*2+1,
		4*2+1, 4*2+2, 4*2+3,
		// NY
		4*3+0, 4*3+2, 4*3+1,
		4*3+1, 4*3+2, 4*3+3,
		// PY
		4*4+0, 4*4+2, 4*4+1,
		4*4+1, 4*4+2, 4*4+3,
		// NY
		4*5+0, 4*5+2, 4*5+1,
		4*5+1, 4*5+2, 4*5+3,
	};

	mesh_upload_verts(mesh, vtxs, sizeof(vtxs));
	mesh_upload_indexes(mesh, indexes, sizeof(indexes));
}

