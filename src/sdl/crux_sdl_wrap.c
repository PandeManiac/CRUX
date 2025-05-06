#include "sdl/crux_sdl_wrap.h"

int crux_sdl_init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	return 0;
}