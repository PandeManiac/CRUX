#include "sdl/crux_sdl_wrap.h"
#include "assert/crux_assert.h"

void crux_sdl_init(void)
{
	ASSERT_FATAL(SDL_Init(SDL_INIT_VIDEO) >= 0, "Failed to Initialize SDL!");

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
}