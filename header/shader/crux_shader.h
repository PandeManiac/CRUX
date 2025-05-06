#ifndef CRUX_SHADER_H
#define CRUX_SHADER_H

#include "glad/crux_glad_wrap.h"

int crux_shader_compile(GLenum type, const char* source, GLuint* out);
int crux_shader_program(GLuint vertex_shader, GLuint fragment_shader, GLuint* out);

#endif // CRUX_SHADER_H