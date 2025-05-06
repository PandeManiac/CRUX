#ifndef PIXEL_H
#define PIXEL_H

#include <stdint.h>

typedef struct pixel
{
	uint8_t material_index;
	uint8_t emission_index;
	uint8_t emission_intensity;
	uint8_t temperature;
	uint8_t padding[4];
} pixel;

#endif // PIXEL_H