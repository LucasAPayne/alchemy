#version 330 core
out vec4 frag_color;
in vec4 vert_color;

uniform vec4 color;

void main()
{
    frag_color = vert_color*color;
}
