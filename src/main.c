#include "ecraft.h"

int main(int argc, char **argv) {
	if (!window_init()) {
		window_deinit();
		return -1;
	}

	struct mesh quad_instances = gen_instanced_quad();
	struct texinfo texinfo = loadtexture("minecraft/pack.png");
	GLuint shader = loadshader("data/shaders/basic.vs", "data/shaders/basic.fs");

	{
		glUseProgram(shader);
		shader_set_int(shader, "u_texture", 0);

		mat4s mat = glms_mat4_identity();
		shader_set_mat4(shader, "u_proj", &mat);
		shader_set_mat4(shader, "u_view", &mat);

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
		
		glViewport(0, 0, get_window_size().x, get_window_size().y);
		glClearColor(0.4f, 0.8f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shader);
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

