#ifndef _CAMERA_H
#define _CAMERA_H

#include <cglm/struct.h>

#include "shader.h"
#include "tools.h"

struct camera {
	vec3s pos;
	vec2s rot;
	f32 fov;
};

struct camera camera_init(void);
mat4s camera_get_proj(const struct camera *camera, f32 ratio);
mat4s camera_get_view(const struct camera *camera);
void camera_set_uniforms(const struct camera *camera, f32 ratio, shader_t shader);

#endif

