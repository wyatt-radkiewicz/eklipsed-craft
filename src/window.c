#include "tools.h"
#include "window.h"
#include "vulkan_util.h"

static bool _sdl2_inited = false;
static size_t _num_windows = 0;

static void window_handle_event(window_t *window, const SDL_Event *event);

static bool sdl2_init(void) {
	if (_sdl2_inited) return true;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("%s", SDL_GetError());
		return false;
	}

	_sdl2_inited = true;
	return true;
}

bool window_init(window_t *self, const char *title, ivec2s size) {
	if (!sdl2_init()) return false;

	*self = (window_t){ .size = size, .handlers = vector_init(*self->handlers) };
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

	if (!vk_instance_init(
		&self->vk_instance,
#ifdef DEBUG
		&self->vk_dbgmsgr,
#else
		NULL,
#endif
		self->sdl_window
	)) {
		printf("Couldn't init vulkan\n");
		return false;
	}
	if (!SDL_Vulkan_CreateSurface(self->sdl_window, self->vk_instance, &self->vk_window_surf)) {
		printf("Couldn't create the vulkan surface for the window!\n");
		return false;
	}
	if ((self->vk_phys = vk_get_physdev(self->vk_instance, self->vk_window_surf)) == VK_NULL_HANDLE) {
		printf("Couldn't find a physical device\n");
		return false;
	}
	if (!vk_get_queue_families(self->vk_phys, &self->vk_qf, self->vk_window_surf)) {
		printf("Couldn't find needed vulkan queue families!\n");
		return false;
	}
	if (!vk_dev_init(&self->vk_dev, self->vk_phys, &self->vk_qf)) {
		printf("Couldn't create logical vulkan device!\n");
		return false;
	}
	vkGetDeviceQueue(self->vk_dev, self->vk_qf.gfx, 0, &self->vk_gfxq);
	vkGetDeviceQueue(self->vk_dev, self->vk_qf.present, 0, &self->vk_presentq);
	if (!vk_swapchain_init(self->vk_dev, self->vk_phys, self->sdl_window, &self->vk_qf, self->vk_window_surf, &self->vk_scdata)) {
		printf("Couldn't create the vulkan swapchain!\n");
		return false;
	}

	_num_windows++;
	self->handlers = vector_push(self->handlers, window_handle_event);
	
	return true;
}
void window_deinit(window_t *self) {
	vk_swapchain_deinit(&self->vk_scdata, self->vk_dev);
	vkDestroySurfaceKHR(self->vk_instance, self->vk_window_surf, NULL);
	vkDestroyDevice(self->vk_dev, NULL);

	vk_instance_deinit(
		self->vk_instance,
#ifdef DEBUG
		&self->vk_dbgmsgr
#else
		NULL
#endif
	);

	SDL_DestroyWindow(self->sdl_window);
	vector_deinit(self->handlers);
	*self = (window_t){0};
	_num_windows--;

	if (_num_windows) return;
	SDL_Quit();
	_sdl2_inited = false;
}
void window_poll_events(window_t *self) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		for (uint32_t i = 0; i < vector_len(self->handlers); i++) {
			if (self->handlers[i]) self->handlers[i](self, &event);
		}
	}
}
static void window_handle_event(window_t *window, const SDL_Event *event) {
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
float window_get_ratio(window_t *self) {
	return (float)self->size.x / (float)self->size.y;
}
void window_set_lock_mouse(window_t *self, bool lock) {
	self->lock_mouse = lock;
	SDL_SetWindowMouseGrab(self->sdl_window, lock);
	SDL_SetRelativeMouseMode(lock);
}
void window_update_inputs(window_t *self) {
	if (self->lock_mouse) {
		SDL_GetRelativeMouseState(&self->mouserel.x, &self->mouserel.y);
	}
}

