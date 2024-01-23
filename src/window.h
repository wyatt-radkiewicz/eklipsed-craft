#ifndef _WINDOW_H
#define _WINDOW_H

#include <stdbool.h>

#include <cglm/struct.h>
#include <SDL.h>

#include "tools.h"
#include "vulkan_util.h"

typedef struct window window_t;
typedef void(*window_event_handler_t)(window_t *window, const SDL_Event *event);
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
	vk_queue_families_t vk_qf;
	VkQueue vk_gfxq;
	VkQueue vk_presentq;
	VkSurfaceKHR vk_window_surf;
	vk_swapchain_data_t vk_scdata;
};

bool window_init(window_t *self, const char *title, ivec2s size);
void window_deinit(window_t *self);
void window_poll_events(window_t *self);
float window_get_ratio(window_t *self);
void window_set_lock_mouse(window_t *self, bool lock);
void window_update_inputs(window_t *self);

#endif

