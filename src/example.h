#pragma once

#include "input.h"
#include "sound.h"
#include "util/types.h"
#include "util/time.h"
#include "renderer/font.h"
#include "renderer/sprite.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
// #define NK_GLFW_GL3_IMPLEMENTATION
// #define NK_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#pragma warning(push, 0)
#include <nuklear/nuklear.h>
#pragma warning(pop)

#include "nuklear_example.h"

typedef struct ExampleState
{
    Input input;
    SpriteRenderer sprite_renderer;
    FontRenderer font_renderer;
    FontRenderer frame_time_renderer;
    Sprite logo;
    f32 logo_x_direction;
    f32 logo_y_direction;
    vec3s* colors;
    int last_color_index;

    Sprite player;
    int dash_counter;
    int dash_frames;
    f32 dash_direction; // -1: left, 1: right
    f32 dash_distance;
    Timer dash_cooldown;

    vec3s clear_color;

    SoundOutput sound_output;
    b32 is_shooting;

    Stopwatch stopwatch;

    // nk_user_font debug_panel_font;
    // nk_context debug_panel;

    struct nk_alchemy_state alchemy_state;
    struct nk_context* ctx;
    struct nk_colorf bg;
} ExampleState;

void init_example_state(ExampleState* state);
void delete_example_state(ExampleState* state);
void example_update_and_render(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height);
