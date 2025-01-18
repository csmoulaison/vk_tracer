#include "stdio.h"
#include "window.h"
#include "vulkan.h"

int main() {
	printf("Initializing...\n");

	SDL_Init(SDL_INIT_EVERYTHING);
	Window w = new_window();
	VulkanState vk = new_vulkan_state();

	printf("Initialized.\n");

	// TODO: Main loop

	printf("Destroying resources...\n");

	destroy_vulkan_state(&vk);

	printf("Destroyed.\n");
	
	return 0;
}
