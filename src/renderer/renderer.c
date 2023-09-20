#include "renderer/renderer.h"

#include <glad/glad.h>

void renderer_viewport(int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
}

void renderer_clear(v4 color)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(color.r, color.g, color.b, color.a);
}