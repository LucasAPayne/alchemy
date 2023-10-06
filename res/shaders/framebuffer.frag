#version 330 core

in vec2 tex_coords;
uniform sampler2D screen_texture;

out vec4 frag_color;

void main()
{
    frag_color = texture(screen_texture, tex_coords);
}
