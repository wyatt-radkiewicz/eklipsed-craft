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
	if (!window_init(&window, "Eklipsed craft", (ivec2s){ .x = 800, .y = 600 })) goto ret;
	window_set_lock_mouse(&window, true);

	struct texinfo texinfo = texinfo_load("minecraft/pack.png");

	struct mesh quad_mesh = mesh_init(vtx_attrs_basic(), true, ibo_attrs_quad());
	struct quad_inst *quads = vector_init(*quads);
	quad_upload_vtxs(&quad_mesh);
	quads = vector_push(quads, (struct quad_inst) {
		.world = glms_mat4_identity(),
		.uv = (vec4s){ .x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 1.0f},
	});
	quads = vector_push(quads, (struct quad_inst) {
		.world = glms_translate(glms_mat4_identity(), (vec3s){ .x = 1.0f, .y = 1.0f, .z = -2.0f }),
		.uv = (vec4s){ .x = 0.0f, .y = 0.0f, .z = 3.0f, .w = 2.0f},
	});
	mesh_upload_instances(&quad_mesh, quads, vector_len(quads) * sizeof(*quads));

	struct mesh cube_mesh = mesh_init(vtx_attrs_cube(), true, ibo_attrs_cube());
	struct cube_inst *cubes = vector_init(*cubes);
	cube_upload_vtxs(&cube_mesh);
	cubes = vector_push(cubes, (struct cube_inst) {
		.world = glms_rotate_z(glms_mat4_identity(), torad(45.0f)),
		.pz = (vec4s){ .x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 1.0f, },
		.nz = (vec4s){ .x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 1.0f, },
		.py = (vec4s){ .x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 1.0f, },
		.ny = (vec4s){ .x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 1.0f, },
		.px = (vec4s){ .x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 1.0f, },
		.nx = (vec4s){ .x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 1.0f, },
	});
	cubes = vector_push(cubes, (struct cube_inst) {
		.world = glms_translate(glms_mat4_identity(), (vec3s){ .x = -4.0f, .y = 0.0f, .z = 1.0f }),
		.pz = (vec4s){ .x = 0.0f, .y = 0.5f, .z = 1.0f, .w = 0.5f, },
		.nz = (vec4s){ .x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 1.0f, },
		.py = (vec4s){ .x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 1.0f, },
		.ny = (vec4s){ .x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 1.0f, },
		.px = (vec4s){ .x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 1.0f, },
		.nx = (vec4s){ .x = 0.0f, .y = 0.0f, .z = 1.0f, .w = 1.0f, },
	});
	mesh_upload_instances(&cube_mesh, cubes, vector_len(cubes) * sizeof(*cubes));

	struct camera camera = camera_init();
	camera.pos.z = 3;

	shader_t shader = shader_load("data/shaders/basic.vs", "data/shaders/basic.fs", vtx_attrs_basic(), ibo_attrs_quad());
	shader_set(shader, "utexture", 0);

	shader_t cube_shader = shader_load("data/shaders/cube.vs", "data/shaders/basic.fs", vtx_attrs_cube(), ibo_attrs_cube());
	shader_set(cube_shader, "utexture", 0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

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

		glViewport(0, 0, window.size.x, window.size.y);
		glClearColor(0.3f, 0.7f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glUseProgram(shader);
		camera_set_uniforms(&camera, window_get_ratio(&window), shader);
		tex_bind(texinfo.tex, 0);
		glBindVertexArray(quad_mesh.vao);
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL, vector_len(quads));

		glUseProgram(cube_shader);
		camera_set_uniforms(&camera, window_get_ratio(&window), cube_shader);
		tex_bind(texinfo.tex, 0);
		glBindVertexArray(cube_mesh.vao);
		glDrawElementsInstanced(GL_TRIANGLES, 6*6, GL_UNSIGNED_INT, NULL, vector_len(cubes));

		SDL_GL_SwapWindow(window.sdl_window);
		const clock_t frame_end = clock();
		dt = (f64)(frame_end - frame_start) / (f64)CLOCKS_PER_SEC;
	}

	vector_deinit(quads);
	vector_deinit(cubes);
	mesh_deinit(&quad_mesh);
	mesh_deinit(&cube_mesh);
	glDeleteTextures(1, &texinfo.tex);
	glDeleteProgram(shader);
ret:
	window_deinit(&window);
	tools_deinit();
	return 0;
}

