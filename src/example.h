#pragma once

#include "input.h"
#include "types.h"
#include "renderer/font.h"
#include "renderer/sprite.h"

struct example_state
{
    keyboard_input key_input;

    sprite_renderer spr_renderer;
    font_renderer fon_renderer;
    sprite logo;
    int logo_x_direction;
    int logo_y_direction;
    glm::vec3 colors[7] = {glm::vec3(1.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                     glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 1.0f),
                     glm::vec3(0.0f, 1.0f, 1.0f)};
    int last_color_index;
};

void init_example_state(example_state* state);
void delete_example_state(example_state* state);
void example_update_and_render(example_state* state, u32 window_width, u32 window_height);
