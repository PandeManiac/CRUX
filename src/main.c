#include "window/crux_window.h"
#include "sim/sim_pixel.h"
#include "sim/sim_material.h"
#include "sdl/crux_sdl_wrap.h"
#include "glad/crux_glad_wrap.h"
#include "file/crux_file.h"
#include "shader/crux_shader.h"
#include "vmap/vmap.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

static double get_time_seconds(void)
{
	return (double)SDL_GetTicks64() / 1000.0;
}

static void simulate(pixel* read, pixel* write, uint16_t w, uint16_t h)
{
	// Copy pixels to the write buffer before simulation
	for (size_t i = 0; i < w * h; ++i)
	{
		write[i] = read[i];
	}

	for (int y = h - 2; y >= 0; --y) // Loop from bottom to top (y-axis)
	{
		for (int x = 0; x < w; ++x) // Loop over each pixel in the row (x-axis)
		{
			size_t	i	= (size_t)(y * w + x);	  // Index for current pixel
			uint8_t mat = read[i].material_index; // Material type at this pixel

			switch (mat)
			{
				case MATERIAL_WATER:
				case MATERIAL_LAVA:
				{
					size_t below = (size_t)((y + 1) * w + x); // Index of the cell below

					// Check if the space below is air
					if (read[below].material_index == MATERIAL_AIR)
					{
						write[below].material_index = mat;
						write[i].material_index		= MATERIAL_AIR;
					}
					else if (x > 0 && read[below - 1].material_index == MATERIAL_AIR)
					{
						write[below - 1].material_index = mat;
						write[i].material_index			= MATERIAL_AIR;
					}
					else if (x < w - 1 && read[below + 1].material_index == MATERIAL_AIR)
					{
						write[below + 1].material_index = mat;
						write[i].material_index			= MATERIAL_AIR;
					}

					break;
				}

				case MATERIAL_FIRE:
				{
					// Spread fire to nearby wood
					for (int dy = -1; dy <= 1; ++dy)
					{
						for (int dx = -1; dx <= 1; ++dx)
						{
							int nx = x + dx, ny = y + dy;
							if (nx < 0 || ny < 0 || nx >= w || ny >= h) continue; // Ensure inside bounds
							size_t ni = (size_t)(ny * w + nx);					  // Calculate index for neighbor
							if (read[ni].material_index == MATERIAL_WOOD) write[ni].material_index = MATERIAL_FIRE;
						}
					}

					// Random chance to decay fire to air
					if (rand() % 5 == 0) write[i].material_index = MATERIAL_AIR;
					break;
				}

				case MATERIAL_WOOD:
				case MATERIAL_STONE:
					break;

				default:
					break;
			}
		}
	}
}

static void draw_material(pixel* pixels, uint16_t sim_width, uint16_t sim_height, int sim_x, int sim_y, uint8_t material, int radius)
{
	// Draw material in a circular region (radius around (sim_x, sim_y))
	for (int dy = -radius; dy <= radius; ++dy)
	{
		for (int dx = -radius; dx <= radius; ++dx)
		{
			int x = sim_x + dx;
			int y = sim_y + dy;
			if (x < 0 || y < 0 || x >= sim_width || y >= sim_height) continue; // Out-of-bounds check
			if (dx * dx + dy * dy > radius * radius) continue;				   // Check within circle radius

			pixels[y * sim_width + x].material_index = material;
		}
	}
}

static const char* get_material_name(uint8_t material)
{
	switch (material)
	{
		case MATERIAL_AIR:
			return "Air";
		case MATERIAL_WOOD:
			return "Wood";
		case MATERIAL_FIRE:
			return "Fire";
		case MATERIAL_WATER:
			return "Water";
		case MATERIAL_STONE:
			return "Stone";
		case MATERIAL_LAVA:
			return "Lava";
		default:
			return "Unknown";
	}
}

int main(void)
{
	const uint16_t sim_width	   = 320;
	const uint16_t sim_height	   = 180;
	const uint32_t pixel_count	   = sim_width * sim_height;
	const uint32_t pixels_mem_size = pixel_count * sizeof(pixel);
	const uint32_t texels_mem_size = pixel_count;

	const char* sim_title = "CRUX";

	crux_window window = { 0 };
	if (crux_sdl_init() != 0) return EXIT_FAILURE;
	if (crux_window_init(&window, sim_title) != 0) return EXIT_FAILURE;
	if (crux_glad_init() != 0) return EXIT_FAILURE;

	uint32_t material_palette[256] = { 0 };
	for (int i = 0; i < 256; ++i)
	{
		material_palette[i] = RGBA(0, 0, 0, 255);
	}

	material_palette[MATERIAL_AIR]	 = RGBA(10, 10, 10, 255);
	material_palette[MATERIAL_WOOD]	 = RGBA(139, 69, 19, 255);
	material_palette[MATERIAL_FIRE]	 = RGBA(255, 69, 0, 255);
	material_palette[MATERIAL_WATER] = RGBA(0, 191, 255, 255);
	material_palette[MATERIAL_STONE] = RGBA(169, 169, 169, 255);

	// Memory allocations using vmap
	pixel* pixels_front = NULL;
	if (vmap_reserve(pixels_mem_size, (void**)&pixels_front) != 0) return EXIT_FAILURE;
	if (vmap_commit(pixels_mem_size, pixels_front) != 0) return EXIT_FAILURE;
	if (vmap_prefault(pixels_mem_size, pixels_front) != 0) return EXIT_FAILURE;

	pixel* pixels_back = NULL;
	if (vmap_reserve(pixels_mem_size, (void**)&pixels_back) != 0) return EXIT_FAILURE;
	if (vmap_commit(pixels_mem_size, pixels_back) != 0) return EXIT_FAILURE;
	if (vmap_prefault(pixels_mem_size, pixels_back) != 0) return EXIT_FAILURE;

	// Initialize pixels with AIR material
	for (uint32_t i = 0; i < pixel_count; ++i)
	{
		pixels_front[i].material_index = MATERIAL_AIR;
		pixels_back[i].material_index  = MATERIAL_AIR;
	}

	uint8_t* texels = NULL;
	if (vmap_reserve(texels_mem_size, (void**)&texels) != 0) return EXIT_FAILURE;
	if (vmap_commit(texels_mem_size, texels) != 0) return EXIT_FAILURE;
	if (vmap_prefault(texels_mem_size, texels) != 0) return EXIT_FAILURE;

	// OpenGL setup for textures
	GLuint texture_simulation;
	glGenTextures(1, &texture_simulation);
	glBindTexture(GL_TEXTURE_2D, texture_simulation);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8UI, sim_width, sim_height);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	GLuint texture_palette;
	glGenTextures(1, &texture_palette);
	glBindTexture(GL_TEXTURE_1D, texture_palette);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, material_palette);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	GLuint texture_emission;
	glGenTextures(1, &texture_emission);
	glBindTexture(GL_TEXTURE_1D, texture_emission);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, material_palette);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	size_t vs_size = 0;
	size_t fs_size = 0;

	char* vs_path[MAX_PATH];
	char* fs_path[MAX_PATH];

	crux_shader_build_path(vs_path, sizeof(vs_path), "vertex.glsl");
	crux_shader_build_path(fs_path, sizeof(fs_path), "fragment.glsl");

	const char* vs_src = crux_file_load_text("vertex.glsl", &vs_size);
	const char* fs_src = crux_file_load_text("fragment.glsl", &fs_size);

	GLuint vs;
	GLuint fs;

	if (crux_shader_compile(GL_VERTEX_SHADER, vs_src, &vs) != 0) return EXIT_FAILURE;
	if (crux_shader_compile(GL_FRAGMENT_SHADER, fs_src, &fs) != 0) return EXIT_FAILURE;

	GLuint shader_program;
	if (crux_shader_program(vs, fs, &shader_program) != 0) return EXIT_FAILURE;
	glUseProgram(shader_program);
	glDeleteShader(vs);
	glDeleteShader(fs);
	glUniform1i(glGetUniformLocation(shader_program, "sim_grid"), 0);
	glUniform1i(glGetUniformLocation(shader_program, "palette"), 1);
	glUniform1i(glGetUniformLocation(shader_program, "emission_palette"), 2);

	// Vertex buffer setup
	const float vertices[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	};

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	uint8_t current_material = MATERIAL_WOOD;

	double sim_time = 0.0, sim_dt = 1.0 / 60.0;
	double accumulator = 0.0, current_time = get_time_seconds();
	int	   frame_counter = 0, sim_counter = 0;
	double stat_timer = 0.0;

	SDL_Event event;
	int		  running = 1;

	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				running = 0;
			}

			if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
					case SDLK_1:
						current_material = MATERIAL_AIR;
						break;
					case SDLK_2:
						current_material = MATERIAL_WOOD;
						break;
					case SDLK_3:
						current_material = MATERIAL_FIRE;
						break;
					case SDLK_4:
						current_material = MATERIAL_WATER;
						break;
					case SDLK_5:
						current_material = MATERIAL_STONE;
						break;
				}

				printf("[INPUT] Current material: %s\n", get_material_name(current_material));
			}
		}

		int		 mouse_x, mouse_y;
		uint32_t mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

		if (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT))
		{
			float scale_x = (float)mouse_x / (float)window.width;
			float scale_y = (float)mouse_y / (float)window.height;

			int sim_x = (int)(scale_x * sim_width);
			int sim_y = (int)(scale_y * sim_height);

			draw_material(pixels_front, sim_width, sim_height, sim_x, sim_y, current_material, 3);
		}

		double new_time	  = get_time_seconds();
		double frame_time = new_time - current_time;
		if (frame_time > 0.25) frame_time = 0.25;
		current_time = new_time;

		accumulator += frame_time;
		stat_timer += frame_time;
		frame_counter++;

		while (accumulator >= sim_dt)
		{
			simulate(pixels_front, pixels_back, sim_width, sim_height);

			pixel* temp	 = pixels_front;
			pixels_front = pixels_back;
			pixels_back	 = temp;

			accumulator -= sim_dt;
			sim_counter++;
		}

		if (stat_timer >= 1.0)
		{
			printf("[STATS] FPS: %d | Sim Steps/sec: %d\n", frame_counter, sim_counter);
			stat_timer	  = 0.0;
			frame_counter = 0;
			sim_counter	  = 0;
		}

		for (size_t i = 0; i < pixel_count; ++i)
		{
			texels[i] = pixels_front[i].material_index;
		}

		glBindTexture(GL_TEXTURE_2D, texture_simulation);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, sim_width, sim_height, GL_RED_INTEGER, GL_UNSIGNED_BYTE, texels);
		glViewport(0, 0, window.width, window.height);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_simulation);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_1D, texture_palette);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_1D, texture_emission);

		glUseProgram(shader_program);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		SDL_GL_SwapWindow(window.window);
	}

	return 0;
}