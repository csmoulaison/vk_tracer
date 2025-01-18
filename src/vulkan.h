#ifndef vulkan_h_INCLUDED
#define vulkan_h_INCLUDED

#include "vulkan/vulkan.h"

struct {
	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkDevice device;

	VkQueue queue_compute;
	VkQueue queue_present;
} typedef VulkanState;

VulkanState new_vulkan_state();
void destroy_vulkan_state(VulkanState* vk);

#endif // vulkan_h_INCLUDED
