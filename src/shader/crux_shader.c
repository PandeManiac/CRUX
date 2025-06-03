#include "shader/crux_shader.h"
#include "file/crux_file.h"
#include "assert/crux_assert.h"

#include <stdio.h>
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

static const char* crux_shader_type_string(GLenum type)
{
	switch (type)
	{
		case GL_VERTEX_SHADER:
			return "Vertex Shader";
		case GL_FRAGMENT_SHADER:
			return "Fragment Shader";
		case GL_GEOMETRY_SHADER:
			return "Geometry Shader";
		case GL_COMPUTE_SHADER:
			return "Compute Shader";
		case GL_TESS_CONTROL_SHADER:
			return "Tessellation Control Shader";
		case GL_TESS_EVALUATION_SHADER:
			return "Tessellation Evaluation Shader";
		default:
			return "Unknown Shader Type";
	}
}

static void crux_shader_print_log(GLenum type, GLuint shader)
{
	GLint log_size = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
	if (log_size <= 1) return;

	char* log = (char*)malloc((size_t)log_size);
	if (!log) return;

	glGetShaderInfoLog(shader, log_size, NULL, log);
	fprintf(stderr, "%s Compilation Failed:\n%s\n", crux_shader_type_string(type), log);
	free(log);
}

void crux_shader_compile(GLenum type, const char* source, GLuint* out)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		crux_shader_print_log(type, shader);
		glDeleteShader(shader);
		ASSERT_FATAL(0, "Shader compilation failed!");
	}

	*out = shader;
}

void crux_shader_program(GLuint vertex_shader, GLuint fragment_shader, GLuint* out)
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
		if (log)
		{
			glGetProgramInfoLog(program, logLength, NULL, log);
			fprintf(stderr, "Shader Program Linking Failed:\n%s\n", log);
			free(log);
		}
		glDeleteProgram(program);
		ASSERT_FATAL(0, "Shader program linking failed!");
	}

	*out = program;
}

void crux_shader_build_path(char* out_path, size_t max_len, const char* shader_filename)
{
	char  exe_path[MAX_PATH];
	DWORD len = GetModuleFileNameA(NULL, exe_path, MAX_PATH);
	ASSERT_FATAL(len > 0 && len < MAX_PATH, "Failed to get module filename.");

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