#pragma once

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear/nuklear.h>

#include "input.h"
#include "renderer/texture.h"

typedef struct Renderer Renderer;

#define NK_ALCHEMY_TEXT_MAX 256

typedef struct UIDevice
{
    struct nk_buffer cmds;
    struct nk_draw_null_texture tex_null;
    u32 shader;

    u32 vbo, vao, ebo;
    i32 attrib_pos;
    i32 attrib_uv;
    i32 attrib_col;
    Texture font_tex;
} UIDevice;

// TODO(lucas): Remove some things like window width/height and input from this struct
typedef struct UIRenderState
{
    int width, height;
    int display_width, display_height;
    UIDevice device;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_vec2 fb_scale;
    u32 text[NK_ALCHEMY_TEXT_MAX];
    int text_len;

    Keyboard* keyboard;
    Mouse* mouse;
} UIRenderState;

UIRenderState ui_render_state_init(u32 shader);
void ui_render_state_shutdown(UIRenderState* state);

void ui_font_stash_begin(Renderer* renderer, struct nk_font_atlas **atlas);
void ui_font_stash_end(Renderer* renderer);
void ui_new_frame(Renderer* renderer, u32 window_width, u32 window_height);
void ui_render(Renderer* renderer, enum nk_anti_aliasing aa);
