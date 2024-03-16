#pragma once

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
<<<<<<< HEAD
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
=======
>>>>>>> ebd83c9268a6a9fec3725ad1abd65f4521e57b33
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear/nuklear.h>

#include "alchemy/input.h"
<<<<<<< HEAD
=======
#include "alchemy/renderer/font.h"
>>>>>>> ebd83c9268a6a9fec3725ad1abd65f4521e57b33
#include "alchemy/renderer/texture.h"

typedef struct Renderer Renderer;

#define NK_ALCHEMY_TEXT_MAX 256

<<<<<<< HEAD
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
=======
// TODO(lucas): Remove some things like window width/height and input from this struct
typedef struct UIState
{
    int width, height;
    struct nk_context ctx;
    struct nk_user_font user_font;
>>>>>>> ebd83c9268a6a9fec3725ad1abd65f4521e57b33
    u32 text[NK_ALCHEMY_TEXT_MAX];
    int text_len;

    Keyboard* keyboard;
    Mouse* mouse;
<<<<<<< HEAD
} UIRenderState;

UIRenderState ui_render_state_init(u32 shader);
void ui_render_state_shutdown(UIRenderState* state);

void ui_font_stash_begin(Renderer* renderer, struct nk_font_atlas **atlas);
void ui_font_stash_end(Renderer* renderer);
=======
} UIState;

// TODO(lucas): Make default font and default font size
// IMPORTANT: Note that the arena used should live for the lifetime of the UI.
void ui_state_init(Renderer* renderer, Font font, u32 font_size, MemoryArena* arena);
void ui_state_delete(UIState* state);

>>>>>>> ebd83c9268a6a9fec3725ad1abd65f4521e57b33
void ui_new_frame(Renderer* renderer, u32 window_width, u32 window_height);
void ui_render(Renderer* renderer, enum nk_anti_aliasing aa);
