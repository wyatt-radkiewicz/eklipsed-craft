#ifndef _CAMERA_H
#define _CAMERA_H

#include <cglm/struct.h>

#include "tools.h"

typedef struct camera {
	vec3s pos;
	vec2s rot;
	float fov;
} camera_t;

camera_t camera_init(void);
mat4s camera_get_proj(const camera_t *camera, float ratio);
mat4s camera_get_view(const camera_t *camera);

#endif

