#include "ecraft.h"

int main(int argc, char **argv) {
	if (!window_init()) {
		window_deinit();
		return -1;
	}

	struct mesh quad_instances = gen_instanced_quad();
	struct texinfo texinfo = loadtexture("minecraft/pack.png");
	GLuint shader = loadshader("data/shaders/basic.vs", "data/shaders/basic.fs");
	struct camera camera = make_camera();

	{
		glUseProgram(shader);
		shader_set_int(shader, "u_texture", 0);

		camera.pos.z = 2;

		struct quad_instance instances[2] = {
			{
				.mat = glms_scale_uni(glms_mat4_identity(), 0.4f),
				.uv_pos = glms_vec2_zero(),
				.uv_size = glms_vec2_one(),
			},
			{
				.mat = glms_scale_uni(glms_translate(glms_mat4_identity(), (vec3s){ .x = 0.0f, .y = 0.75f, .z = 0.0f }), 0.25f),
				.uv_pos = glms_vec2_zero(),
				.uv_size = glms_vec2_one(),
			},
		};
		mesh_buffer_instances(&quad_instances, instances, sizeof(instances));
	}

	double dt = 0.0;
	bool runloop = true;
	while (runloop) {
		const clock_t start = clock();
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				runloop = false;
				break;
			}
		}

		// Move camera
		const uint8_t *keys = SDL_GetKeyboardState(NULL);
		const vec3s forward = (vec3s){
			.x = -sinf(camera.rot.y) * cosf(camera.rot.x),
			.y = sinf(camera.rot.x),
			.z = -cosf(camera.rot.y) * cosf(camera.rot.x),
		};
		const vec3s side = glms_vec3_crossn(forward, (vec3s){ .x = 0.0f, .y = 1.0f, .z = 0.0f });
		const float speed = 1.5f;
		camera.pos = glms_vec3_add(camera.pos, glms_vec3_mul(forward, glms_vec3_fill((float)(keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S]) * speed * dt)));
		camera.pos = glms_vec3_add(camera.pos, glms_vec3_mul(side, glms_vec3_fill((float)(keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]) * speed * dt)));

		const float sens = 1.0f / 1000.0f;
		camera.rot.y -= (get_mouse_pos().x - (get_window_sizept().x / 2.0f)) * sens;
		camera.rot.x -= (get_mouse_pos().y - (get_window_sizept().y / 2.0f)) * sens;
		SDL_WarpMouseInWindow(g_window, get_window_sizept().x / 2, get_window_sizept().y / 2);

		camera_set_uniforms(&camera, shader);
		
		glViewport(0, 0, get_window_size().x, get_window_size().y);
		glClearColor(0.4f, 0.8f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texinfo.tex);
		glBindVertexArray(quad_instances.vao);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL, 2);
		SDL_GL_SwapWindow(g_window);
		dt = (double)(clock() - start) / (double)CLOCKS_PER_SEC;
	}

	glDeleteProgram(shader);
	glDeleteTextures(1, &texinfo.tex);
	delete_mesh(&quad_instances);

	window_deinit();
	return 0;
}

