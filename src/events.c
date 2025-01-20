#include "events.h"

bool should_quit() {
	SDL_Event e;
	while(SDL_PollEvent(&e)) {
		if(e.type == SDL_QUIT) {
			return true;
		}

		if(e.type == SDL_KEYDOWN && e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
			return true;
		}
	}
	
	return false;
}
