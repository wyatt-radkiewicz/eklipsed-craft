#include "tools.h"
#include "window.h"

static bool _sdl2_inited = false;
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
	self->sdl_window = SDL_CreateWindow(
		title,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		self->size.x, self->size.y,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
	);
	if (!self->sdl_window) {
		printf("%s", SDL_GetError());
		return false;
	}

	_num_windows++;
	self->handlers = vector_push(self->handlers, window_handle_event);
	
	return true;
}
void window_deinit(struct window *self) {
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
		break;
	}
}
f32 window_get_ratio(struct window *self) {
	return (f32)self->size.x / (f32)self->size.y;
}
void window_set_lock_mouse(struct window *self, bool lock) {
	self->lock_mouse = lock;
	SDL_SetWindowMouseGrab(self->sdl_window, lock);
	SDL_SetRelativeMouseMode(lock);
}
void window_update_inputs(struct window *self) {
	if (self->lock_mouse) {
		SDL_GetRelativeMouseState(&self->mouserel.x, &self->mouserel.y);
	}
}

