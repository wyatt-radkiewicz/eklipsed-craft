#include "camera.h"
#include "tools.h"
#include "window.h"
#include "vulkan_util.h"

int main(int argc, char **argv) {
	struct window window;

	VkPipeline pipeline = VK_NULL_HANDLE;
	VkPipelineLayout layout = VK_NULL_HANDLE;
	VkRenderPass rpass = VK_NULL_HANDLE;

	tools_init();
	if (!window_init(&window, "Eklipsed craft", (ivec2s){ .x = 800, .y = 600 })) goto ret;
	//window_set_lock_mouse(&window, true);
	
	if (!vk_generate_pipeline(
		window.vk_dev,
		&(struct vk_gfx_pipeline_cinfo){
			.scdata = &window.vk_scdata,
			.cull_faces = false,
			.depth_test = false,
			.clamp_depth = false,
			.multisamples = 1,
			.blend_state = BLEND_ALPHA,
		},
		&pipeline,
		&layout,
		&rpass,
		"shaders/test.vert.spv",
		"shaders/test.frag.spv"
	)) goto ret;

	struct camera camera = camera_init();
	camera.pos.z = 3;

	f64 dt = 0.0;
	while (!window.should_close) {
		const clock_t frame_start = clock();
		window_poll_events(&window);
		window_update_inputs(&window);

		camera.rot.y -= window.mouserel.x * 0.003f;
		camera.rot.x -= window.mouserel.y * 0.003f;
		const u8 *keys = SDL_GetKeyboardState(NULL);
		const i32 movement = keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S];
		const i32 lateral_movement = keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A];
		const vec3s forward = (vec3s) {
			.x = -sinf(camera.rot.y) * cosf(camera.rot.x),
			.y = sinf(camera.rot.x),
			.z = -cos(camera.rot.y) * cosf(camera.rot.x),
		};
		const vec3s side = glms_vec3_crossn(forward, (vec3s){ .x = 0.0f, .y = 1.0f, .z = 0.0f });
		camera.pos = glms_vec3_add(
			glms_vec3_add(camera.pos, glms_vec3_scale(forward, movement * 3.0f * dt)),
			glms_vec3_scale(side, lateral_movement * 2.5f * dt)
		);

		const clock_t frame_end = clock();
		dt = (f64)(frame_end - frame_start) / (f64)CLOCKS_PER_SEC;
	}

ret:
	vkDestroyPipeline(window.vk_dev, pipeline, NULL);
	vkDestroyPipelineLayout(window.vk_dev, layout, NULL);
	vkDestroyRenderPass(window.vk_dev, rpass, NULL);

	window_deinit(&window);
	tools_deinit();
	return 0;
}

