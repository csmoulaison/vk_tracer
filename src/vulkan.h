#ifndef vulkan_h_INCLUDED
#define vulkan_h_INCLUDED

#include "vulkan/vulkan.h"
#include "window.h"

// TODO Ensure we don't go over this during swapchain creation
#define SWAP_IMAGES_MAX 3

struct {
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physical_device;
	VkDevice device;

	VkQueue queue_compute;
	VkQueue queue_present;

	VkSwapchainKHR swapchain;
	VkImage swap_images[SWAP_IMAGES_MAX];
	VkImageView swap_views[SWAP_IMAGES_MAX];
	uint32_t images_len;
} typedef VulkanState;

VulkanState new_vulkan_state(Window* window);
void destroy_vulkan_state(VulkanState* vk);
const char** window_instance_extensions();

#endif // vulkan_h_INCLUDED
