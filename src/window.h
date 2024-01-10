#ifndef _WINDOW_H
#define _WINDOW_H

#include <stdbool.h>

#include <cglm/struct.h>
#include <SDL.h>
#include <vulkan/vulkan.h>

#include "tools.h"

struct window;
typedef void(*window_event_handler_t)(struct window *window, const SDL_Event *event);
struct window {
	SDL_Window *sdl_window;

	bool should_close, lock_mouse;
	ivec2s size, mousepos, mouserel;
	window_event_handler_t *handlers;

	VkInstance vk_instance;
};

bool window_init(struct window *self, const char *title, ivec2s size);
void window_deinit(struct window *self);
void window_poll_events(struct window *self);
f32 window_get_ratio(struct window *self);
void window_set_lock_mouse(struct window *self, bool lock);
void window_update_inputs(struct window *self);

#endif

