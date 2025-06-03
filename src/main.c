#include "window/crux_window.h"
#include "sim/sim_pixel.h"
#include "sim/sim_material.h"
#include "sim/sim_texture.h"
#include "sdl/crux_sdl_wrap.h"
#include "glad/crux_glad_wrap.h"
#include "file/crux_file.h"
#include "shader/crux_shader.h"
#include "vmap/vmap.h"
#include "threads/sim_thread.h"
#include "assert/crux_assert.h"

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

static inline void draw_material_world(sim_pixel* pixels, uint16_t sim_width, uint16_t sim_height, int32_t sim_origin_x, int32_t sim_origin_y, int32_t world_x, int32_t world_y, uint8_t material, int radius)
{
	for (int dy = -radius; dy <= radius; ++dy)
	{
		for (int dx = -radius; dx <= radius; ++dx)
		{
			if (dx * dx + dy * dy > radius * radius) continue;

			int32_t wx = world_x + dx;
			int32_t wy = world_y + dy;

			if (wx < sim_origin_x || wy < sim_origin_y || wx >= sim_origin_x + sim_width || wy >= sim_origin_y + sim_height) continue;

			int32_t lx = wx - sim_origin_x;
			int32_t ly = wy - sim_origin_y;

			pixels[ly * sim_width + lx].material_index = material;
		}
	}
}

int main(void)
{
	const uint16_t view_width	   = 320;
	const uint16_t view_height	   = 180;
	const uint16_t sim_view_mult   = 20;
	const uint16_t sim_width	   = view_width * sim_view_mult;
	const uint16_t sim_height	   = view_height * sim_view_mult;
	const uint32_t sim_pixel_count = sim_width * sim_height;
	const uint32_t texels_mem_size = view_width * view_height;

	double camera_xf = sim_width / 2.0;
	double camera_yf = sim_height / 2.0;

	crux_window window = { 0 };

	crux_sdl_init();
	crux_window_init(&window, "CRUX");
	crux_glad_init();

	uint32_t material_palette[256] = { 0 };
	material_palette_initialize(material_palette);

	sim_pixel* pixels_front = NULL;
	sim_pixel* pixels_back	= NULL;
	uint8_t*   texels		= NULL;
	sim_pixel_buffers_allocate(&pixels_front, &pixels_back, &texels, sim_pixel_count);

	GLuint texture_view	   = sim_texture_view_initialize(view_width, view_height);
	GLuint texture_palette = sim_texture_palette_initialize(material_palette);

	GLuint vs			  = 0;
	GLuint fs			  = 0;
	GLuint shader_program = 0;

	const char* vs_src = crux_file_load_text("vertex.glsl", NULL);
	const char* fs_src = crux_file_load_text("fragment.glsl", NULL);

	crux_shader_compile(GL_VERTEX_SHADER, vs_src, &vs);
	crux_shader_compile(GL_FRAGMENT_SHADER, fs_src, &fs);
	crux_shader_program(vs, fs, &shader_program);

	glUseProgram(shader_program);
	glUniform1i(glGetUniformLocation(shader_program, "view_grid"), 0);
	glUniform1i(glGetUniformLocation(shader_program, "palette"), 1);

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

	GLuint pbos[2];
	glGenBuffers(2, pbos);
	for (int i = 0; i < 2; ++i)
	{
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[i]);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, texels_mem_size, NULL, GL_STREAM_DRAW);
	}
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	uint8_t	  current_material = MATERIAL_WOOD;
	SDL_Event event;
	int		  running = 1, pbo_index = 0;
	double	  accumulator = 0.0, current_time = get_time_seconds();
	double	  sim_dt = 1.0 / 60.0, stat_timer = 0.0;
	int		  frame_counter = 0, sim_counter = 0;

	int checker_phase = 0;

	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT) running = 0;
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
			}
		}

		double new_time	  = get_time_seconds();
		double frame_time = new_time - current_time;
		if (frame_time > 0.25) frame_time = 0.25;
		current_time = new_time;

		accumulator += frame_time;
		stat_timer += frame_time;
		frame_counter++;

		const uint8_t* keys			= SDL_GetKeyboardState(NULL);
		double		   camera_speed = 300.0;
		if (keys[SDL_SCANCODE_W]) camera_yf -= camera_speed * frame_time;
		if (keys[SDL_SCANCODE_S]) camera_yf += camera_speed * frame_time;
		if (keys[SDL_SCANCODE_A]) camera_xf -= camera_speed * frame_time;
		if (keys[SDL_SCANCODE_D]) camera_xf += camera_speed * frame_time;

		if (camera_xf < view_width / 2.0) camera_xf = view_width / 2.0;
		if (camera_yf < view_height / 2.0) camera_yf = view_height / 2.0;
		if (camera_xf > sim_width - view_width / 2.0) camera_xf = sim_width - view_width / 2.0;
		if (camera_yf > sim_height - view_height / 2.0) camera_yf = sim_height - view_height / 2.0;

		int camera_x = (int)camera_xf;
		int camera_y = (int)camera_yf;

		int32_t sim_origin_x = 0;
		int32_t sim_origin_y = 0;

		int		 mouse_x, mouse_y;
		uint32_t mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
		if (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT))
		{
			float scale_x = (float)mouse_x / (float)window.width;
			float scale_y = (float)mouse_y / (float)window.height;
			int	  world_x = camera_x - view_width / 2 + (int)(scale_x * view_width);
			int	  world_y = camera_y - view_height / 2 + (int)(scale_y * view_height);
			draw_material_world(pixels_front, sim_width, sim_height, sim_origin_x, sim_origin_y, world_x, world_y, current_material, 3);
		}

		while (accumulator >= sim_dt)
		{
			simulate_checkerboard(pixels_front, pixels_back, sim_width, sim_height, checker_phase);
			sim_pixel* temp = pixels_front;
			pixels_front	= pixels_back;
			pixels_back		= temp;
			accumulator -= sim_dt;
			sim_counter++;
			checker_phase ^= 1;
		}

		if (stat_timer >= 1.0)
		{
			printf("[STATS] FPS: %d | Sim Steps/sec: %d\n", frame_counter, sim_counter);
			stat_timer	  = 0.0;
			frame_counter = 0;
			sim_counter	  = 0;
		}

		for (uint16_t y = 0; y < view_height; ++y)
		{
			for (uint16_t x = 0; x < view_width; ++x)
			{
				int32_t	 world_x   = camera_x - view_width / 2 + x;
				int32_t	 world_y   = camera_y - view_height / 2 + y;
				uint32_t tex_index = y * view_width + x;

				if (world_x < 0 || world_y < 0 || world_x >= sim_width || world_y >= sim_height)
					texels[tex_index] = MATERIAL_AIR;
				else
					texels[tex_index] = pixels_front[world_y * sim_width + world_x].material_index;
			}
		}

		pbo_index = (pbo_index + 1) % 2;
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbos[pbo_index]);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, texels_mem_size, NULL, GL_STREAM_DRAW);
		void* ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		if (ptr)
		{
			memcpy(ptr, texels, texels_mem_size);
			glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		}
		glBindTexture(GL_TEXTURE_2D, texture_view);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, view_width, view_height, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 0);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		glViewport(0, 0, window.width, window.height);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shader_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_view);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_1D, texture_palette);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		SDL_GL_SwapWindow(window.window);
	}

	glDeleteBuffers(2, pbos);
	return 0;
}