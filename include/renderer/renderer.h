#pragma once

#include "renderer/font.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/sprite.h"
#include "util/types.h"

typedef struct RenderObject
{
    u32 shader;
    u32 vao;
    u32 vbo;
    u32 ibo;
} RenderObject;

typedef struct Renderer
{
    RenderObject line_renderer;
    RenderObject rect_renderer;
    RenderObject sprite_renderer;
    RenderObject font_renderer;
} Renderer;

Renderer renderer_init();
void renderer_delete(Renderer* renderer);

void renderer_viewport(Renderer* renderer, int x, int y, int width, int height);
void renderer_clear(v4 color);

// TODO(lucas): Line, Triangle, Quad structs?
void draw_line(Renderer* renderer, v2 start, v2 end, v4 color);
void draw_rect(Renderer* renderer, v2 position, v2 size, v4 color, f32 rotation);
