#ifndef PIXEL_H
#define PIXEL_H

#include <stdint.h>

typedef struct
{
	uint8_t material_index;
	uint8_t data;
} sim_pixel;

void sim_pixel_buffers_allocate(sim_pixel** front, sim_pixel** back, uint8_t** texels, uint64_t sim_pixels_mem_size);

#endif // PIXEL_H