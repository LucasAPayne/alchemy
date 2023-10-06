#pragma once

#include "window.h"
#include "renderer/font.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "renderer/sprite.h"
#include "util/alchemy_math.h"
#include "util/types.h"

typedef struct RenderObject
{
    u32 shader;
    u32 vao;
    u32 vbo;
    u32 ibo;
} RenderObject;

typedef struct Framebuffer
{
    u32 id;
    u32 rbo;
    Texture texture;
    RenderObject renderer;
} Framebuffer;

typedef struct RendererConfig
{
    u32 circle_line_segments;
} RendererConfig;

typedef struct Renderer
{
    RenderObject line_renderer;
    RenderObject circle_renderer;
    RenderObject rect_renderer;
    RenderObject sprite_renderer;
    RenderObject font_renderer;

    Framebuffer framebuffer;

    rect viewport;
    v4 clear_color;

    RendererConfig config;
} Renderer;

Renderer renderer_init(int viewport_width, int viewport_height);
void renderer_delete(Renderer* renderer);

void renderer_new_frame(Renderer* renderer, Window window);
void renderer_render(Renderer* renderer);

void renderer_viewport(rect viewport);
void renderer_clear(v4 color);

// TODO(lucas): Line, Triangle, Quad structs?
void draw_line(Renderer* renderer, v2 start, v2 end, v4 color);
void draw_circle(Renderer* renderer, v2 position, f32 radius, v4 color);
void draw_rect(Renderer* renderer, v2 position, v2 size, v4 color, f32 rotation);
