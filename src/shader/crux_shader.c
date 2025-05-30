#include "shader/crux_shader.h"

#include <stdio.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

int crux_shader_compile(GLenum type, const char* source, GLuint* out)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		GLint logLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
		char* log = (char*)malloc((size_t)logLength);
		glGetShaderInfoLog(shader, logLength, NULL, log);
		printf("%s Compilation Failed:\n%s\n", glGetString(type), log);
		free(log);
		return -1;
	}

	*out = shader;
	return 0;
}

int crux_shader_program(GLuint vertex_shader, GLuint fragment_shader, GLuint* out)
{
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	GLint success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (!success)
	{
		GLint logLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		char* log = (char*)malloc((size_t)logLength);
		glGetProgramInfoLog(program, logLength, NULL, log);
		printf("Shader Program Linking Failed:\n%s\n", log);
		free(log);
		return -1;
	}

	*out = program;
	return 0;
}

void crux_shader_build_path(char* out_path, size_t max_len, const char* shader_filename)
{
	char  exe_path[MAX_PATH];
	DWORD len = GetModuleFileNameA(NULL, exe_path, MAX_PATH);

	if (len == 0 || len == MAX_PATH) return;

	for (int i = (int)len - 1; i >= 0; --i)
	{
		if (exe_path[i] == '\\' || exe_path[i] == '/')
		{
			exe_path[i + 1] = '\0';
			break;
		}
	}

	snprintf(out_path, max_len, "%s..\\..\\..\\shaders\\%s", exe_path, shader_filename);
}