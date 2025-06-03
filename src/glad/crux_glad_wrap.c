#include "glad/crux_glad_wrap.h"
#include "sdl/crux_sdl_wrap.h"
#include "assert/crux_assert.h"

void crux_glad_init(void)
{
	ASSERT_FATAL(gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress), "Failed to initialize GLAD!");
}