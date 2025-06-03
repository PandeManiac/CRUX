#ifndef CRUX_WINDOW_H
#define CRUX_WINDOW_H

#include "sdl/crux_sdl_wrap.h"

typedef struct crux_window
{
	uint32_t	  width;
	uint32_t	  height;
	SDL_Window*	  window;
	SDL_GLContext gl;
} crux_window;

void crux_window_init(crux_window* out, const char* title);
void crux_window_destroy(crux_window* win);

#endif // CRUX_WINDOW_H