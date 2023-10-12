#pragma once

#include "input.h"
#include "sound.h"
#include "ui.h"
#include "util/types.h"
#include "util/time.h"
#include "renderer/renderer.h"

typedef struct Player
{
    v2 position;
    v2 size;
    v4 color;
    f32 rotation;
    
    int dash_counter;
    int dash_frames;
    f32 dash_direction; // -1: left, 1: right
    f32 dash_distance;
    Timer dash_cooldown;
} Player;

typedef struct ExampleState
{
    Input input;
    Renderer renderer;

    Font immortal_font;
    Font cardinal_font;

    Texture logo_tex;

    Sprite logo;
    v2 logo_direction;
    v4 colors[7];
    int last_color_index;

    Player player;

    SoundOutput sound_output;
    b32 is_shooting;

    Stopwatch stopwatch;

    void* sword_cursor;

    struct nk_alchemy_state alchemy_state;
    struct nk_font* immortal;
} ExampleState;

void example_state_init(ExampleState* state, Window window);
void example_state_delete(ExampleState* state);
void example_update_and_render(ExampleState* state, Window window, f32 delta_time);
