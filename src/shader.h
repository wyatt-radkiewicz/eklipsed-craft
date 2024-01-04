#ifndef _SHADER_H
#define _SHADER_H

#include <cglm/struct.h>
#include <glad/glad.h>

#include "tools.h"

struct vtx_attr;

typedef GLuint shader_t;

shader_t shader_load(const char *vs_path, const char *fs_path,
	const struct vtx_attr *vtx_attrs, const struct vtx_attr *ibo_attrs);
void shader_seti(shader_t self, const char *var, i32 val);
void shader_setf(shader_t self, const char *var, f32 val);
void shader_setv2(shader_t self, const char *var, vec2s val);
void shader_setv3(shader_t self, const char *var, vec3s val);
void shader_setv4(shader_t self, const char *var, vec4s val);
void shader_setm4(shader_t self, const char *var, const mat4s *val);
#define shader_set(self, var, val) (_Generic((val), \
	int : shader_seti, \
	float : shader_setf, \
	vec2s : shader_setv2, \
	vec3s : shader_setv3, \
	vec4s : shader_setv4, \
	mat4s * : shader_setm4, \
	const mat4s * : shader_setm4 \
)((self), (var), (val)))

#endif

