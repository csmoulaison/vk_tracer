#include "vulkan.h"

#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "stdlib.h"

#include "exit_codes.h"
#include "app_name.h"

VulkanState new_vulkan_state() {
	VulkanState vk;

	// Create instance
	{
		VkApplicationInfo app;
		app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app.pNext = NULL;
		app.pApplicationName = APP_NAME;
		app.applicationVersion = 1;
		app.pEngineName = NULL;
		app.engineVersion = 0;
		app.apiVersion = VK_API_VERSION_1_3;
		
		VkInstanceCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		info.pNext = NULL;
		info.flags = 0;
		info.pApplicationInfo = &app;
		info.enabledLayerCount = 0; // TODO: validation layers
		info.ppEnabledLayerNames = NULL;
		info.enabledExtensionCount = 0;
		info.ppEnabledExtensionNames = NULL;

		if(vkCreateInstance(&info, NULL, &vk.instance) != VK_SUCCESS) {
			printf("Could not create instance.\n");
		}
	}

	// Create physical device
	uint32_t compute_family_index;
	{
		uint32_t devices_len;
		if(vkEnumeratePhysicalDevices(vk.instance, &devices_len, NULL) != VK_SUCCESS) {
			printf("Could not enumerate physical devices to get device count.\n");
		}
		VkPhysicalDevice devices[devices_len];
		if(vkEnumeratePhysicalDevices(vk.instance, &devices_len, devices) != VK_SUCCESS) {
			printf("Could not enumerate physical devices to get devices.\n");
		}

		vk.physical_device = NULL;
		for(int i = 0; i < devices_len; i++) {
			// Not needed: Get device properties
			// VkPhysicalDeviceProperties props;
			// vkGetPhysicalDeviceProperties(devices[i], &props);

			// Check queue families
			uint32_t fams_len;
			vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &fams_len, NULL);
			VkQueueFamilyProperties fams[fams_len];
			vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &fams_len, fams);

			bool compute = false;
			for(int j = 0; j < fams_len; j++) {
				if(fams[j].queueFlags & VK_QUEUE_COMPUTE_BIT) {
					compute = true;
					compute_family_index = j;
				}
			}
			if(!compute) {
				continue;
			}

			// Check device extensions
			uint32_t exts_len;
			vkEnumerateDeviceExtensionProperties(devices[i], NULL, &exts_len, NULL);
			VkExtensionProperties exts[exts_len];
			vkEnumerateDeviceExtensionProperties(devices[i], NULL, &exts_len, exts);

			bool swapchain = false;
			for(int j = 0; j < exts_len; j++) {
				if(strcmp(exts[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
					swapchain = true;
				}
			}
			if(!swapchain) {
				continue;
			}

		vk.physical_device = devices[i];
		}

		// Exit if we haven't found an eligible device
		if(vk.physical_device == NULL) {
			printf("No suitable physical device.\n");
			exit(ERR_NO_SUITABLE_PHYSICAL_DEVICE);
		}
	}

	// Create logical device
	{ 
		VkDeviceQueueCreateInfo queue;
		queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue.pNext = NULL;
		queue.flags = 0;
		queue.queueFamilyIndex = compute_family_index;
		queue.queueCount = 1; // 1 because only 1 queue, right?
		float priority = 1.0f;
	 	queue.pQueuePriorities = &priority;
		
		VkDeviceCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	 	info.pNext = NULL;
	 	info.flags = 0;
	 	info.queueCreateInfoCount = 1;
	 	info.pQueueCreateInfos = &queue;
	 	info.enabledExtensionCount = 1;
	 	const char* swapchain_ext = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	 	info.ppEnabledExtensionNames = &swapchain_ext;
	 	info.pEnabledFeatures = NULL;
	 
		if(vkCreateDevice(vk.physical_device, &info, NULL, &vk.device) != VK_SUCCESS) {
			printf("Error! Could not create logical device.\n");
			exit(ERR_VULKAN);
		}
	}

	return vk;
}

void destroy_vulkan_state(VulkanState* vk) {
	vkDestroyDevice(vk->device, NULL);
	vkDestroyInstance(vk->instance, NULL);
}
