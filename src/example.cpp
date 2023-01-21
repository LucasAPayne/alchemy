#include "example.h"
#include "types.h"
#include "renderer/font.h"
#include "renderer/shader.h"
#include "renderer/sprite.h"
#include "renderer/texture.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>

#include <stdlib.h> // rand

void init_example_state(example_state* state)
{
    srand(0);
    state->key_input = {0};

    // Compile and Load shaders
    u32 sprite_shader = shader_init("shaders/sprite.vert", "shaders/sprite.frag");
    u32 font_shader = shader_init("shaders/font.vert", "shaders/font.frag");

    init_sprite_renderer(&state->spr_renderer, sprite_shader);
    init_font_renderer(&state->fon_renderer, font_shader);
    load_font(&state->fon_renderer, "fonts/cardinal.ttf", 24);

    u32 logo_tex = generate_texture("textures/dvd.png");
    state->logo = {0};
    state->logo.renderer = &state->spr_renderer;
    state->logo.texture = logo_tex;
    state->logo.color = glm::vec3(1.0f);
    state->logo.position = glm::vec2(0.0f, 0.0f);
    state->logo.size = glm::vec2(300.0f, 150.0f);
    state->logo.rotation = 0.0f;
    state->logo_x_direction = 1;
    state->logo_y_direction = 1;

    shader_set_int(state->spr_renderer.shader, "image", 0);
}

void delete_example_state(example_state* state)
{
    delete_font_renderer(&state->fon_renderer);
    delete_sprite_renderer(&state->spr_renderer);
    delete_texture(state->logo.texture);
}

void example_update_and_render(example_state* state, u32 window_width, u32 window_height)
{
    state->logo.position.x += 1 * state->logo_x_direction;
    state->logo.position.y += 1 * state->logo_y_direction;

    if (state->logo.position.x > (window_width - state->logo.size.x) || state->logo.position.x < 0)
    {
        // Bounce off screen boundary
        state->logo_x_direction *= -1;

        int color_index = rand() % 7;
        // Make sure new color is different. Incredibly efficient
        while (color_index == state->last_color_index)
            color_index = rand() % 7;
        state->logo.color = state->colors[color_index];
        state->last_color_index = color_index;
    }
    if (state->logo.position.y > (window_height - state->logo.size.y) || state->logo.position.y < 0)
    {
        state->logo_y_direction *= -1;

        int color_index = rand() % 7;
        // Make sure new color is different. Incredibly efficient
        while (color_index == state->last_color_index)
            color_index = rand() % 7;
        state->logo.color = state->colors[color_index];
        state->last_color_index = color_index;
    }

    // TODO(lucas): Move this to the renderer
    // TODO(lucas): Sizing window up looks wonky while dragging but fine after releasing mouse.
    glViewport(0, 0, window_width, window_height);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    if (is_key_down(state->key_input, key::A))
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 projection = glm::ortho(0.0f, (f32)window_width, (f32)window_height, 0.0f, -1.0f, 1.0f);
    shader_set_mat4f(state->fon_renderer.shader, "projection", projection);
    shader_set_mat4f(state->spr_renderer.shader, "projection", projection);

    draw_sprite(state->logo);

    glm::vec3 font_color = glm::vec3(0.6f, 0.2f, 0.2f);
    render_text(&state->fon_renderer, "Alchemy Engine", glm::vec2(500.0f, 50.0f), 1.0f, font_color);
}