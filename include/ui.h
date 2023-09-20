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
#include "renderer/renderer.h"

#define NK_ALCHEMY_TEXT_MAX 256

typedef struct nk_alchemy_device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture tex_null;
    u32 shader;

    u32 vbo, vao, ebo;
    i32 attrib_pos;
    i32 attrib_uv;
    i32 attrib_col;
    Texture font_tex;
} nk_alchemy_device;

typedef struct nk_alchemy_state {
    int width, height;
    int display_width, display_height;
    nk_alchemy_device device;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_vec2 fb_scale;
    u32 text[NK_ALCHEMY_TEXT_MAX];
    int text_len;

    Keyboard* keyboard;
    Mouse* mouse;
} nk_alchemy_state;

struct nk_context    nk_alchemy_init(nk_alchemy_state* state, u32 ui_shader);
void                 nk_alchemy_shutdown(nk_alchemy_state* state);
void                 nk_alchemy_font_stash_begin(nk_alchemy_state* state, struct nk_font_atlas **atlas);
void                 nk_alchemy_font_stash_end(nk_alchemy_state* state);
void                 nk_alchemy_new_frame(nk_alchemy_state* state, u32 window_width, u32 window_height);
void                 nk_alchemy_render(nk_alchemy_state* state, enum nk_anti_aliasing);

void                 nk_alchemy_device_destroy(nk_alchemy_state* state);
void                 nk_alchemy_device_create(nk_alchemy_state* state, u32 ui_shader);
