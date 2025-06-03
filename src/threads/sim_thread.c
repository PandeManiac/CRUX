#include "threads/sim_thread.h"
#include "threads/tinycthread.h"
#include "sim/sim_material.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define TILE_SIZE	  64
#define MAX_JOBS	  32000
#define MAX_THREADS	  32
#define FIRE_LIFESPAN 10

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

typedef struct
{
	sim_pixel* read;
	sim_pixel* write;
	uint16_t   sim_w;
	uint16_t   sim_h;
	uint16_t   tile_x;
	uint16_t   tile_y;
	uint16_t   tile_w;
	uint16_t   tile_h;
	uint32_t   rng_seed;
} sim_tile_job;

typedef struct
{
	sim_tile_job* jobs;
	int			  job_start;
	int			  job_end;
} sim_thread_context;

static int get_hardware_thread_count(void)
{
#ifdef _WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return (int)sysinfo.dwNumberOfProcessors;
#else
	long n = sysconf(_SC_NPROCESSORS_ONLN);
	return (n > 0) ? (int)n : 1;
#endif
}

static inline uint32_t xorshift32(uint32_t* state)
{
	uint32_t x = *state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	*state = x;
	return x;
}

static int simulate_tile_worker(void* arg)
{
	sim_thread_context* ctx = (sim_thread_context*)arg;

	for (int j = ctx->job_start; j < ctx->job_end; ++j)
	{
		sim_tile_job* job	= &ctx->jobs[j];
		sim_pixel*	  read	= job->read;
		sim_pixel*	  write = job->write;
		uint16_t	  sim_w = job->sim_w;
		uint16_t	  sim_h = job->sim_h;
		uint32_t	  rng	= job->rng_seed;

		uint16_t x0 = job->tile_x;
		uint16_t y0 = job->tile_y;
		uint16_t x1 = x0 + job->tile_w;
		uint16_t y1 = y0 + job->tile_h;

		for (int y = y1 - 1; y >= (int)y0; --y)
		{
			for (int x = x0; x < x1; ++x)
			{
				size_t	i	= (size_t)(y * sim_w + x);
				uint8_t mat = write[i].material_index;

				switch (mat)
				{
					case MATERIAL_WATER:
					case MATERIAL_LAVA:
					{
						if (y + 1 < sim_h)
						{
							size_t below = (size_t)((y + 1) * sim_w + x);

							if (read[below].material_index == MATERIAL_AIR)
							{
								write[below].material_index = mat;
								write[below].data			= 0;
								write[i].material_index		= MATERIAL_AIR;
								write[i].data				= 0;
							}
							else if (x > 0 && read[below - 1].material_index == MATERIAL_AIR)
							{
								write[below - 1].material_index = mat;
								write[below - 1].data			= 0;
								write[i].material_index			= MATERIAL_AIR;
								write[i].data					= 0;
							}
							else if (x + 1 < sim_w && read[below + 1].material_index == MATERIAL_AIR)
							{
								write[below + 1].material_index = mat;
								write[below + 1].data			= 0;
								write[i].material_index			= MATERIAL_AIR;
								write[i].data					= 0;
							}
						}
						break;
					}

					case MATERIAL_FIRE:
					{
						for (int dy = -1; dy <= 1; ++dy)
						{
							for (int dx = -1; dx <= 1; ++dx)
							{
								int nx = x + dx;
								int ny = y + dy;
								if (nx < 0 || ny < 0 || nx >= sim_w || ny >= sim_h) continue;
								size_t ni = (size_t)(ny * sim_w + nx);

								if (read[ni].material_index == MATERIAL_WOOD && (xorshift32(&rng) % 2 == 0))
								{
									write[ni].material_index = MATERIAL_IGNITE;
									write[ni].data			 = 0;
								}
							}
						}

						write[i].data++;

						if (write[i].data >= 3)
						{
							uint32_t age		  = write[i].data;
							uint32_t decay_chance = (age > 6) ? 1 : (7 - age);

							if (decay_chance <= 1 || (xorshift32(&rng) % decay_chance == 0))
							{
								write[i].material_index = MATERIAL_AIR;
								write[i].data			= 0;
							}
						}
						break;
					}

					default:
						break;
				}
			}
		}
	}

	return 0;
}

void simulate_checkerboard(sim_pixel* read, sim_pixel* write, uint16_t sim_w, uint16_t sim_h, int checker_phase)
{
	static sim_tile_job jobs[MAX_JOBS];
	int					job_count = 0;

	// Copy and promote ignite
	for (uint32_t i = 0; i < (uint32_t)(sim_w * sim_h); ++i)
	{
		uint8_t mat	 = read[i].material_index;
		uint8_t data = read[i].data;

		if (mat == MATERIAL_IGNITE)
		{
			mat	 = MATERIAL_FIRE;
			data = 0;
		}

		write[i].material_index = mat;
		write[i].data			= data;
	}

	// Create jobs
	for (uint16_t tile_y = 0; tile_y < sim_h; tile_y += TILE_SIZE)
	{
		for (uint16_t tile_x = 0; tile_x < sim_w; tile_x += TILE_SIZE)
		{
			uint16_t tile_ix = tile_x / TILE_SIZE;
			uint16_t tile_iy = tile_y / TILE_SIZE;

			if ((tile_ix + tile_iy) % 2 != checker_phase) continue;

			uint16_t tw = (tile_x + TILE_SIZE > sim_w) ? (sim_w - tile_x) : TILE_SIZE;
			uint16_t th = (tile_y + TILE_SIZE > sim_h) ? (sim_h - tile_y) : TILE_SIZE;

			if (tw == 0 || th == 0) continue;

			if (job_count >= MAX_JOBS)
			{
				fprintf(stderr, "ERROR: MAX_JOBS exceeded (%d), increase limit.\n", job_count);
				abort();
			}

			int job_index	= job_count++;
			jobs[job_index] = (sim_tile_job) { .read = read, .write = write, .sim_w = sim_w, .sim_h = sim_h, .tile_x = tile_x, .tile_y = tile_y, .tile_w = tw, .tile_h = th, .rng_seed = 0xA53F9E37u + ((uint32_t)job_index * 0x12345u) };
		}
	}

	if (job_count == 0) return;

	// 🔧 Detect thread count from system
	int num_threads = get_hardware_thread_count();
	if (num_threads > 32) num_threads = 32;
	if (job_count < num_threads) num_threads = job_count;

	thrd_t			   threads[MAX_THREADS];
	sim_thread_context thread_ctx[MAX_THREADS];
	int				   jobs_per_thread = job_count / num_threads;

	for (int i = 0; i < num_threads; ++i)
	{
		int start = i * jobs_per_thread;
		int end	  = (i == num_threads - 1) ? job_count : start + jobs_per_thread;

		thread_ctx[i].jobs		= jobs;
		thread_ctx[i].job_start = start;
		thread_ctx[i].job_end	= end;

		thrd_create(&threads[i], simulate_tile_worker, &thread_ctx[i]);
	}

	for (int i = 0; i < num_threads; ++i)
		thrd_join(threads[i], NULL);
}