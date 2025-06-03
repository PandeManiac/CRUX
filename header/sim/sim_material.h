#ifndef SIM_MATERIAL_H
#define SIM_MATERIAL_H

#include <stdint.h>

#define RGBA(r, g, b, a) ((uint32_t)(((a) << 24) | ((b) << 16) | ((g) << 8) | (r)))
#define MATERIAL_IGNITE	 254

typedef uint8_t material_id;

enum material_id_enum
{
	MATERIAL_AIR = 0,
	MATERIAL_WOOD,
	MATERIAL_FIRE,
	MATERIAL_WATER,
	MATERIAL_SMOKE,
	MATERIAL_SAND,
	MATERIAL_LAVA,
	MATERIAL_ICE,
	MATERIAL_OIL,
	MATERIAL_STONE,

	MATERIAL_COUNT
};

void material_palette_initialize(uint32_t* out_palette);

#endif // SIM_MATERIAL_H
