#pragma once

#include "alchemy/window.h"
#include "alchemy/renderer/font.h"
#include "alchemy/renderer/shader.h"
#include "alchemy/renderer/sprite.h"
#include "alchemy/renderer/texture.h"
#include "alchemy/renderer/ui.h"
#include "alchemy/util/math.h"
#include "alchemy/util/types.h"

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
    u32 circle_line_segments;
    int msaa_level;
} RendererConfig;

typedef enum RenderCommandType
{
    RENDER_COMMAND_RenderCommandLine,
    RENDER_COMMAND_RenderCommandTriangle,
    RENDER_COMMAND_RenderCommandTriangleOutline,
    RENDER_COMMAND_RenderCommandQuad,
    RENDER_COMMAND_RenderCommandQuadOutline,
    RENDER_COMMAND_RenderCommandQuadGradient,
    RENDER_COMMAND_RenderCommandCircle,
    RENDER_COMMAND_RenderCommandSprite,
    RENDER_COMMAND_RenderCommandText
} RenderCommandType;

typedef struct RenderCommand
{
    RenderCommandType type;
} RenderCommand;

typedef struct RenderCommandLine
{
    RenderCommand header;
    v4 color;
    v2 start;
    v2 end;
    v2 origin;
    f32 thickness;
    f32 rotation;
} RenderCommandLine;

typedef struct RenderCommandTriangle
{
    RenderCommand header;
    v2 a;
    v2 b;
    v2 c;
    v2 origin;
    v4 color;
    f32 rotation;
} RenderCommandTriangle;

typedef struct RenderCommandTriangleOutline
{
    RenderCommand header;
    v2 a;
    v2 b;
    v2 c;
    v2 origin;
    v4 color;
    f32 thickness;
    f32 rotation;
} RenderCommandTriangleOutline;

typedef struct RenderCommandQuad
{
    RenderCommand header;
    v2 position;
    v2 origin;
    v2 size;
    v4 color;
    f32 rotation;
} RenderCommandQuad;

typedef struct RenderCommandQuadOutline
{
    RenderCommand header;
    v2 position;
    v2 origin;
    v2 size;
    v4 color;
    f32 thickness;
    f32 rotation;
} RenderCommandQuadOutline;

typedef struct RenderCommandQuadGradient
{
    RenderCommand header;
    v2 position;
    v2 origin;
    v2 size;
    v4 color_left;
    v4 color_bottom;
    v4 color_right;
    v4 color_top;
    f32 rotation;
} RenderCommandQuadGradient;

typedef struct RenderCommandCircle
{
    RenderCommand header;
    v2 position;
    v4 color;
    f32 radius;
} RenderCommandCircle;

typedef struct RenderCommandSprite
{
    RenderCommand header;
    Sprite sprite;
} RenderCommandSprite;

typedef struct RenderCommandText
{
    RenderCommand header;
    Text text;
} RenderCommandText;

typedef struct RenderCommandBuffer
{
    usize max_size;
    usize size;
    u8* base;
} RenderCommandBuffer;

typedef struct RenderID
{
    u32 id;
    b32 used;
} RenderID;

typedef enum ResourceType
{
    RESOURCE_TYPE_TEXTURE = 0,
    RESOURCE_TYPE_SHADER
} ResourceType;

typedef struct ResourceLoadInfo
{
    char* filename;
    ResourceType type;
} ResourceLoadInfo;

typedef struct Renderer
{
    RenderCommandBuffer command_buffer;

    RenderObject triangle_renderer;
    RenderObject quad_renderer;
    RenderObject circle_renderer;

    RenderObject sprite_renderer;
    RenderObject font_renderer;
    RenderObject framebuffer_renderer;
    RenderObject ui_renderer;

    u32 poly_shader;
    u32 poly_border_shader;

    UIRenderState ui_render_state;

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
    MemoryArena command_buffer_arena;
    MemoryArena scratch_arena;

    RenderID tex_ids[64];
    Texture textures_to_generate[64];
    u32 tex_index;
} Renderer;

void opengl_init(Window window);

Renderer renderer_init(Window window, int viewport_width, int viewport_height, usize command_buffer_size);
void renderer_delete(Renderer* renderer);

void renderer_new_frame(Renderer* renderer, Window window);
void renderer_render(Renderer* renderer);

void renderer_viewport(Renderer* renderer, rect viewport);
void renderer_clear(v4 color);

// TODO(lucas): Add additional functions that take in origins, and consider taking rotation out of the default functions
void draw_line(Renderer* renderer, v2 start, v2 end, v4 color, f32 thickness, f32 rotation);

// NOTE(lucas): Vertices must be specified in couter-clockwise order!
void draw_triangle(Renderer* renderer, v2 a, v2 b, v2 c, v4 color, f32 rotation);
void draw_triangle_outline(Renderer* renderer, v2 a, v2 b, v2 c, v4 color, f32 thickness, f32 rotation);

// TODO(lucas): Rounded edges option
// TODO(lucas): Add functions to take in rect instead of position/size or start/end
void draw_quad(Renderer* renderer, v2 position, v2 size, v4 color, f32 rotation);
void draw_quad_outline(Renderer* renderer, v2 position, v2 size, v4 color, f32 thickness, f32 rotation);
void draw_quad_gradient(Renderer* renderer, v2 position, v2 size, v4 color_left, v4 color_bottom, v4 color_right,
                        v4 color_top, f32 rotation);

void draw_circle(Renderer* renderer, v2 position, f32 radius, v4 color);

void draw_sprite(Renderer* renderer, Sprite sprite);
void draw_text(Renderer* renderer, Text text);

u32 renderer_next_tex_id(Renderer* renderer);
void renderer_push_texture(Renderer* renderer, Texture texture);

inline v4 color_red(void)         {return (v4){1.0f, 0.0f, 0.0f, 1.0f};}
inline v4 color_green(void)       {return (v4){0.0f, 1.0f, 0.0f, 1.0f};}
inline v4 color_blue(void)        {return (v4){0.0f, 0.0f, 1.0f, 1.0f};}
inline v4 color_black(void)       {return (v4){0.0f, 0.0f, 0.0f, 1.0f};}
inline v4 color_white(void)       {return (v4){1.0f, 1.0f, 1.0f, 1.0f};}
inline v4 color_cyan(void)        {return (v4){0.0f, 1.0f, 1.0f, 1.0f};}
inline v4 color_magenta(void)     {return (v4){1.0f, 0.0f, 1.0f, 1.0f};}
inline v4 color_yellow(void)      {return (v4){1.0f, 1.0f, 0.0f, 1.0f};}
inline v4 color_transparent(void) {return (v4){0.0f, 0.0f, 0.0f, 0.0f};}

// NOTE(lucas): For internal use only
void output_line(Renderer* renderer, v2 start, v2 end, v2 origin, v4 color, f32 thickness, f32 rotation);

void output_triangle(Renderer* renderer, v2 a, v2 b, v2 c, v2 origin, v4 color, f32 rotation);
void output_triangle_outline(Renderer* renderer, v2 a, v2 b, v2 c, v2 origin, v4 color, f32 thickness, f32 rotation);

void output_quad(Renderer* renderer, v2 position, v2 origin, v2 size, v4 color, f32 rotation);
void output_quad_outline(Renderer* renderer, v2 position, v2 origin, v2 size, v4 color, f32 thickness, f32 rotation);
void output_quad_gradient(Renderer* renderer, v2 position, v2 origin, v2 size, v4 color_left, v4 color_bottom,
                          v4 color_right, v4 color_top, f32 rotation);

void output_circle(Renderer* renderer, v2 position, f32 radius, v4 color);
