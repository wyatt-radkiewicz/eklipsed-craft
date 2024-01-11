#include "vulkan_util.h"

#define PROC(proc) PFN_##proc fn##proc;
VK_PROCS
#undef PROC

static VKAPI_ATTR VkBool32 VKAPI_CALL dbgmsg_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* usrdata
) {
	if (severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) return VK_FALSE;
	printf("validation layer: %s\n", data->pMessage);
    return VK_FALSE;
}

static bool vk_load_procs(VkInstance inst) {
#define PROC(proc) \
	fn##proc = (PFN_##proc)vkGetInstanceProcAddr(inst, #proc); \
	if (fn##proc == NULL) { \
		printf("Couldn't find vulkan function: " #proc "\n"); \
		return false; \
	}
VK_PROCS
#undef PROC
	return true;
}

bool vk_instance_init(VkInstance *self, VkDebugUtilsMessengerEXT *dbgmsgr, SDL_Window *sdl_window) {
	bool ret = true;
	const char **vnames = vector_init(const char *);

	{
		u32 nexts;
		SDL_Vulkan_GetInstanceExtensions(sdl_window, &nexts, NULL);
		vnames = vector_resize(vnames, nexts);
		SDL_Vulkan_GetInstanceExtensions(sdl_window, &nexts, vnames);
#ifdef __APPLE__
		vnames = vector_push(vnames, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		vnames = vector_push(vnames, "VK_KHR_get_physical_device_properties2");
#endif
#ifdef DEBUG
		vnames = vector_push(vnames, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
	}

#ifdef DEBUG
	const char *layers[] = {
		"VK_LAYER_KHRONOS_validation",
	};
	{
		// Check for layer support
		u32 nprops;
		vkEnumerateInstanceLayerProperties(&nprops, NULL);
		VkLayerProperties *props = malloc(sizeof(*props) * nprops);
		vkEnumerateInstanceLayerProperties(&nprops, props);
		
		for (u32 i = 0; i < arrlen(layers); i++) {
			bool found = false;
			for (u32 j = 0; j < nprops; j++) {
				found |= strcmp(props[j].layerName, layers[i]) == 0;
			}
			if (found) continue;

			// Couldn't find the layer, so we can't initialize this fucker
			ret = false;
			free(props);
			goto cleanup;
		}

		free(props);
	}
#endif

#ifdef DEBUG
	VkDebugUtilsMessengerCreateInfoEXT dbg_cinfo = (VkDebugUtilsMessengerCreateInfoEXT){
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = dbgmsg_callback,
	};
#endif

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
#ifdef DEBUG
		.ppEnabledLayerNames = layers,
		.enabledLayerCount = arrlen(layers),
		.pNext = &dbg_cinfo,
#else
		.enabledLayerCount = 0,
#endif
		.flags = 0
#ifdef __APPLE__
			| VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
#endif
		,
	};

	if (vkCreateInstance(&cinfo, NULL, self) != VK_SUCCESS) {
		printf("Couldn't create the instance\n");
		ret = false;
		goto cleanup;
	}

	if (!vk_load_procs(*self)) {
		printf("Couldn't load the functions\n");
		ret = false;
		goto cleanup;
	}

#ifdef DEBUG
	if (fnvkCreateDebugUtilsMessengerEXT(*self, &dbg_cinfo, NULL, dbgmsgr) != VK_SUCCESS) {
		printf("Couldn't create the debug messenger for vulkan\n");
		ret = false;
		goto cleanup;
	}
#else
	assert(!dbgmsgr);
#endif

cleanup:
	vector_deinit(vnames);
	return ret;
}

void vk_instance_deinit(VkInstance self, VkDebugUtilsMessengerEXT *dbgmsgr) {
#ifdef DEBUG
	fnvkDestroyDebugUtilsMessengerEXT(self, *dbgmsgr, NULL);
#else
	assert(!dbgmsgr);
#endif

	vkDestroyInstance(self, NULL);
}

static bool is_dev_useable(VkPhysicalDevice dev) {
	return vk_get_queue_families(dev, NULL);
}

VkPhysicalDevice vk_get_physdev(VkInstance inst) {
	u32 ndevs;
	vkEnumeratePhysicalDevices(inst, &ndevs, NULL);
	VkPhysicalDevice *devs = malloc(sizeof(*devs) * ndevs);
	vkEnumeratePhysicalDevices(inst, &ndevs, devs);

	VkPhysicalDevice dev = VK_NULL_HANDLE;
	for (u32 i = 0; i < ndevs; i++) {
		if (is_dev_useable(devs[i])) {
			dev = devs[i];
			break;
		}
	}

	free(devs);
	return dev;
}
bool vk_get_queue_families(VkPhysicalDevice dev, struct vk_queue_families *qf) {
	u32 nfams;
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &nfams, NULL);
	VkQueueFamilyProperties *fams = malloc(sizeof(*fams) * nfams);
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &nfams, fams);

	bool gfx_found = false;
	for (u32 i = 0; i < nfams; i++) {
		if (fams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			gfx_found = true;
			if (qf) qf->gfx = i;
		}
	}

	free(fams);

	return gfx_found;
}
bool vk_dev_init(VkDevice *dev, VkPhysicalDevice phys, const struct vk_queue_families *qf) {
	VkDeviceCreateInfo cinfo = (VkDeviceCreateInfo){
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pQueueCreateInfos = (VkDeviceQueueCreateInfo[]){
			(VkDeviceQueueCreateInfo){
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = qf->gfx,
				.queueCount = 1,
				.pQueuePriorities = (float[]){ 1.0 },
			}
		},
		.queueCreateInfoCount = 1,
#ifdef DEBUG
		.enabledLayerCount = 1,
		.ppEnabledLayerNames = (const char *[]){
			"VK_LAYER_KHRONOS_validation"
		},
#else
		.enabledLayerCount = 0,
#endif
		.ppEnabledExtensionNames = (const char *[]){
#ifdef __APPLE__
			"VK_KHR_portability_subset",
#endif
		},
#ifdef __APPLE__
		.enabledExtensionCount = 1,
#else
		.enabledExtensionCount = 0,
#endif
		.pEnabledFeatures = &(VkPhysicalDeviceFeatures){
			
		},
	};

	if (vkCreateDevice(phys, &cinfo, NULL, dev) != VK_SUCCESS) {
		printf("Couldn't create vulkan logical device!\n");
		return false;
	}

	return true;
}

