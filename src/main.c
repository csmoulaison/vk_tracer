#include "stdio.h"
#include "window.h"
#include "vulkan.h"
#include "events.h"

int main() {
	printf("Initializing...\n");
	SDL_Init(SDL_INIT_EVERYTHING);
	Window w = new_window();
	VulkanState vk = new_vulkan_state(&w);
	printf("Initialized.\n");

	while(!should_quit()) {
		// TODO main loop
	}

	printf("Destroying resources...\n");
	destroy_vulkan_state(&vk);
	printf("Destroyed.\n");
	
	return 0;
}
