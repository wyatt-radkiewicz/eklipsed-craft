#include "camera.h"
#include "tools.h"
#include "window.h"
#include "vulkan_util.h"

typedef struct renderer {
	VkPipeline pipeline;
	VkPipelineLayout layout;
	VkRenderPass rpass;
	VkCommandPool cmdpool;
	VkCommandBuffer gfxbuf;

	VkSemaphore imgavail;
	VkSemaphore renderfinished;
	VkFence inflight;
} renderer_t;

static bool renderer_init(renderer_t *self, window_t *window) {
	*self = (renderer_t){0};
	
	if (!vk_generate_pipeline(
		window->vk_dev,
		&(vk_gfx_pipeline_cinfo_t){
			.scdata = &window->vk_scdata,
			.cull_faces = false,
			.depth_test = false,
			.clamp_depth = false,
			.multisamples = 1,
			.blend_state = BLEND_ALPHA,
		},
		&self->pipeline,
		&self->layout,
		&self->rpass,
		"shaders/test.vert.spv",
		"shaders/test.frag.spv"
	)) return false;
	vk_set_framebuffers_to_window_surface(window->vk_dev, &window->vk_scdata, self->rpass);

	if (vkCreateCommandPool(window->vk_dev, &(VkCommandPoolCreateInfo){
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = window->vk_qf.gfx,
		},
		NULL,
		&self->cmdpool
	) != VK_SUCCESS) {
		printf("Error creating command pool.\n");
		return false;
	}
	if (vkAllocateCommandBuffers(window->vk_dev, &(VkCommandBufferAllocateInfo){
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandPool = self->cmdpool,
			.commandBufferCount = 1,
		},
		&self->gfxbuf
	) != VK_SUCCESS) return false;
	if (vkCreateSemaphore(window->vk_dev, &(VkSemaphoreCreateInfo){ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO }, NULL, &self->imgavail) != VK_SUCCESS
		|| vkCreateSemaphore(window->vk_dev, &(VkSemaphoreCreateInfo){ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO }, NULL, &self->renderfinished) != VK_SUCCESS
		|| vkCreateFence(window->vk_dev, &(VkFenceCreateInfo){ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
															  .flags = VK_FENCE_CREATE_SIGNALED_BIT }, NULL, &self->inflight) != VK_SUCCESS) {
		printf("Error creating gpu syncronization primitives.\n");
		return false;
	}

	return true;
}

static void renderer_deinit(renderer_t *self, const window_t *window) {
	vkDestroySemaphore(window->vk_dev, self->imgavail, NULL);
	vkDestroySemaphore(window->vk_dev, self->renderfinished, NULL);
	vkDestroyFence(window->vk_dev, self->inflight, NULL);
	vkDestroyCommandPool(window->vk_dev, self->cmdpool, NULL);
	vkDestroyPipeline(window->vk_dev, self->pipeline, NULL);
	vkDestroyPipelineLayout(window->vk_dev, self->layout, NULL);
	vkDestroyRenderPass(window->vk_dev, self->rpass, NULL);
}

static void renderer_record_cmdbuf(renderer_t *self, const window_t *window, uint32_t framebuffer) {
	vkBeginCommandBuffer(self->gfxbuf, &(VkCommandBufferBeginInfo){
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	});

	vkCmdBeginRenderPass(self->gfxbuf, &(VkRenderPassBeginInfo){
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.framebuffer = window->vk_scdata.vfbufs[framebuffer],
		.renderPass = self->rpass,
		.renderArea = (VkRect2D){
			.offset = (VkOffset2D){ .x = 0, .y = 0 },
			.extent = window->vk_scdata.size,
		},
		.clearValueCount = 1,
		.pClearValues = (VkClearValue[]){
			(VkClearValue){
				.color.float32 = { 0.1f, 0.6f, 1.0f, 1.0f },
			},
		},
	}, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(self->gfxbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, self->pipeline);

	vkCmdSetViewport(self->gfxbuf, 0, 1, (VkViewport[]){
		(VkViewport){
			.x = 0, .y = 0,
			.width = window->vk_scdata.size.width,
			.height = window->vk_scdata.size.height,
		},
	});

	vkCmdSetScissor(self->gfxbuf, 0, 1, (VkRect2D[]){
		(VkRect2D){
			.offset.x = 0, .offset.y = 0,
			.extent.width = window->vk_scdata.size.width,
			.extent.height = window->vk_scdata.size.height,
		},
	});

	vkCmdDraw(self->gfxbuf, 3, 1, 0, 0);

	vkCmdEndRenderPass(self->gfxbuf);

	vkEndCommandBuffer(self->gfxbuf);
}

static void renderer_frame(renderer_t *self, window_t *window) {
	vkWaitForFences(window->vk_dev, 1, &self->inflight, true, UINT64_MAX);
	vkResetFences(window->vk_dev, 1, &self->inflight);

	uint32_t currimg;
	vkAcquireNextImageKHR(window->vk_dev, window->vk_scdata.sc, UINT64_MAX, self->imgavail, VK_NULL_HANDLE, &currimg);
	vkResetCommandBuffer(self->gfxbuf, 0);
	renderer_record_cmdbuf(self, window, currimg);

	vkQueueSubmit(
		window->vk_gfxq,
		1,
		(VkSubmitInfo[]){
			(VkSubmitInfo){
				.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
				.commandBufferCount = 1,
				.pCommandBuffers = (VkCommandBuffer[]){ self->gfxbuf },
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = (VkSemaphore[]){ self->imgavail },
				.pWaitDstStageMask = (VkPipelineStageFlags[]){ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },
				.signalSemaphoreCount = 1,
				.pSignalSemaphores = (VkSemaphore[]){ self->renderfinished },
			},
		},
		self->inflight
	);

	vkQueuePresentKHR(window->vk_gfxq, &(VkPresentInfoKHR){
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = (VkSemaphore[]){ self->renderfinished },
		.pResults = NULL,
		.swapchainCount = 1,
		.pSwapchains = (VkSwapchainKHR[]){ window->vk_scdata.sc },
		.pImageIndices = (uint32_t[]){ currimg },
	});
}

int main(int argc, char **argv) {
	window_t window;

	tools_init();
	if (!window_init(&window, "Eklipsed craft", (ivec2s){ .x = 800, .y = 600 })) goto ret_early;
	//window_set_lock_mouse(&window, true);
	renderer_t renderer;
	if (!renderer_init(&renderer, &window)) goto ret;

	camera_t camera = camera_init();
	camera.pos.z = 3;

	double dt = 0.0;
	while (!window.should_close) {
		const clock_t frame_start = clock();
		window_poll_events(&window);
		window_update_inputs(&window);

		camera.rot.y -= window.mouserel.x * 0.003f;
		camera.rot.x -= window.mouserel.y * 0.003f;
		const uint8_t *keys = SDL_GetKeyboardState(NULL);
		const int32_t movement = keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S];
		const int32_t lateral_movement = keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A];
		const vec3s forward = (vec3s) {
			.x = -sinf(camera.rot.y) * cosf(camera.rot.x),
			.y = sinf(camera.rot.x),
			.z = -cos(camera.rot.y) * cosf(camera.rot.x),
		};
		const vec3s side = glms_vec3_crossn(forward, (vec3s){ .x = 0.0f, .y = 1.0f, .z = 0.0f });
		camera.pos = glms_vec3_add(
			glms_vec3_add(camera.pos, glms_vec3_scale(forward, movement * 3.0f * dt)),
			glms_vec3_scale(side, lateral_movement * 2.5f * dt)
		);

		renderer_frame(&renderer, &window);

		const clock_t frame_end = clock();
		dt = (double)(frame_end - frame_start) / (double)CLOCKS_PER_SEC;
	}

	vkDeviceWaitIdle(window.vk_dev);

ret:
	renderer_deinit(&renderer, &window);
ret_early:
	window_deinit(&window);
	tools_deinit();
	return 0;
}

