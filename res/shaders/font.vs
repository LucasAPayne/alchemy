#version 330 core
layout (location = 0) in vec2 a_pos;

out vec2 tex_coords;

uniform mat4 transform;
uniform mat4 projection;

void main()
{
    gl_Position = projection * transform * vec4(a_pos, 0.0, 1.0);
    tex_coords = a_pos.xy;
}
