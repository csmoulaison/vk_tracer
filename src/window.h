#ifndef window_h_INCLUDED
#define window_h_INCLUDED

#include "SDL2/SDL.h"

struct {
	SDL_Window* sdl;
} typedef Window;

Window new_window();

#endif // window_h_INCLUDED
