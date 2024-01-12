#ifndef _WINDOW_H
#define _WINDOW_H

#include <stdbool.h>

#include <cglm/struct.h>
#include <SDL.h>

#include "tools.h"
#include "vulkan_util.h"

struct window;
typedef void(*window_event_handler_t)(struct window *window, const SDL_Event *event);
struct window {
	SDL_Window *sdl_window;

	bool should_close, lock_mouse;
	ivec2s size, mousepos, mouserel;
	window_event_handler_t *handlers;

	VkInstance vk_instance;
#ifdef DEBUG
	VkDebugUtilsMessengerEXT vk_dbgmsgr;
#endif
	VkPhysicalDevice vk_phys;
	VkDevice vk_dev;
	struct vk_queue_families vk_qf;
	VkQueue vk_gfxq;
	VkQueue vk_presentq;
	VkSurfaceKHR vk_window_surf;
	struct vk_swapchain_data vk_scdata;
};

bool window_init(struct window *self, const char *title, ivec2s size);
void window_deinit(struct window *self);
void window_poll_events(struct window *self);
f32 window_get_ratio(struct window *self);
void window_set_lock_mouse(struct window *self, bool lock);
void window_update_inputs(struct window *self);

#endif

