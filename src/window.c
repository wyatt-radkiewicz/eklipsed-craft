#include <glad/glad.h>

#include "tools.h"
#include "window.h"

static bool _sdl2_inited = false;
static bool _glad_loaded = false;
static usize _num_windows = 0;

static void window_handle_event(struct window *window, const SDL_Event *event);

static bool sdl2_init(void) {
	if (_sdl2_inited) return true;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("%s", SDL_GetError());
		return false;
	}

	_sdl2_inited = true;
	return true;
}

bool window_init(struct window *self, const char *title, ivec2s size) {
	if (!sdl2_init()) return false;

	*self = (struct window){ .size = size, .handlers = vector_init(*self->handlers) };
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	self->sdl_window = SDL_CreateWindow(
		title,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		self->size.x, self->size.y,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
	);
	if (!self->sdl_window) {
		printf("%s", SDL_GetError());
		return false;
	}

	self->gl_context = SDL_GL_CreateContext(self->sdl_window);
	if (!self->gl_context) {
		printf("%s", SDL_GetError());
		return false;
	}
	SDL_GL_MakeCurrent(self->sdl_window, self->gl_context);

	if (!_glad_loaded && !gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		printf("glad: cannot load opengl functions!\n");
		return false;
	}
	_glad_loaded = true;

	glViewport(0, 0, self->size.x, self->size.y);

	_num_windows++;
	self->handlers = vector_push(self->handlers, window_handle_event);
	
	return true;
}
void window_deinit(struct window *self) {
	SDL_GL_DeleteContext(self->gl_context);
	SDL_DestroyWindow(self->sdl_window);
	vector_deinit(self->handlers);
	*self = (struct window){0};
	_num_windows--;

	if (_num_windows) return;
	SDL_Quit();
	_sdl2_inited = false;
}
void window_poll_events(struct window *self) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		for (u32 i = 0; i < vector_len(self->handlers); i++) {
			if (self->handlers[i]) self->handlers[i](self, &event);
		}
	}
}
static void window_handle_event(struct window *window, const SDL_Event *event) {
	switch (event->type) {
	case SDL_QUIT:
		window->should_close = true;
		break;
	case SDL_WINDOWEVENT:
		if (event->window.event != SDL_WINDOWEVENT_RESIZED) break;
		window->size.x = event->window.data1;
		window->size.y = event->window.data2;
		break;
	case SDL_MOUSEMOTION:
		window->mousepos.x = event->motion.x;
		window->mousepos.y = event->motion.y;
		window->mousevel.x = event->motion.xrel;
		window->mousevel.y = event->motion.yrel;
		break;
	}

	if (window->lock_mouse) SDL_WarpMouseInWindow(window->sdl_window, window->size.x / 2, window->size.y / 2);
}
f32 window_get_ratio(struct window *self) {
	return (f32)self->size.x / (f32)self->size.y;
}

