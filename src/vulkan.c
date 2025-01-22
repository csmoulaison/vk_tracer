#include "vulkan.h"

#include "SDL2/SDL_vulkan.h"
#include "string.h"
#include "stdbool.h"
#include "stdlib.h"

#include "exit_codes.h"
#include "app_name.h"
#include "build_flags.h"

VulkanState new_vulkan_state(Window* window) {
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

		#ifdef DEBUG
			info.enabledLayerCount = 1;
			info.ppEnabledLayerNames = &(const char*){"VK_LAYER_KHRONOS_validation"};
		#else
			info.enabledLayerCount = 0;
			info.ppEnabledLayerNames = NULL;
		#endif
		
		uint32_t exts_len = 0;
		SDL_Vulkan_GetInstanceExtensions(window->sdl, &exts_len, NULL);
		const char** window_exts = malloc(sizeof(char*) * exts_len);
		SDL_Vulkan_GetInstanceExtensions(window->sdl, &exts_len, window_exts);

		#ifdef DEBUG
			exts_len++;
		#endif

		const char* exts[exts_len];
		memcpy(exts, window_exts, sizeof(char*) * (exts_len - 1));

		#ifdef DEBUG
			exts[exts_len - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		#endif

		info.enabledExtensionCount = exts_len;
		info.ppEnabledExtensionNames = exts;

		VkResult res = vkCreateInstance(&info, NULL, &vk.instance);
		if(res != VK_SUCCESS) {
			printf("Error %i: Could not create instance.\n", res);
			exit(ERR_VULKAN);
		}
	}

	// Create surface
	{
		bool res = SDL_Vulkan_CreateSurface(window->sdl, vk.instance, &vk.surface);
		if(!res) {
			printf("Could not create surface.\n");
			exit(ERR_BAD_SURFACE);
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
				if(strcmp(exts[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
					swapchain = true;
				}
			}
			if(!(swapchain)) {
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

	 	const char* swapchain_ext = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
	 	info.enabledExtensionCount = 1;
	 	info.ppEnabledExtensionNames = &swapchain_ext;

	 	info.pEnabledFeatures = NULL;

	 	VkResult res = vkCreateDevice(vk.physical_device, &info, NULL, &vk.device);
		if(res != VK_SUCCESS) {
			printf("Error %i: Could not create logical device.\n", res);
			exit(ERR_VULKAN);
		}
	}

	// Create swapchain, images, and image views
	{
		// Query surface capabilities
		uint32_t image_count = 0;
		VkSurfaceTransformFlagBitsKHR pre_transform;
		{
			VkSurfaceCapabilitiesKHR abilities;
			VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk.physical_device, vk.surface, &abilities);
			if(res != VK_SUCCESS) {
				printf("Error %i: Could not get physical device surface capabilities.\n", res);
				exit(ERR_VULKAN);
			}

			if(abilities.minImageExtent.width  > SURFACE_WIDTH
			|| abilities.maxImageExtent.width  < SURFACE_WIDTH
			|| abilities.minImageExtent.height > SURFACE_HEIGHT
			|| abilities.maxImageExtent.height < SURFACE_HEIGHT) {
				printf("Surface KHR extents are not compatible with configured surface sizes.\n", res);
				exit(ERR_BAD_SURFACE);
			}

			image_count = abilities.minImageCount + 1;
			if(abilities.maxImageCount > 0 && image_count > abilities.maxImageCount) {
				image_count = abilities.maxImageCount;
			}

			pre_transform = abilities.currentTransform;
		}

		// Choose surface format
		VkSurfaceFormatKHR surface_format;
		{
			uint32_t formats_len;
			vkGetPhysicalDeviceSurfaceFormatsKHR(vk.physical_device, vk.surface, &formats_len, NULL);
			if(formats_len == 0) {
				printf("Physical device doesn't support any formats?\n");
				exit(ERR_BAD_SURFACE);
			}

			VkSurfaceFormatKHR formats[formats_len];
			vkGetPhysicalDeviceSurfaceFormatsKHR(vk.physical_device, vk.surface, &formats_len, formats);

			surface_format = formats[0];
			for(int i = 0; i < formats_len; i++) {
				if(formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					surface_format = formats[i];
					break;
				}
			}
		}

		// Choose presentation mode
		// 
		// Default to VK_PRESENT_MODE_FIFO_KHR, as this is the only mode required to
		// be supported by the spec.
		VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
		{
			uint32_t modes_len;
			VkResult res = vkGetPhysicalDeviceSurfacePresentModesKHR(vk.physical_device, vk.surface, &modes_len, NULL);
			if(res != VK_SUCCESS) {
				printf("Error %i: Could not get number of physical device surface present modes.\n", res);
				exit(ERR_VULKAN);
			}

			VkPresentModeKHR modes[modes_len];
			res = vkGetPhysicalDeviceSurfacePresentModesKHR(vk.physical_device, vk.surface, &modes_len, modes);
			if(res != VK_SUCCESS) {
				printf("Error %i: Could not get physical device surface present modes.\n", res);
				exit(ERR_VULKAN);
			}

			for(int i = 0; i < modes_len; i++) {
				if(modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
					present_mode = modes[i];
					break;
				}
			}
		}

		// Create swapchain
		VkSwapchainCreateInfoKHR info;
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.pNext = NULL;
		info.flags = 0; // TODO mutable format or any other flags?
		info.surface = vk.surface;
		info.minImageCount = image_count; // TODO get this value.
		info.imageFormat = surface_format.format;
		info.imageColorSpace = surface_format.colorSpace;
		info.imageExtent = (VkExtent2D){SURFACE_WIDTH, SURFACE_HEIGHT};
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // TODO probably right, but we'll see.
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // TODO needs to be different if compute is in different family from present?
		info.queueFamilyIndexCount = 0; // Not used in exclusive mode. Need to check for concurrent.
		info.pQueueFamilyIndices = NULL; // Also not used in exclusive mode, see above.
		info.preTransform = pre_transform;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.presentMode = present_mode;
		info.clipped = VK_TRUE;
		info.oldSwapchain = VK_NULL_HANDLE;

		VkResult res = vkCreateSwapchainKHR(vk.device, &info, NULL, &vk.swapchain);
		if(res != VK_SUCCESS) {
			printf("Error %i: Could not create swapchain.\n", res);
			exit(ERR_VULKAN);
		}

		// Create swapchain images and image views
		{
			// Images
			VkResult res = vkGetSwapchainImagesKHR(vk.device, vk.swapchain, &vk.images_len, NULL);
			if(res != VK_SUCCESS) {
				printf("Error %i: Could not get number of swapchain images.\n", res);
				exit(ERR_VULKAN);
			}

			res = vkGetSwapchainImagesKHR(vk.device, vk.swapchain, &vk.images_len, vk.swap_images);
			if(res != VK_SUCCESS) {
				printf("Error %i: Could not get swapchain images.\n", res);
				exit(ERR_VULKAN);
			}

			// Image views
			for(int i = 0; i < vk.images_len; i++) {
				VkImageViewCreateInfo info;
				info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				info.pNext = NULL;
				info.flags = 0;
				info.image = vk.swap_images[i];
				info.viewType = VK_IMAGE_VIEW_TYPE_2D;
				info.format = surface_format.format;
				info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				info.subresourceRange.baseMipLevel = 0;
				info.subresourceRange.levelCount = 1;
				info.subresourceRange.baseArrayLayer = 0;
				info.subresourceRange.layerCount = 1;
				
				res = vkCreateImageView(vk.device, &info, NULL, &vk.swap_views[i]);
				if(res != VK_SUCCESS) {
					printf("Error %i: Could not create image views.\n", res);
					exit(ERR_VULKAN);
				}
			}
		}
	}

	// Create render pass
	{
		VkRenderPassCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.pNext = NULL;
		info.flags = 0;
		info.attachmentCount = //
		info.pAttachments = //
		info.subpassCount = //
		info.pSubpasses = //
		info.dependencyCount = //
		info.pDependencies = //
		
		VkResult res = vkCreateRenderPass(vk.device, &info, NULL, &vk.renderpass);
	}

	return vk;
}

void update_vulkan_state(VulkanState* vk) {

}

void destroy_vulkan_state(VulkanState* vk) {
	for(int i = 0; i < vk->images_len; i++) {
		vkDestroyImageView(vk->device, vk->swap_views[i], NULL);
	}
	vkDestroySwapchainKHR(vk->device, vk->swapchain, NULL);
	vkDestroySurfaceKHR(vk->instance, vk->surface, NULL);
	vkDestroyDevice(vk->device, NULL);
	vkDestroyInstance(vk->instance, NULL);
}
