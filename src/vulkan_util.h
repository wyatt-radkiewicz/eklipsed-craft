#ifndef _VULKAN_UTIL_H
#define _VULKAN_UTIL_H

#include <vulkan/vulkan.h>
#include <SDL_vulkan.h>

#include "tools.h"

bool vk_instance_init(VkInstance *self, SDL_Window *sdl_window);
void vk_instance_deinit(VkInstance self);

#endif

