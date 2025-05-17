#version 330 core
layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec4 a_color;

uniform mat4 model;
uniform mat4 projection;

out vec4 vert_color;

void main()
{
    gl_Position = projection * model * vec4(a_pos, 0.0, 1.0);
    vert_color = a_color;
}