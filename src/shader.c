#include "mesh.h"
#include "shader.h"

static GLuint compile_shader(const char *spath, GLint type) {
	const char *const src = loadfile(spath);
	if (!src) return 0;

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLint maxlen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxlen);
		char *str = malloc(maxlen);
		glGetShaderInfoLog(shader, maxlen, NULL, str);
		printf("shader compilation error: %s", str);
		glDeleteShader(shader);
		free(str);
		return 0;
	}

	return shader;
}

shader_t shader_load(const char *vs_path, const char *fs_path,
	const struct vtx_attr *vtx_attrs, const struct vtx_attr *ibo_attrs) {
	GLuint vert = compile_shader(vs_path, GL_VERTEX_SHADER),
		   frag = compile_shader(fs_path, GL_FRAGMENT_SHADER);
	if (!vert || !frag) {
		if (vert) glDeleteShader(vert);
		if (frag) glDeleteShader(frag);
		return 0;
	}

	GLuint shader = glCreateProgram();
	glAttachShader(shader, vert);
	glAttachShader(shader, frag);

	// Set the attrib locations
	u32 loc = 0;
	while (!vtx_attr_is_end(vtx_attrs)) {
		glBindAttribLocation(shader, loc, vtx_attrs->name);
		loc += vtx_attr_num_locations(vtx_attrs++);
	}
	while (!vtx_attr_is_end(ibo_attrs)) {
		glBindAttribLocation(shader, loc, ibo_attrs->name);
		loc += vtx_attr_num_locations(ibo_attrs++);
	}

	glLinkProgram(shader);

	GLint linked;
	glGetProgramiv(shader, GL_LINK_STATUS, &linked);
	if (!linked) {
		GLint maxlen;
		glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &maxlen);
		char *str = malloc(maxlen);
		glGetProgramInfoLog(shader, maxlen, NULL, str);
		printf("shader linking error: %s", str);
		glDeleteProgram(shader);
		glDeleteShader(vert);
		glDeleteShader(frag);
		free(str);
		return 0;
	}

	glDetachShader(shader, vert);
	glDetachShader(shader, frag);
	return shader;
}
void shader_log_validation(shader_t self) {
	GLint status, maxlen;
	glGetProgramiv(self, GL_VALIDATE_STATUS, &status);
	if (status) return;
	glGetProgramiv(self, GL_INFO_LOG_LENGTH, &maxlen);
	char *str = malloc(maxlen);
	glGetProgramInfoLog(self, maxlen, NULL, str);
	printf("shader validation error: %s\n", str);
	free(str);
}
void shader_seti(shader_t self, const char *var, i32 val) {
	glUniform1i(glGetUniformLocation(self, var), val);
}
void shader_setf(shader_t self, const char *var, f32 val) {
	glUniform1f(glGetUniformLocation(self, var), val);
}
void shader_setv2(shader_t self, const char *var, vec2s val) {
	glUniform2f(glGetUniformLocation(self, var), val.x, val.y);
}
void shader_setv3(shader_t self, const char *var, vec3s val) {
	glUniform3f(glGetUniformLocation(self, var), val.x, val.y, val.z);
}
void shader_setv4(shader_t self, const char *var, vec4s val) {
	glUniform4f(glGetUniformLocation(self, var), val.x, val.y, val.z, val.w);
}
void shader_setm4(shader_t self, const char *var, const mat4s *val) {
	glUniformMatrix4fv(glGetUniformLocation(self, var), 1, GL_FALSE, (void *)&val->raw);
}

