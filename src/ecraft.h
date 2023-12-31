#ifndef _ecraft_h
#define _ecraft_h
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <cglm/struct.h>
#include <glad/glad.h>
#include <SDL.h>

//
// Tools and utilities
//
#define arrsize(arr) (sizeof(arr)/sizeof((arr)[0]))

char *loadfile(const char *path, size_t *slen);
GLuint loadshader(const char *vspath, const char *fspath);

struct texinfo {
	GLuint tex;
	int width, height;
};
struct texinfo loadtexture(const char *path);
void shader_set_int(GLuint shader, const char *var, int val);
void shader_set_float(GLuint shader, const char *var, float val);
void shader_set_vec2(GLuint shader, const char *var, vec2s val);
void shader_set_vec3(GLuint shader, const char *var, const vec3s *val);
void shader_set_vec4(GLuint shader, const char *var, const vec4s *val);
void shader_set_mat4(GLuint shader, const char *var, const mat4s *val);

ivec2s get_window_size(void);
ivec2s get_window_sizept(void);
ivec2s get_mouse_pos(void);

struct camera {
	vec3s pos;
	vec2s rot;
	float fov;
};
struct camera make_camera(void);
mat4s camera_get_proj(const struct camera *camera);
mat4s camera_get_view(const struct camera *camera);
void camera_set_uniforms(const struct camera *camera, GLuint shader);

#define torad(degrees) ((degrees)/180.0f*M_PI)
#define todeg(radians) ((radians)/M_PI*180.0f)

//
// Window and context creation
//
extern SDL_Window *g_window;
extern SDL_GLContext *g_glcontext;
bool window_init(void);
void window_deinit(void);

//
// Rendering
//
struct quadvert {
	vec3s pos, normal;
	vec2s uv;
};
struct quad_instance {
	mat4s mat;
	vec2s uv_pos;
	vec2s uv_size;
};
struct mesh {
	GLuint vao, vbo, ebo, ibo;
};
struct mesh gen_instanced_quad(void);
void delete_mesh(struct mesh *mesh);
void mesh_buffer_verts(struct mesh *mesh, const void *data, size_t size);
void mesh_buffer_elements(struct mesh *mesh, const unsigned int *data, size_t size);
void mesh_buffer_instances(struct mesh *mesh, const void *data, size_t size);

#endif

