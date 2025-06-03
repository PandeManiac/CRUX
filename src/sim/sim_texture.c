#include "sim/sim_texture.h"

GLuint sim_texture_view_initialize(uint16_t width, uint16_t height)
{
	GLuint texture_view;
	glGenTextures(1, &texture_view);
	glBindTexture(GL_TEXTURE_2D, texture_view);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8UI, width, height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	return texture_view;
}

GLuint sim_texture_palette_initialize(const uint32_t* material_palette)
{
	GLuint texture_palette;
	glGenTextures(1, &texture_palette);
	glBindTexture(GL_TEXTURE_1D, texture_palette);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, material_palette);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	return texture_palette;
}