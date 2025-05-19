#include "window/crux_window.h"

int crux_window_init(crux_window* out, const char* title)
{
	if (!out) return -1;

	out->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (!out->window) return -2;

	out->gl = SDL_GL_CreateContext(out->window);
	if (!out->gl) return -3;

	SDL_GetWindowSize(out->window, (int*)&out->width, (int*)&out->height);

	SDL_GL_SetSwapInterval(1);
	return 0;
}

void crux_window_destroy(crux_window* win)
{
	if (win->gl) SDL_GL_DeleteContext(win->gl);
	if (win->window) SDL_DestroyWindow(win->window);
	SDL_Quit();
}