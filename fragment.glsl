#version 460 core

uniform usampler2D sim_grid;
uniform sampler1D palette;

in vec2 frag_texcoord;
out vec4 out_color;

void main()
{
	ivec2 sim_size  = textureSize(sim_grid, 0);

	// Flip Y-coordinate before scaling
	vec2 flipped_coord = vec2(frag_texcoord.x, 1.0 - frag_texcoord.y);
	ivec2 sim_coord = ivec2(flipped_coord * vec2(sim_size));

	uvec4 sim = texelFetch(sim_grid, sim_coord, 0);
	uint material_index = sim.r;

	vec4 base_color = texelFetch(palette, int(material_index), 0);
	out_color = base_color;
}