#include "window.h"
#include "app_name.h"

Window new_window() {
	SDL_Window* w = SDL_CreateWindow(
		APP_NAME,
		0, 
		0, 
		SURFACE_WIDTH, 
		SURFACE_HEIGHT,
		SDL_WINDOW_VULKAN
	);
	return (Window){w};
}
