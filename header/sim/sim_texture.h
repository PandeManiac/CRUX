#ifndef SIM_TEXTURE_H
#define SIM_TEXTURE_H

#include "glad/crux_glad_wrap.h"

GLuint sim_texture_view_initialize(uint16_t width, uint16_t height);
GLuint sim_texture_palette_initialize(const uint32_t* material_palette);

#endif // SIM_TEXTURE_H