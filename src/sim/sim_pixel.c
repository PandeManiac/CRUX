#include "sim/sim_pixel.h"
#include "sim/sim_material.h"
#include "assert/crux_assert.h"
#include "vmap/vmap.h"

void sim_pixel_buffers_allocate(sim_pixel** front, sim_pixel** back, uint8_t** texels, uint64_t sim_pixel_count)
{
	uint64_t sim_pixels_mem_size = sim_pixel_count * sizeof(sim_pixel);

	int err_reserve_front  = vmap_reserve(sim_pixels_mem_size, (void**)front);
	int err_reserve_back   = vmap_reserve(sim_pixels_mem_size, (void**)back);
	int err_reserve_texels = vmap_reserve(sim_pixels_mem_size, (void**)texels);

	ASSERT_FATAL(err_reserve_front == 0, "Failed to reserve memory for front!");
	ASSERT_FATAL(err_reserve_back == 0, "Failed to reserve memory for back!");
	ASSERT_FATAL(err_reserve_texels == 0, "Failed to reserve memory for texels!");

	int err_commit_front  = vmap_commit(sim_pixels_mem_size, *front);
	int err_commit_back	  = vmap_commit(sim_pixels_mem_size, *back);
	int err_commit_texels = vmap_commit(sim_pixels_mem_size, *texels);

	ASSERT_FATAL(err_commit_front == 0, "Failed to commit memory for front!");
	ASSERT_FATAL(err_commit_back == 0, "Failed to commit memory for back!");
	ASSERT_FATAL(err_commit_texels == 0, "Failed to commit memory for texels!");

	int err_prefault_front	= vmap_prefault(sim_pixels_mem_size, *front);
	int err_prefault_back	= vmap_prefault(sim_pixels_mem_size, *back);
	int err_prefault_texels = vmap_prefault(sim_pixels_mem_size, *texels);

	ASSERT_FATAL(err_prefault_front == 0, "Failed to prefault memory for front!");
	ASSERT_FATAL(err_prefault_back == 0, "Failed to prefault memory for back!");
	ASSERT_FATAL(err_prefault_texels == 0, "Failed to prefault memory for texels!");

	for (uint32_t i = 0; i < sim_pixel_count; ++i)
	{
		(*front)[i].material_index = MATERIAL_AIR;
		(*back)[i].material_index  = MATERIAL_AIR;
	}
}