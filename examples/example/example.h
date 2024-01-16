#pragma once

#include "input.h"
#include "sound.h"
#include "renderer/ui.h"
#include "state.h"
#include "util/alchemy_memory.h"
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
    Renderer* renderer;

    Font immortal_font;
    Font cardinal_font;
    Font matrix_font;

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

    MemoryArena transient_arena;
} ExampleState;

ALCHEMY_EXPORT UPDATE_AND_RENDER(update_and_render);

// void example_state_delete(ExampleState* state);
