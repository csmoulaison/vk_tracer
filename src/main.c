#include "SDL2/SDL.h"
#include "vulkan/vulkan.h"

int main() {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window = SDL_CreateWindow("vk_tracer", 0, 0, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

	uint32_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
	printf("Extension count: %i\n", extension_count);

	return 0;
}
