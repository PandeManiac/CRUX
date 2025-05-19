#ifndef PIXEL_H
#define PIXEL_H

#include <stdint.h>

typedef struct pixel
{
	uint8_t material_index;
	uint8_t data;
} pixel;

#endif // PIXEL_H