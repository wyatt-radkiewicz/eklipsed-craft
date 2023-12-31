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
	camera.pos.z = 2.0f;
	camera.pos.x = 2.0f;
	camera.rot.y = torad(-15.0f);

	{
		glUseProgram(shader);
		shader_set_int(shader, "u_texture", 0);
		camera_set_uniforms(&camera, shader);

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

	bool runloop = true;
	while (runloop) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				runloop = false;
				break;
			}
		}

		if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_LEFT]) camera.rot.y -= 0.01f;
		if (SDL_GetKeyboardState(NULL)[SDL_SCANCODE_RIGHT]) camera.rot.y += 0.01f;
		
		glViewport(0, 0, get_window_size().x, get_window_size().y);
		glClearColor(0.4f, 0.8f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shader);
		camera_set_uniforms(&camera, shader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texinfo.tex);
		glBindVertexArray(quad_instances.vao);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL, 2);
		SDL_GL_SwapWindow(g_window);
	}

	glDeleteProgram(shader);
	glDeleteTextures(1, &texinfo.tex);
	delete_mesh(&quad_instances);

	window_deinit();
	return 0;
}

