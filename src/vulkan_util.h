#ifndef _VULKAN_UTIL_H
#define _VULKAN_UTIL_H

#include <vulkan/vulkan.h>
#include <SDL.h>
#include <SDL_vulkan.h>

#include "tools.h"

struct vk_queue_families {
	u32 gfx;
	u32 present;
};

struct vk_swapchain_data {
	VkSwapchainKHR sc;
	VkImage *vimgs;
	VkImageView *vviews;
	VkFormat fmt;
	VkExtent2D size;
};

enum blend_state {
	BLEND_NONE,
	BLEND_ALPHA,
	BLEND_ADDITIVE,
};

struct vk_gfx_pipeline_cinfo {
	u8 multisamples;
	enum blend_state blend_state;
	bool depth_test, clamp_depth, cull_faces;
	const struct vk_swapchain_data *scdata;

	// Add vertex info later on
};

bool vk_instance_init(VkInstance *self, VkDebugUtilsMessengerEXT *dbgmsgr, SDL_Window *sdl_window);
void vk_instance_deinit(VkInstance self, VkDebugUtilsMessengerEXT *dbgmsgr);
VkPhysicalDevice vk_get_physdev(VkInstance inst, VkSurfaceKHR window_surf);
bool vk_get_queue_families(VkPhysicalDevice dev, struct vk_queue_families *qf, VkSurfaceKHR present_surf);
bool vk_dev_init(VkDevice *dev, VkPhysicalDevice phys, const struct vk_queue_families *qf);
bool vk_swapchain_init(VkDevice dev, VkPhysicalDevice phys, SDL_Window *target_window, const struct vk_queue_families *qf, VkSurfaceKHR window_surf, struct vk_swapchain_data *data);
void vk_swapchain_deinit(struct vk_swapchain_data *self, VkDevice dev);
bool vk_shadermodule_init(VkDevice dev, const char *binfile, VkShaderModule *shader);
bool vk_generate_pipeline(VkDevice dev, const struct vk_gfx_pipeline_cinfo *cinfo, VkPipeline *pipeline, VkPipelineLayout *layout, VkRenderPass *pass, const char *vbin, const char *fbin);

#define VK_PROCS \
	PROC(vkCreateDebugUtilsMessengerEXT) \
	PROC(vkDestroyDebugUtilsMessengerEXT)

#define PROC(proc) extern PFN_##proc fn##proc;
VK_PROCS
#undef PROC

#endif

