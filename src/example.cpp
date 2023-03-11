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
#include <stdio.h>  // sprintf_s
#include <string.h> // Temporary

internal void update_dvd(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height)
{
    f32 speed = 100.0f; // pixels per second

    state->logo.position.x += speed * delta_time * state->logo_x_direction;
    state->logo.position.y += speed * delta_time * state->logo_y_direction;

    // TODO(lucas): Clamp position to boundaries
    if (state->logo.position.x > (window_width - state->logo.size.x) || state->logo.position.x < 0)
    {
        // Bounce off screen boundary
        state->logo_x_direction *= -1.0f;

        int color_index = rand() % 7;
        // Make sure new color is different. Incredibly efficient
        while (color_index == state->last_color_index)
            color_index = rand() % 7;
        state->logo.color = state->colors[color_index];
        state->last_color_index = color_index;
    }
    if (state->logo.position.y > (window_height - state->logo.size.y) || state->logo.position.y < 0)
    {
        state->logo_y_direction *= -1.0f;

        int color_index = rand() % 7;
        // Make sure new color is different. Incredibly efficient
        while (color_index == state->last_color_index)
            color_index = rand() % 7;
        state->logo.color = state->colors[color_index];
        state->last_color_index = color_index;
    }
}

internal void update_player(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height)
{
    f32 speed = 250.0f; // pixels per second
    // Update player position
    state->player.position.x += speed * delta_time * state->gamepad.left_stick_x;
    state->player.position.y += speed * delta_time * state->gamepad.left_stick_y;

    // Dash
    if (is_gamepad_button_released(state->gamepad.left_shoulder) && state->dash_counter == 0)
    {
        state->dash_counter = state->dash_frames;
        state->dash_direction = -1.0f;
    }
    if (is_gamepad_button_released(state->gamepad.right_shoulder) && state->dash_counter == 0)
    {
        state->dash_counter = state->dash_frames;
        state->dash_direction = 1.0f;
    }
    if (state->dash_counter > 0)
    {
        f32 dash_delta = state->dash_distance / (f32)(state->dash_frames);
        state->player.position.x += dash_delta * state->dash_direction;
        state->dash_counter--;
    }

    // Bounds checking
    if (state->player.position.x > (window_width - state->player.size.x))
    {
        state->player.position.x = window_width - state->player.size.x;
    }
    if (state->player.position.x < 0.0f)
    {
        state->player.position.x = 0.0f;
    }
    if (state->player.position.y > (window_height - state->player.size.y))
    {
        state->player.position.y = window_height - state->player.size.y;
    }
    if (state->player.position.y < 0.0f)
    {
        state->player.position.y = 0.0f;
    }

    // Update player rotation
    state->player.rotation += 2.0f * state->gamepad.right_trigger_val;
    state->player.rotation -= 2.0f * state->gamepad.left_trigger_val;
    if (state->player.rotation > 45.0f)
        state->player.rotation = 45.0f;
    if (state->player.rotation < -45.0f)
        state->player.rotation = -45.0f;
    if (!is_gamepad_button_pressed(state->gamepad.left_trigger) &&
        !is_gamepad_button_pressed(state->gamepad.right_trigger))
    {
        if (state->player.rotation > 0.0f)
            state->player.rotation -= 2.0f;
        if (state->player.rotation < 0.0f)
            state->player.rotation += 2.0f;
        if (abs(state->player.rotation - 0.0f) < 2.0f)
            state->player.rotation = 0.0f;
    }

    // Vibration test
    if (is_gamepad_button_pressed(state->gamepad.x_button))
        gamepad_set_vibration(&state->gamepad, 16000, 16000);
}

void init_example_state(ExampleState* state)
{
    srand(0);
    state->keyboard = {0};
    state->gamepad = {0};

    // Compile and Load shaders
    u32 sprite_shader = shader_init("shaders/sprite.vert", "shaders/sprite.frag");
    u32 font_shader = shader_init("shaders/font.vert", "shaders/font.frag");

    init_sprite_renderer(&state->sprite_renderer, sprite_shader);
    init_font_renderer(&state->font_renderer, font_shader);
    load_font(&state->font_renderer, "fonts/cardinal.ttf", 24);

    u32 logo_tex = generate_texture("textures/dvd.png");
    state->logo = {0};
    state->logo.renderer = &state->sprite_renderer;
    state->logo.texture = logo_tex;
    state->logo.color = glm::vec3(1.0f);
    state->logo.position = glm::vec2(0.0f, 0.0f);
    state->logo.size = glm::vec2(300.0f, 150.0f);
    state->logo.rotation = 0.0f;
    state->logo_x_direction = 1.0f;
    state->logo_y_direction = 1.0f;
    state->clear_color = glm::vec3(0.2f, 0.2f, 0.2f);

    u32 player_tex = generate_texture("textures/white_pixel.png");
    state->player = {0};
    state->player.renderer = &state->sprite_renderer;
    state->player.texture = player_tex;
    state->player.color = glm::vec3(1.0f);
    state->player.position = glm::vec2(0.0f, 0.0f);
    state->player.size = glm::vec2(50.0f, 50.0f);
    state->player.rotation = 0.0f;
    state->dash_counter = 0;
    state->dash_frames = 15;
    state->dash_direction = 0.0f;
    state->dash_distance = 300.0f;

    shader_set_int(state->sprite_renderer.shader, "image", 0);

    const char* test_sound_filename = "sounds/pew.wav";
    strncpy_s(state->sound_output.filename, sizeof(state->sound_output.filename), test_sound_filename,
              strlen(test_sound_filename));
    set_volume(&state->sound_output, 0.5f);
    state->sound_output.should_play = false;
    state->is_shooting = false;
}

void delete_example_state(ExampleState* state)
{
    delete_font_renderer(&state->font_renderer);
    delete_sprite_renderer(&state->sprite_renderer);
    delete_texture(state->logo.texture);
    delete_texture(state->player.texture);
}

void example_update_and_render(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height)
{
    update_dvd(state, delta_time, window_width, window_height);
    update_player(state, delta_time, window_width, window_height);  
    
    // TODO(lucas): Sizing window up looks wonky while dragging but fine after releasing mouse.
    glViewport(0, 0, window_width, window_height);
    glClearColor(state->clear_color.x, state->clear_color.y, state->clear_color.z, 1.0f);

    if (is_key_released(&state->keyboard, Key::A))
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    
    state->sound_output.should_play = false;
    if (is_gamepad_button_pressed(state->gamepad.a_button) && !state->is_shooting)
    {
        state->is_shooting = true;
        state->sound_output.should_play = true;
    }
    if (is_gamepad_button_released(state->gamepad.a_button))
        state->is_shooting = false;
    
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 projection = glm::ortho(0.0f, (f32)window_width, (f32)window_height, 0.0f, -1.0f, 1.0f);
    shader_set_mat4f(state->font_renderer.shader, "projection", projection);
    shader_set_mat4f(state->sprite_renderer.shader, "projection", projection);

    draw_sprite(state->logo);
    draw_sprite(state->player);

    glm::vec3 font_color = glm::vec3(0.6f, 0.2f, 0.2f);
    render_text(&state->font_renderer, "Alchemy Engine", glm::vec2(500.0f, 50.0f), 1.0f, font_color);
    char buffer[512];
    sprintf_s(buffer, sizeof(buffer), "MS per frame: %f", delta_time * 1000.0f);
    render_text(&state->font_renderer, buffer, glm::vec2(750.0f, 650.0f), 1.0f, font_color);
}
