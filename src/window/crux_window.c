#include "window/crux_window.h"
#include "assert/crux_assert.h"

void crux_window_init(crux_window* out, const char* title)
{
	ASSERT_FATAL(out, "crux_window* out was NULL!");

	out->window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
	ASSERT_FATAL(out->window, "crux_window SDL_Window creation failed!");

	out->gl = SDL_GL_CreateContext(out->window);
	ASSERT_FATAL(out->gl, "crux_window SDL_GLContext creation failed!");

	SDL_GetWindowSize(out->window, (int*)&out->width, (int*)&out->height);
	SDL_GL_SetSwapInterval(0);
}

void crux_window_destroy(crux_window* win)
{
	if (win->gl) SDL_GL_DeleteContext(win->gl);
	if (win->window) SDL_DestroyWindow(win->window);
	SDL_Quit();
}