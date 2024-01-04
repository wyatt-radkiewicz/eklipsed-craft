#include <glad/glad.h>

#include "camera.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "tools.h"
#include "window.h"

int main(int argc, char **argv) {
	struct window window;

	tools_init();
	if (!window_init(&window, "Eklipsed craft", (ivec2s){ .x = 800, .y = 600 })) {
		goto ret;
	}

	struct texinfo texinfo = texinfo_load("minecraft/pack.png");

	struct mesh quad_mesh = mesh_init(vtx_attrs_quad(), true, ibo_attrs_quad());
	struct quad_inst *quads = vector_init(*quads);
	quad_upload_vtxs(&quad_mesh);
	quads = vector_push(quads, (struct quad_inst) {
		.world = glms_mat4_identity(),
		.texmat = glms_mat3_identity(),
	});
	mesh_upload_instances(&quad_mesh, quads, vector_len(quads) * sizeof(*quads));

	struct camera camera = camera_init();
	camera.pos.z = 3;

	shader_t shader = shader_load("data/shaders/basic.vs", "data/shaders/basic.fs", vtx_attrs_quad(), ibo_attrs_quad());
	shader_set(shader, "utexture", 0);

	while (!window.should_close) {
		window_poll_events(&window);

		glViewport(0, 0, window.size.x, window.size.y);
		glClearColor(0.3f, 0.7f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shader);
		camera_set_uniforms(&camera, window_get_ratio(&window), shader);
		tex_bind(texinfo.tex, 0);
		glBindVertexArray(quad_mesh.vao);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL, vector_len(quads));
		SDL_GL_SwapWindow(window.sdl_window);
	}

	mesh_deinit(&quad_mesh);
	glDeleteTextures(1, &texinfo.tex);
	glDeleteProgram(shader);
ret:
	window_deinit(&window);
	tools_deinit();
	return 0;
}

