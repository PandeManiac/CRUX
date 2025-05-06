#include "glad/crux_glad_wrap.h"
#include "sdl/crux_sdl_wrap.h"

int crux_glad_init(void)
{
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) return -1;
	return 0;
}