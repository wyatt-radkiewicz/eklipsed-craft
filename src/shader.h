#ifndef _SHADER_H
#define _SHADER_H

#include <cglm/struct.h>
#include <glad/glad.h>

typedef GLuint shader_t;

shader_t shader_load(const char *vs_path, const char *fs_path);
void shader_seti(shader_t self, const char *var, int val);
void shader_setf(shader_t self, const char *var, float val);
void shader_setv2(shader_t self, const char *var, vec2s val);
void shader_setv3(shader_t self, const char *var, vec3s val);
void shader_setv4(shader_t self, const char *var, vec4s val);
void shader_setm4(shader_t self, const char *var, const mat4s *val);
#define shader_set(shader, var, val) (_Generic((val), \
	int : shader_seti((shader), (var), (val)), \
	float : shader_setf((shader), (var), (val)), \
	vec2s : shader_setv2((shader), (var), (val)), \
	vec3s : shader_setv3((shader), (var), (val)), \
	vec4s : shader_setv4((shader), (var), (val)), \
	mat4s : shader_setm4((shader), (var), &(val)), \
)

#endif

