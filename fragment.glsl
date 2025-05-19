#version 460 core

uniform usampler2D view_grid;
uniform sampler1D palette;

in vec2 frag_texcoord;
out vec4 out_color;

void main()
{
    ivec2 view_size = textureSize(view_grid, 0);
    vec2 flipped_coord = vec2(frag_texcoord.x, 1.0 - frag_texcoord.y);
    ivec2 texel_coord = ivec2(flipped_coord * vec2(view_size));

    uvec4 texel = texelFetch(view_grid, texel_coord, 0);
    uint material_index = texel.r;

    vec4 base_color = texelFetch(palette, int(material_index), 0);
    out_color = base_color;
}