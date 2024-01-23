#ifndef _VULKAN_UTIL_H
#define _VULKAN_UTIL_H

#include <vulkan/vulkan.h>
#include <SDL.h>
#include <SDL_vulkan.h>

#include "tools.h"

typedef struct vk_queue_families {
	uint32_t gfx;
	uint32_t present;
} vk_queue_families_t;

typedef struct vk_swapchain_data {
	VkSwapchainKHR sc;
	VkImage *vimgs;
	VkImageView *vviews;
	VkFormat fmt;
	VkExtent2D size;
	VkFramebuffer *vfbufs;
} vk_swapchain_data_t;

enum blend_state {
	BLEND_NONE,
	BLEND_ALPHA,
	BLEND_ADDITIVE,
};

typedef struct vk_gfx_pipeline_cinfo {
	uint8_t multisamples;
	enum blend_state blend_state;
	bool depth_test, clamp_depth, cull_faces;
	const vk_swapchain_data_t *scdata;

	// Add vertex info later on
} vk_gfx_pipeline_cinfo_t;

bool vk_instance_init(VkInstance *self, VkDebugUtilsMessengerEXT *dbgmsgr, SDL_Window *sdl_window);
void vk_instance_deinit(VkInstance self, VkDebugUtilsMessengerEXT *dbgmsgr);
VkPhysicalDevice vk_get_physdev(VkInstance inst, VkSurfaceKHR window_surf);
bool vk_get_queue_families(VkPhysicalDevice dev, vk_queue_families_t *qf, VkSurfaceKHR present_surf);
bool vk_dev_init(VkDevice *dev, VkPhysicalDevice phys, const vk_queue_families_t *qf);
bool vk_swapchain_init(VkDevice dev, VkPhysicalDevice phys, SDL_Window *target_window, const vk_queue_families_t *qf, VkSurfaceKHR window_surf, vk_swapchain_data_t *data);
void vk_swapchain_deinit(vk_swapchain_data_t *self, VkDevice dev);
bool vk_shadermodule_init(VkDevice dev, const char *binfile, VkShaderModule *shader);
bool vk_generate_pipeline(VkDevice dev, const vk_gfx_pipeline_cinfo_t *cinfo, VkPipeline *pipeline, VkPipelineLayout *layout, VkRenderPass *pass, const char *vbin, const char *fbin);
void vk_set_framebuffers_to_window_surface(VkDevice dev, vk_swapchain_data_t *data, VkRenderPass pass);

#define VK_PROCS \
	PROC(vkCreateDebugUtilsMessengerEXT) \
	PROC(vkDestroyDebugUtilsMessengerEXT)

#define PROC(proc) extern PFN_##proc fn##proc;
VK_PROCS
#undef PROC

#endif

