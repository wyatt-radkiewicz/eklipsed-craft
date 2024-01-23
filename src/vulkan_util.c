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
		uint32_t nexts;
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
		uint32_t nprops;
		vkEnumerateInstanceLayerProperties(&nprops, NULL);
		VkLayerProperties *props = malloc(sizeof(*props) * nprops);
		vkEnumerateInstanceLayerProperties(&nprops, props);
		
		for (uint32_t i = 0; i < arrlen(layers); i++) {
			bool found = false;
			for (uint32_t j = 0; j < nprops; j++) {
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
	const VkDebugUtilsMessengerCreateInfoEXT dbg_cinfo = (VkDebugUtilsMessengerCreateInfoEXT){
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

	const VkApplicationInfo appinfo = (VkApplicationInfo){
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "ecraft",
		.applicationVersion = VK_MAKE_VERSION(0, 1, 0),
		.pEngineName = "ecraft_emu",
		.engineVersion = VK_MAKE_VERSION(0, 1, 0),
		.apiVersion = VK_API_VERSION_1_0,
	};

	const VkInstanceCreateInfo cinfo = (VkInstanceCreateInfo){
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

static bool is_dev_useable(VkPhysicalDevice dev, VkSurfaceKHR window_surf) {
	bool has_queues = vk_get_queue_families(dev, NULL, window_surf);
	bool has_swapchain_support = false;

	uint32_t nexts;
	vkEnumerateDeviceExtensionProperties(dev, NULL, &nexts, NULL);
	VkExtensionProperties *exts = malloc(sizeof(*exts) * nexts);
	vkEnumerateDeviceExtensionProperties(dev, NULL, &nexts, exts);

	for (uint32_t i = 0; i < nexts; i++) {
		has_swapchain_support |= strcmp(exts[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0;
	}

	free(exts);

	if (!has_swapchain_support) return false;

	uint32_t len;
	vkGetPhysicalDeviceSurfaceFormatsKHR(dev, window_surf, &len, NULL);
	if (!len) return false;
	vkGetPhysicalDeviceSurfacePresentModesKHR(dev, window_surf, &len, NULL);
	if (!len) return false;

	return has_queues;
}

VkPhysicalDevice vk_get_physdev(VkInstance inst, VkSurfaceKHR window_surf) {
	uint32_t ndevs;
	vkEnumeratePhysicalDevices(inst, &ndevs, NULL);
	VkPhysicalDevice *devs = malloc(sizeof(*devs) * ndevs);
	vkEnumeratePhysicalDevices(inst, &ndevs, devs);

	VkPhysicalDevice dev = VK_NULL_HANDLE;
	for (uint32_t i = 0; i < ndevs; i++) {
		if (is_dev_useable(devs[i], window_surf)) {
			dev = devs[i];
			break;
		}
	}

	free(devs);
	return dev;
}
bool vk_get_queue_families(VkPhysicalDevice dev, vk_queue_families_t *qf, VkSurfaceKHR present_surf) {
	uint32_t nfams;
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &nfams, NULL);
	VkQueueFamilyProperties *fams = malloc(sizeof(*fams) * nfams);
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &nfams, fams);

	bool gfx_found = false, present_found = false;
	for (uint32_t i = 0; i < nfams; i++) {
		if (fams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			gfx_found = true;
			if (qf) qf->gfx = i;
		}

		if (present_surf != VK_NULL_HANDLE) continue;
		VkBool32 supports_surf;
		vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, present_surf, &supports_surf);
		if (supports_surf) {
			present_found = true;
			if (qf) qf->present = i;
		}
	}

	free(fams);

	return gfx_found || present_found;
}
bool vk_dev_init(VkDevice *dev, VkPhysicalDevice phys, const vk_queue_families_t *qf) {
	const VkDeviceCreateInfo cinfo = (VkDeviceCreateInfo){
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pQueueCreateInfos = (VkDeviceQueueCreateInfo[]){
			(VkDeviceQueueCreateInfo){
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = qf->gfx,
				.queueCount = 1,
				.pQueuePriorities = (float[]){ 1.0 },
			},
			(VkDeviceQueueCreateInfo){
				.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = qf->present,
				.queueCount = 1,
				.pQueuePriorities = (float[]){ 1.0 },
			}
		},
		.queueCreateInfoCount = 2,
#ifdef DEBUG
		.enabledLayerCount = 1,
		.ppEnabledLayerNames = (const char *[]){
			"VK_LAYER_KHRONOS_validation"
		},
#else
		.enabledLayerCount = 0,
#endif
		.ppEnabledExtensionNames = (const char *[]){
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef __APPLE__
			"VK_KHR_portability_subset",
#endif
		},
#ifdef __APPLE__
		.enabledExtensionCount = 2,
#else
		.enabledExtensionCount = 1,
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

bool vk_swapchain_init(VkDevice dev, VkPhysicalDevice phys, SDL_Window *target_window, const vk_queue_families_t *qf, VkSurfaceKHR window_surf, vk_swapchain_data_t *data) {
	bool ret = false;

	// Get the details of the physical device's swapchain support
	uint32_t nformats;
	vkGetPhysicalDeviceSurfaceFormatsKHR(phys, window_surf, &nformats, NULL);
	VkSurfaceFormatKHR *formats = malloc(sizeof(*formats) * nformats);
	vkGetPhysicalDeviceSurfaceFormatsKHR(phys, window_surf, &nformats, formats);

	uint32_t nmodes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(phys, window_surf, &nmodes, NULL);
	VkPresentModeKHR *modes = malloc(sizeof(*modes) * nmodes);
	vkGetPhysicalDeviceSurfacePresentModesKHR(phys, window_surf, &nmodes, modes);

	VkSurfaceCapabilitiesKHR caps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys, window_surf, &caps);

	// Choose best formats and modes
	if (!nformats || !nmodes) goto cleanup;
	VkSurfaceFormatKHR chosen_format = formats[0];
	for (uint32_t i = 0; i < nformats; i++) {
		if (formats[i].colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) continue;
		if (formats[i].format == VK_FORMAT_R8G8B8A8_SRGB) chosen_format = formats[i];
	}
	VkPresentModeKHR chosen_mode = modes[0];
	for (uint32_t i = 0; i < nmodes; i++) {
		if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) chosen_mode = modes[i];
	}
	VkExtent2D size;
	if (caps.currentExtent.width == (uint32_t)-1 || caps.currentExtent.height == (uint32_t)-1) {
		size = caps.currentExtent;
	} else {
		int x, y;
		SDL_GetWindowSizeInPixels(target_window, &x, &y);
		size = (VkExtent2D){
			.width = clamp(x, caps.minImageExtent.width, caps.maxImageExtent.width),
			.height = clamp(y, caps.minImageExtent.height, caps.maxImageExtent.height),
		};
	}
	uint32_t nimgs = caps.maxImageCount == 0 ? caps.minImageCount + 1 : min(caps.minImageCount + 1, caps.maxImageCount);

	// Use those to create a swapchain
	const VkSwapchainCreateInfoKHR cinfo = (VkSwapchainCreateInfoKHR){
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.minImageCount = nimgs,
		.imageFormat = chosen_format.format,
		.imageColorSpace = chosen_format.colorSpace,
		.imageExtent = size,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		
		.imageSharingMode = qf->gfx == qf->present ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
		.queueFamilyIndexCount = qf->gfx == qf->present ? 0 : 2,
		.pQueueFamilyIndices = qf->gfx == qf->present ? NULL : (uint32_t[]){ qf->gfx, qf->present },

		.preTransform = caps.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = chosen_mode,
		.clipped = VK_TRUE,
		.surface = window_surf,
		.oldSwapchain = VK_NULL_HANDLE,
	};

	if (vkCreateSwapchainKHR(dev, &cinfo, NULL, &data->sc) != VK_SUCCESS) {
		printf("Couldn't create the swapchain!\n");
		goto cleanup;
	}

	data->size = size;
	data->fmt = chosen_format.format;
	data->vimgs = vector_init(*data->vimgs);
	vkGetSwapchainImagesKHR(dev, data->sc, &nimgs, NULL);
	data->vimgs = vector_resize(data->vimgs, nimgs);
	vkGetSwapchainImagesKHR(dev, data->sc, &nimgs, data->vimgs);

	data->vviews = vector_init(*data->vviews);
	data->vviews = vector_resize(data->vviews, vector_len(data->vimgs));
	for (uint32_t i = 0; i < vector_len(data->vviews); i++) {
		const VkImageViewCreateInfo cinfo = (VkImageViewCreateInfo){
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = data->vimgs[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = data->fmt,
			.components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
			.components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
			.components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
			.components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
			.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.subresourceRange.baseMipLevel = 0,
			.subresourceRange.baseArrayLayer = 0,
			.subresourceRange.layerCount = 1,
			.subresourceRange.levelCount = 1,
		};
		if (vkCreateImageView(dev, &cinfo, NULL, &data->vviews[i])) {
			printf("Couldn't create a view into the swap chain images!\n");
			goto cleanup;
		}
	}
	data->vfbufs = NULL;

	ret = true;
cleanup:
	free(formats);
	free(modes);
	return ret;
}
void vk_swapchain_deinit(vk_swapchain_data_t *self, VkDevice dev) {
	for (uint32_t i = 0; i < vector_len(self->vviews); i++) {
		if (self->vfbufs) vkDestroyFramebuffer(dev, self->vfbufs[i], NULL);
		vkDestroyImageView(dev, self->vviews[i], NULL);
	}
	vector_deinit(self->vimgs);
	vector_deinit(self->vviews);
	if (self->vfbufs) vector_deinit(self->vfbufs);
	vkDestroySwapchainKHR(dev, self->sc, NULL);
}

void vk_set_framebuffers_to_window_surface(VkDevice dev, vk_swapchain_data_t *data, VkRenderPass pass) {
	if (!data->vfbufs) data->vfbufs = vector_init(*data->vfbufs);
	data->vfbufs = vector_resize(data->vfbufs, vector_len(data->vimgs));

	for (uint32_t i = 0; i < vector_len(data->vfbufs); i++) {
		if (vkCreateFramebuffer(dev, &(VkFramebufferCreateInfo){
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = pass,
			.attachmentCount = 1,
			.pAttachments = (VkImageView[]){
				data->vviews[i],
			},
			.width = data->size.width,
			.height = data->size.height,
			.layers = 1,
		}, NULL, data->vfbufs + i) != VK_SUCCESS) {
			printf("Couldn't create framebuffers.\n");
			return;
		}
	}
}

bool vk_shadermodule_init(VkDevice dev, const char *binfile, VkShaderModule *shader) {
	uint8_t *data;
	const size_t datalen = loadfileb(binfile, &data);
	if (!datalen) return false;

	VkShaderModuleCreateInfo cinfo = (VkShaderModuleCreateInfo){
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pCode = (const uint32_t *)data,
		.codeSize = datalen,
	};

	if (vkCreateShaderModule(dev, &cinfo, NULL, shader) != VK_SUCCESS) {
		printf("Couldn't create the shader module from the file \"%s\"!\n", binfile);
		free(data);
		return false;
	}

	free(data);
	return true;
}

bool vk_generate_pipeline(
	VkDevice dev,
	const vk_gfx_pipeline_cinfo_t *cinfo,
	VkPipeline *pipeline,
	VkPipelineLayout *layout,
	VkRenderPass *pass,
	const char *vbin,
	const char *fbin
) {
	if (vkCreatePipelineLayout(dev, &(VkPipelineLayoutCreateInfo){
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 0,
		.pSetLayouts = NULL,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = NULL,
	}, NULL, layout) != VK_SUCCESS) {
		printf("Error creating pipeline layout!\n");
		return false;
	}

	if (vkCreateRenderPass(dev, &(VkRenderPassCreateInfo){
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = (VkAttachmentDescription[]){
			(VkAttachmentDescription){
				.format = cinfo->scdata->fmt,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			},
		},
		.subpassCount = 1,
		.pSubpasses = (VkSubpassDescription[]){
			(VkSubpassDescription){
				.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
				.colorAttachmentCount = 1,
				.pColorAttachments = (VkAttachmentReference[]) {
					(VkAttachmentReference){
						.attachment = 0,
						.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					},
				},
			},
		},
		.dependencyCount = 1,
		.pDependencies = (VkSubpassDependency[]){
			(VkSubpassDependency){
				.srcSubpass = VK_SUBPASS_EXTERNAL,
				.dstSubpass = 0,
				.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.srcAccessMask = 0,
				.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			},
		},
	}, NULL, pass) != VK_SUCCESS) {
		printf("Error creating render pass!\n");
		return false;
	}

	VkShaderModule vert, frag;
	if (!vk_shadermodule_init(dev, vbin, &vert)) {
		printf("Couldn't create the vertex shader.\n");
		return false;
	}
	if (!vk_shadermodule_init(dev, fbin, &frag)) {
		vkDestroyShaderModule(dev, vert, NULL);
		printf("Couldn't create the fragment shader.\n");
		return false;
	}

	bool ret = false;
	if (vkCreateGraphicsPipelines(
		dev,
		VK_NULL_HANDLE,
		1,
		(VkGraphicsPipelineCreateInfo[]){
			(VkGraphicsPipelineCreateInfo){
				.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
				.stageCount = 2,
				.pStages = (VkPipelineShaderStageCreateInfo[]){
					(VkPipelineShaderStageCreateInfo){
						.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
						.stage = VK_SHADER_STAGE_VERTEX_BIT,
						.module = vert,
						.pName = "main",
					},
					(VkPipelineShaderStageCreateInfo){
						.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
						.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
						.module = frag,
						.pName = "main",
					},
				},
				.layout = *layout,
				.renderPass = *pass,
				.subpass = 0,
				.basePipelineHandle = VK_NULL_HANDLE,
				.basePipelineIndex = 0,
				.pInputAssemblyState = &(VkPipelineInputAssemblyStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
					.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
					.primitiveRestartEnable = false,
				},
				.pVertexInputState = &(VkPipelineVertexInputStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
					.vertexAttributeDescriptionCount = 0,
					.pVertexAttributeDescriptions = NULL,
					.vertexBindingDescriptionCount = 0,
					.pVertexBindingDescriptions = NULL,
				},
				.pViewportState = &(VkPipelineViewportStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
					.viewportCount = 1,
					.scissorCount = 1,
				},
				.pDynamicState = &(VkPipelineDynamicStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
					.dynamicStateCount = 2,
					.pDynamicStates = (VkDynamicState[]){
						VK_DYNAMIC_STATE_VIEWPORT,
						VK_DYNAMIC_STATE_SCISSOR,
					},
				},
				.pMultisampleState = &(VkPipelineMultisampleStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
					.sampleShadingEnable = false,
					.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
					.minSampleShading = 1.0f,
					.pSampleMask = NULL,
					.alphaToCoverageEnable = false,
					.alphaToOneEnable = false,
				},
				.pRasterizationState = &(VkPipelineRasterizationStateCreateInfo){
					.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
					.depthClampEnable = false,
					.rasterizerDiscardEnable = false,
					.lineWidth = 1.0f,
					.polygonMode = VK_POLYGON_MODE_FILL,
					.cullMode = VK_CULL_MODE_BACK_BIT,
					.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
					.depthBiasEnable = false,
					.depthBiasConstantFactor = 0.0f,
					.depthBiasClamp = 0.0f,
					.depthBiasSlopeFactor = 0.0f,
				},
				.pDepthStencilState = NULL,
				.pColorBlendState = &(VkPipelineColorBlendStateCreateInfo) {
					.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
					.logicOpEnable = false,
					.logicOp = VK_LOGIC_OP_COPY,
					.attachmentCount = 1,
					.pAttachments = (VkPipelineColorBlendAttachmentState[]){
						(VkPipelineColorBlendAttachmentState){
							.colorWriteMask =	VK_COLOR_COMPONENT_R_BIT |
												VK_COLOR_COMPONENT_G_BIT |
												VK_COLOR_COMPONENT_B_BIT |
												VK_COLOR_COMPONENT_A_BIT,
							.blendEnable = false,
							.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
							.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
							.colorBlendOp = VK_BLEND_OP_ADD,
							.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
							.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
							.alphaBlendOp = VK_BLEND_OP_ADD,
						},
					},
					.blendConstants[0] = 1.0f,
					.blendConstants[1] = 1.0f,
					.blendConstants[2] = 1.0f,
					.blendConstants[3] = 1.0f,
				},
			},
		},
		NULL,
		pipeline
	) != VK_SUCCESS) goto cleanup;
	ret = true;
cleanup:
	vkDestroyShaderModule(dev, vert, NULL);
	vkDestroyShaderModule(dev, frag, NULL);
	return ret;
}

