#pragma once

#include "input.h"
#include "sound.h"
#include "ui.h"
#include "util/types.h"
#include "util/time.h"
#include "renderer/renderer.h"

typedef struct ExampleState
{
    Input input;
    SpriteRenderer sprite_renderer;
    FontRenderer font_renderer;
    FontRenderer frame_time_renderer;
    Texture logo_tex;
    Texture player_tex;

    Sprite logo;
    v2 logo_direction;
    v3* colors;
    int last_color_index;

    Sprite player;
    int dash_counter;
    int dash_frames;
    f32 dash_direction; // -1: left, 1: right
    f32 dash_distance;
    Timer dash_cooldown;

    v4 clear_color;

    SoundOutput sound_output;
    b32 is_shooting;

    Stopwatch stopwatch;

    void* sword_cursor;

    struct nk_alchemy_state alchemy_state;
    struct nk_font* immortal;
} ExampleState;

ExampleState example_state_init(void);
void example_state_delete(ExampleState* state);
void example_update_and_render(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height);
