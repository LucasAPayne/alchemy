#pragma once

#include "gamepad.h"
#include "keyboard.h"
#include "types.h"
#include "renderer/font.h"
#include "renderer/sprite.h"

struct ExampleState
{
    KeyboardInput keyboard;
    Gamepad gamepad;

    SpriteRenderer sprite_renderer;
    FontRenderer font_renderer;
    Sprite logo;
    int logo_x_direction;
    int logo_y_direction;
    glm::vec3 colors[7] = {glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                     glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 1.0f),
                     glm::vec3(0.0f, 1.0f, 1.0f)};
    int last_color_index;

    Sprite player;
    int dash_counter;
    int dash_frames;
    f32 dash_direction; // -1: left, 1: right
    f32 dash_distance;

    glm::vec3 clear_color;
};

void init_example_state(ExampleState* state);
void delete_example_state(ExampleState* state);
void example_update_and_render(ExampleState* state, u32 window_width, u32 window_height);
