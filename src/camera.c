#include "camera.h"

struct camera camera_init(void) {
	return (struct camera) {
		.pos = glms_vec3_zero(),
		.rot = glms_vec2_zero(),
		.fov = 90.0f,
	};
}
mat4s camera_get_proj(const struct camera *camera, f32 ratio) {
	return glms_perspective(camera->fov / 180.0f * M_PI, ratio, 0.1f, 10000.0f);
}
mat4s camera_get_view(const struct camera *camera) {
	return glms_translate(glms_rotate_y(glms_rotate_x(glms_mat4_identity(), -camera->rot.x), -camera->rot.y), glms_vec3_negate(camera->pos));
}

