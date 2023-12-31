#include "ecraft.h"

//
// Window and context creation
//
static bool _sdl2_inited;
SDL_Window *g_window;
SDL_GLContext *g_glcontext;

bool window_init(void) {
	_sdl2_inited = true;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("%s", SDL_GetError());
		_sdl2_inited = false;
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	g_window = SDL_CreateWindow(
		"Eklipsed Craft",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		800, 600,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
	);
	if (!g_window) {
		printf("%s", SDL_GetError());
		return false;
	}

	g_glcontext = SDL_GL_CreateContext(g_window);
	if (!g_glcontext) {
		printf("%s", SDL_GetError());
		return false;
	}

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		printf("glad: cannot load opengl functions!\n");
		return false;
	}

	glViewport(0, 0, 800, 600);

	return true;
}

void window_deinit(void) {
	if (g_glcontext) SDL_GL_DeleteContext(g_glcontext);
	if (g_window) SDL_DestroyWindow(g_window);
	if (_sdl2_inited) SDL_Quit();
}

//
// Rendering
//
struct mesh gen_instanced_quad(void) {
	struct mesh mesh = {0};
	glGenVertexArrays(1, &mesh.vao);
	glGenBuffers(1, &mesh.vbo);
	glGenBuffers(1, &mesh.ibo);
	glGenBuffers(1, &mesh.ebo);

	glBindVertexArray(mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct quadvert), (void *)offsetof(struct quadvert, pos));
	glEnableVertexAttribArray(0);
	glVertexAttribDivisor(0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct quadvert), (void *)offsetof(struct quadvert, normal));
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 0);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct quadvert), (void *)offsetof(struct quadvert, uv));
	glEnableVertexAttribArray(2);
	glVertexAttribDivisor(2, 0);

	static const struct quadvert verts[] = {
		{ // Top left
			.pos = { .x = -0.5f, .y = 0.5f, .z = 0.0f },
			.normal = { .x = 0.0f, .y = 0.0f, .z = 1.0f },
			.uv = { .x = 0.0f, .y = 1.0f },
		},
		{ // Top right
			.pos = { .x = 0.5f, .y = 0.5f, .z = 0.0f },
			.normal = { .x = 0.0f, .y = 0.0f, .z = 1.0f },
			.uv = { .x = 1.0f, .y = 1.0f },
		},
		{ // Bottom left
			.pos = { .x = -0.5f, .y = -0.5f, .z = 0.0f },
			.normal = { .x = 0.0f, .y = 0.0f, .z = 1.0f },
			.uv = { .x = 0.0f, .y = 0.0f },
		},
		{ // Bottom right
			.pos = { .x = 0.5f, .y = -0.5f, .z = 0.0f },
			.normal = { .x = 0.0f, .y = 0.0f, .z = 1.0f },
			.uv = { .x = 1.0f, .y = 0.0f },
		},
	};
	mesh_buffer_verts(&mesh, verts, sizeof(verts));
	static const unsigned int elements[] = {
		0, 2, 1,
		1, 2, 3,
	};
	mesh_buffer_elements(&mesh, elements, sizeof(elements));

	glBindBuffer(GL_ARRAY_BUFFER, mesh.ibo);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(struct quad_instance), (void *)(sizeof(vec4) * 0));
	glEnableVertexAttribArray(3);
	glVertexAttribDivisor(3, 1);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(struct quad_instance), (void *)(sizeof(vec4) * 1));
	glEnableVertexAttribArray(4);
	glVertexAttribDivisor(4, 1);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(struct quad_instance), (void *)(sizeof(vec4) * 2));
	glEnableVertexAttribArray(5);
	glVertexAttribDivisor(5, 1);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(struct quad_instance), (void *)(sizeof(vec4) * 3));
	glEnableVertexAttribArray(6);
	glVertexAttribDivisor(6, 1);
	glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, sizeof(struct quad_instance), (void *)offsetof(struct quad_instance, uv_pos));
	glEnableVertexAttribArray(7);
	glVertexAttribDivisor(7, 1);
	glVertexAttribPointer(8, 2, GL_FLOAT, GL_FALSE, sizeof(struct quad_instance), (void *)offsetof(struct quad_instance, uv_size));
	glEnableVertexAttribArray(8);
	glVertexAttribDivisor(8, 1);
	mesh_buffer_instances(&mesh, NULL, sizeof(struct quad_instance));

	glBindVertexArray(0);

	return mesh;
}

void mesh_buffer_verts(struct mesh *mesh, const void *data, size_t size) {
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}
void mesh_buffer_elements(struct mesh *mesh, const unsigned int *data, size_t size) {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}
void mesh_buffer_instances(struct mesh *mesh, const void *data, size_t size) {
	glBindBuffer(GL_ARRAY_BUFFER, mesh->ibo);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

void delete_mesh(struct mesh *mesh) {
	if (mesh->vao) glDeleteVertexArrays(1, &mesh->vao);
	if (mesh->vbo) glDeleteBuffers(1, &mesh->vbo);
	if (mesh->ibo) glDeleteBuffers(1, &mesh->ibo);
	if (mesh->ebo) glDeleteBuffers(1, &mesh->ebo);
	*mesh = (struct mesh){0};
}

