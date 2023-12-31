#include <stb_image.h>

#include "ecraft.h"

char *loadfile(const char *path, size_t *slen) {
	FILE *fp = fopen(path, "rb");
	if (!fp) return NULL;
	if (fseek(fp, 0, SEEK_END) != 0) return NULL;
	long len = ftell(fp);
	if (len == -1) return NULL;
	if (slen) *slen = len;
	char *str = malloc(len + 1);
	if (!str) return NULL;
	if (fseek(fp, 0, SEEK_SET) != 0) return NULL;
	if (fread(str, len, 1, fp) != 1) {
		free(str);
		return NULL;
	}
	str[len] = '\0';
	return str;
}

static GLuint compileshader(const char *spath, GLint type) {
	const char *const src = loadfile(spath, NULL);
	if (!src) return 0;
	GLuint shader = glCreateShader(type);
	if (!shader) return 0;
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
		return 0;
	}

	return shader;
}
GLuint loadshader(const char *vspath, const char *fspath) {
	GLuint vert = compileshader(vspath, GL_VERTEX_SHADER),
		   frag = compileshader(fspath, GL_FRAGMENT_SHADER);
	if (!vert || !frag) {
		if (vert) glDeleteShader(vert);
		if (frag) glDeleteShader(frag);
		return 0;
	}

	GLuint shader = glCreateProgram();
	glAttachShader(shader, vert);
	glAttachShader(shader, frag);
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
		return 0;
	}

	glDetachShader(shader, vert);
	glDetachShader(shader, frag);
	return shader;
}

struct texinfo loadtexture(const char *path) {
	struct texinfo texinfo;

	glGenTextures(1, &texinfo.tex);
    glBindTexture(GL_TEXTURE_2D, texinfo.tex); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    stbi_set_flip_vertically_on_load(true);

	int num_channels;
    unsigned char *data = stbi_load(path, &texinfo.width, &texinfo.height, &num_channels, 4);
    if (!data) {
		glDeleteTextures(1, &texinfo.tex);
		return (struct texinfo){0};
	}

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texinfo.width, texinfo.height, 0, num_channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
	return texinfo;
}

void shader_set_int(GLuint shader, const char *var, int val) {
	glUniform1i(glGetUniformLocation(shader, var), val);
}
void shader_set_float(GLuint shader, const char *var, float val) {
	glUniform1f(glGetUniformLocation(shader, var), val);
}
void shader_set_vec2(GLuint shader, const char *var, vec2s val) {
	glUniform2f(glGetUniformLocation(shader, var), val.x, val.y);
}
void shader_set_vec3(GLuint shader, const char *var, const vec3s *val) {
	glUniform3f(glGetUniformLocation(shader, var), val->x, val->y, val->z);
}
void shader_set_vec4(GLuint shader, const char *var, const vec4s *val) {
	glUniform4f(glGetUniformLocation(shader, var), val->x, val->y, val->z, val->w);
}
void shader_set_mat4(GLuint shader, const char *var, const mat4s *val) {
	glUniformMatrix4fv(glGetUniformLocation(shader, var), 1, GL_FALSE, (void *)val->raw);
}

ivec2s get_window_size(void) {
	ivec2s v;
	SDL_GetWindowSizeInPixels(g_window, &v.x, &v.y);
	return v;
}
ivec2s get_window_sizept(void) {
	ivec2s v;
	SDL_GetWindowSize(g_window, &v.x, &v.y);
	return v;
}
ivec2s get_mouse_pos(void) {
	ivec2s v;
	SDL_GetMouseState(&v.x, &v.y);
	return v;
}

struct camera make_camera(void) {
	return (struct camera) {
		.pos = glms_vec3_zero(),
		.rot = glms_vec2_zero(),
		.fov = 90.0f,
	};
}
mat4s camera_get_proj(const struct camera *camera) {
	return glms_perspective(camera->fov / 180.0f * M_PI, (float)get_window_size().x / (float)get_window_size().y, 0.1f, 1000.0f);
}
mat4s camera_get_view(const struct camera *camera) {
	return glms_translate(glms_rotate_y(glms_rotate_x(glms_mat4_identity(), -camera->rot.x), -camera->rot.y), glms_vec3_negate(camera->pos));
}
void camera_set_uniforms(const struct camera *camera, GLuint shader) {
	const mat4s proj = camera_get_proj(camera),
				view = camera_get_view(camera);
	glUseProgram(shader);
	shader_set_mat4(shader, "u_proj", &proj);
	shader_set_mat4(shader, "u_view", &view);
}

