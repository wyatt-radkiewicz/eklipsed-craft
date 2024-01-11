#ifndef _VULKAN_UTIL_H
#define _VULKAN_UTIL_H

#include <vulkan/vulkan.h>
#include <SDL_vulkan.h>

#include "tools.h"

struct vk_queue_families {
	u32 gfx;
};

bool vk_instance_init(VkInstance *self, VkDebugUtilsMessengerEXT *dbgmsgr, SDL_Window *sdl_window);
void vk_instance_deinit(VkInstance self, VkDebugUtilsMessengerEXT *dbgmsgr);
VkPhysicalDevice vk_get_physdev(VkInstance inst);
bool vk_get_queue_families(VkPhysicalDevice dev, struct vk_queue_families *qf);
bool vk_dev_init(VkDevice *dev, VkPhysicalDevice phys, const struct vk_queue_families *qf);

#define VK_PROCS \
	PROC(vkCreateDebugUtilsMessengerEXT) \
	PROC(vkDestroyDebugUtilsMessengerEXT)

#define PROC(proc) extern PFN_##proc fn##proc;
VK_PROCS
#undef PROC

#endif

