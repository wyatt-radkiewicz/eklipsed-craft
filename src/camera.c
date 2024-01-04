#include "camera.h"

struct camera camera_init(void) {
	return (struct camera) {
		.pos = glms_vec3_zero(),
		.rot = glms_vec2_zero(),
		.fov = 90.0f,
	};
}
mat4s camera_get_proj(const struct camera *camera, f32 ratio) {
	return glms_perspective(camera->fov / 180.0f * M_PI, ratio, 0.1f, 1000.0f);
}
mat4s camera_get_view(const struct camera *camera) {
	return glms_translate(glms_rotate_y(glms_rotate_x(glms_mat4_identity(), -camera->rot.x), -camera->rot.y), glms_vec3_negate(camera->pos));
}
void camera_set_uniforms(const struct camera *camera, f32 ratio, shader_t shader) {
	const mat4s proj = camera_get_proj(camera, ratio),
				view = camera_get_view(camera);
	glUseProgram(shader);
	shader_set(shader, "uproj", &proj);
	shader_set(shader, "uview", &view);
}

