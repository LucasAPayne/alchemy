#pragma once

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear/nuklear.h>

#include "alchemy/input.h"
#include "alchemy/renderer/font.h"
#include "alchemy/renderer/texture.h"

typedef struct Renderer Renderer;

#define NK_ALCHEMY_TEXT_MAX 256

// TODO(lucas): Remove some things like window width/height and input from this struct
typedef struct UIState
{
    int width, height;
    struct nk_context ctx;
    struct nk_user_font user_font;
    u32 text[NK_ALCHEMY_TEXT_MAX];
    int text_len;

    Keyboard* keyboard;
    Mouse* mouse;
} UIState;

// TODO(lucas): Make default font and default font size
// IMPORTANT: Note that the arena used should live for the lifetime of the UI.
void ui_state_init(Renderer* renderer, Font font, u32 font_size, MemoryArena* arena);
void ui_state_delete(UIState* state);

void ui_new_frame(Renderer* renderer, u32 window_width, u32 window_height);
void ui_render(Renderer* renderer, enum nk_anti_aliasing aa);

void ui_draw_text_area(Renderer* renderer, TextArea* text_area, v2 offset);
