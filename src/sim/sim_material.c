#include "sim/sim_material.h"

#include "assert/crux_assert.h"
#include <stdio.h>
#include <stdlib.h>

void material_palette_initialize(uint32_t* out_palette)
{
	ASSERT_FATAL(out_palette, "Failed to initialize material palette!");
	for (int i = 0; i < 256; ++i) out_palette[i] = RGBA(0, 0, 0, 255);

	out_palette[MATERIAL_AIR]	 = RGBA(10, 10, 10, 255);
	out_palette[MATERIAL_WOOD]	 = RGBA(139, 69, 19, 255);
	out_palette[MATERIAL_FIRE]	 = RGBA(255, 69, 0, 255);
	out_palette[MATERIAL_WATER]	 = RGBA(0, 191, 255, 255);
	out_palette[MATERIAL_STONE]	 = RGBA(169, 169, 169, 255);
	out_palette[MATERIAL_IGNITE] = RGBA(255, 128, 0, 255);
}