#include "vulkan_util.h"

bool vk_instance_init(VkInstance *self, SDL_Window *sdl_window) {
	bool ret = true;
	const char **vnames = vector_init(const char *);

	{
		u32 nexts;
		SDL_Vulkan_GetInstanceExtensions(sdl_window, &nexts, NULL);
		vnames = vector_resize(vnames, nexts);
		SDL_Vulkan_GetInstanceExtensions(sdl_window, &nexts, vnames);
#ifdef __APPLE__
		vnames = vector_push(vnames, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
	}

	VkApplicationInfo appinfo = (VkApplicationInfo){
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "ecraft",
		.applicationVersion = VK_MAKE_VERSION(0, 1, 0),
		.pEngineName = "ecraft_emu",
		.engineVersion = VK_MAKE_VERSION(0, 1, 0),
		.apiVersion = VK_API_VERSION_1_0,
	};

	VkInstanceCreateInfo cinfo = (VkInstanceCreateInfo){
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appinfo,
		.enabledExtensionCount = vector_len(vnames),
		.ppEnabledExtensionNames = vnames,
		.enabledLayerCount = 0,
		.flags = 0
#ifdef __APPLE__
			| VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
#endif
		,
	};

	if (vkCreateInstance(&cinfo, NULL, self) != VK_SUCCESS) {
		ret = false;
		goto cleanup;
	}

cleanup:
	vector_deinit(vnames);
	return ret;
}

void vk_instance_deinit(VkInstance self) {
	vkDestroyInstance(self, NULL);
}

