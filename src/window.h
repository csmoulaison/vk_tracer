#ifndef window_h_INCLUDED
#define window_h_INCLUDED

#include "SDL2/SDL.h"

#define SURFACE_WIDTH 640
#define SURFACE_HEIGHT 640

struct {
	SDL_Window* sdl;
} typedef Window;

Window new_window();

#endif // window_h_INCLUDED
