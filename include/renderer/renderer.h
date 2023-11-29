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
} Framebuffer;

typedef struct RendererConfig
{
    b32 wireframe_mode;
    b32 rotate_quad_from_center;
    u32 circle_line_segments;
    int msaa_level;
} RendererConfig;

typedef struct Renderer
{
    RenderObject circle_renderer;
    RenderObject rect_renderer;
    RenderObject sprite_renderer;
    RenderObject font_renderer;
    RenderObject framebuffer_renderer;

    // NOTE(lucas): If MSAA is disabled, then the intermediate framebuffer is unused.
    // Otherwise, the main framebufer is used for multisampling operations and is
    // blitted to the intermediate framebuffer. Any post-processing effects will be applied
    // to the intermediate framebuffer.
    Framebuffer framebuffer;
    Framebuffer intermediate_framebuffer;

    rect viewport;
    int window_width;
    int window_height;
    v4 clear_color;

    RendererConfig config;
    MemoryArena scratch_arena;
} Renderer;

Renderer renderer_init(int viewport_width, int viewport_height);
void renderer_delete(Renderer* renderer);

void renderer_new_frame(Renderer* renderer, Window window);
void renderer_render(Renderer* renderer);

void renderer_viewport(Renderer* renderer, rect viewport);
void renderer_clear(v4 color);

// TODO(lucas): Line, Triangle, Quad structs?
void draw_line(Renderer* renderer, v2 start, v2 end, v4 color, f32 thickness);
void draw_circle(Renderer* renderer, v2 position, f32 radius, v4 color);
void draw_quad(Renderer* renderer, v2 position, v2 size, v4 color, f32 rotation);

inline v4 color_red(void)
{
    return (v4){1.0f, 0.0f, 0.0f, 1.0f};
}

inline v4 color_green(void)
{
    return (v4){0.0f, 1.0f, 0.0f, 1.0f};
}

inline v4 color_blue(void)
{
    return (v4){0.0f, 0.0f, 1.0f, 1.0f};
}

inline v4 color_black(void)
{
    return (v4){0.0f, 0.0f, 0.0f, 1.0f};
}

inline v4 color_white(void)
{
    return (v4){1.0f, 1.0f, 1.0f, 1.0f};
}

inline v4 color_cyan(void)
{
    return (v4){0.0f, 1.0f, 1.0f, 1.0f};
}

inline v4 color_magenta(void)
{
    return (v4){1.0f, 0.0f, 1.0f, 1.0f};
}

inline v4 color_yellow(void)
{
    return (v4){1.0f, 1.0f, 0.0f, 1.0f};
}

inline v4 color_transparent(void)
{
    return (v4){0.0f, 0.0f, 0.0f, 0.0f};
}
