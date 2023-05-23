#pragma once

#include "input.h"
#include "sound.h"
#include "types.h"
#include "renderer/font.h"
#include "renderer/sprite.h"

typedef struct ExampleState
{
    Input input;
    SpriteRenderer sprite_renderer;
    FontRenderer font_renderer;
    Sprite logo;
    f32 logo_x_direction;
    f32 logo_y_direction;
    vec3 colors[7] = {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
                       {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f},
                       {0.0f, 1.0f, 1.0f}};
    int last_color_index;

    Sprite player;
    int dash_counter;
    int dash_frames;
    f32 dash_direction; // -1: left, 1: right
    f32 dash_distance;

    vec3 clear_color;

    SoundOutput sound_output;
    b32 is_shooting;
} ExampleState;

void init_example_state(ExampleState* state);
void delete_example_state(ExampleState* state);
void example_update_and_render(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height);
