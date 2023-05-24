#include "example.h"
#include "types.h"
#include "renderer/font.h"
#include "renderer/shader.h"
#include "renderer/sprite.h"
#include "renderer/texture.h"

#include <glad/glad.h>

#include <stdlib.h> // rand
#include <stdio.h>  // sprintf_s
#include <string.h> // Temporary

internal void update_dvd(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height)
{
    f32 speed = 100.0f; // pixels per second

    state->logo.position[0] += speed * delta_time * state->logo_x_direction;
    state->logo.position[1] += speed * delta_time * state->logo_y_direction;

    // TODO(lucas): Clamp position to boundaries
    if (state->logo.position[0] > (window_width - state->logo.size[0]) || state->logo.position[0] < 0)
    {
        // Bounce off screen boundary
        state->logo_x_direction *= -1.0f;

        int color_index = rand() % 7;
        // Make sure new color is different. Incredibly efficient
        while (color_index == state->last_color_index)
            color_index = rand() % 7;
        glm_vec3_copy(state->colors[color_index], state->logo.color);
        state->last_color_index = color_index;
    }
    if (state->logo.position[1] > (window_height - state->logo.size[1]) || state->logo.position[1] < 0)
    {
        state->logo_y_direction *= -1.0f;

        int color_index = rand() % 7;
        // Make sure new color is different. Incredibly efficient
        while (color_index == state->last_color_index)
            color_index = rand() % 7;
        glm_vec3_copy(state->colors[color_index], state->logo.color);
        state->last_color_index = color_index;
    }
}

internal void update_player(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height)
{
    Gamepad* gamepad = &state->input.gamepads[0];
    f32 speed = 250.0f; // pixels per second
    // Update player position
    state->player.position[0] += speed * delta_time * gamepad->left_stick_x;
    state->player.position[1] += speed * delta_time * gamepad->left_stick_y;

    // Dash
    if (is_gamepad_button_released(gamepad->left_shoulder) && state->dash_counter == 0)
    {
        state->dash_counter = state->dash_frames;
        state->dash_direction = -1.0f;
    }
    if (is_gamepad_button_released(gamepad->right_shoulder) && state->dash_counter == 0)
    {
        state->dash_counter = state->dash_frames;
        state->dash_direction = 1.0f;
    }
    if (state->dash_counter > 0)
    {
        f32 dash_delta = state->dash_distance / (f32)(state->dash_frames);
        state->player.position[0] += dash_delta * state->dash_direction;
        state->dash_counter--;
    }

    // Bounds checking
    if (state->player.position[0] > (window_width - state->player.size[0]))
    {
        state->player.position[0] = window_width - state->player.size[0];
    }
    if (state->player.position[0] < 0.0f)
    {
        state->player.position[0] = 0.0f;
    }
    if (state->player.position[1] > (window_height - state->player.size[1]))
    {
        state->player.position[1] = window_height - state->player.size[1];
    }
    if (state->player.position[1] < 0.0f)
    {
        state->player.position[1] = 0.0f;
    }

    // Update player rotation
    state->player.rotation += 2.0f * gamepad->right_trigger_val;
    state->player.rotation -= 2.0f * gamepad->left_trigger_val;
    if (state->player.rotation > 45.0f)
        state->player.rotation = 45.0f;
    if (state->player.rotation < -45.0f)
        state->player.rotation = -45.0f;
    if (!is_gamepad_button_pressed(gamepad->left_trigger) &&
        !is_gamepad_button_pressed(gamepad->right_trigger))
    {
        if (state->player.rotation > 0.0f)
            state->player.rotation -= 2.0f;
        if (state->player.rotation < 0.0f)
            state->player.rotation += 2.0f;
        if (fabs(state->player.rotation - 0.0f) < 2.0f)
            state->player.rotation = 0.0f;
    }

    // Vibration test
    if (is_gamepad_button_pressed(gamepad->x_button))
        gamepad_set_vibration(gamepad, 16000, 16000);
}

void init_example_state(ExampleState* state)
{
    srand(0);
    state->input.keyboard = {0};
    state->input.mouse = {0};
    for (int i = 0; i < MAX_GAMEPADS; ++i)
        state->input.gamepads[i] = {0};

    // Compile and Load shaders
    u32 sprite_shader = shader_init("shaders/sprite.vert", "shaders/sprite.frag");
    u32 font_shader = shader_init("shaders/font.vert", "shaders/font.frag");

    init_sprite_renderer(&state->sprite_renderer, sprite_shader);
    init_font_renderer(&state->font_renderer, font_shader, "fonts/cardinal.ttf");
    init_font_renderer(&state->frame_time_renderer, font_shader, "fonts/immortal.ttf");

    vec2 logo_size = {300.0f, 150.0f};
    vec3 clear_color = {0.2f, 0.2f, 0.2f};

    u32 logo_tex = generate_texture_from_file("textures/dvd.png");
    state->logo = {0};
    state->logo.renderer = &state->sprite_renderer;
    state->logo.texture = logo_tex;
    glm_vec3_copy(state->colors[0], state->logo.color);
    glm_vec2_zero(state->logo.position);
    glm_vec2_copy(logo_size, state->logo.size);
    state->logo.rotation = 0.0f;
    state->logo_x_direction = 1.0f;
    state->logo_y_direction = 1.0f;
    glm_vec3_copy(clear_color, state->clear_color);

    vec2 player_size = {50.0f, 50.0f};

    u32 player_tex = generate_texture_from_file("textures/white_pixel.png");
    state->player = {0};
    state->player.renderer = &state->sprite_renderer;
    state->player.texture = player_tex;
    glm_vec3_one(state->player.color);
    glm_vec3_zero(state->player.position);
    glm_vec3_copy(player_size, state->player.size);
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
    delete_font_renderer(&state->frame_time_renderer);
    delete_sprite_renderer(&state->sprite_renderer);
    delete_texture(state->logo.texture);
    delete_texture(state->player.texture);
}

void example_update_and_render(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height)
{
    Gamepad* gamepad = &state->input.gamepads[0];
    update_dvd(state, delta_time, window_width, window_height);
    update_player(state, delta_time, window_width, window_height);  
    
    // TODO(lucas): Sizing window up looks wonky while dragging but fine after releasing mouse.
    glViewport(0, 0, window_width, window_height);
    glClearColor(state->clear_color[0], state->clear_color[1], state->clear_color[1], 1.0f);

    // if (is_key_released(&state->keyboard, Key::A))
    if (is_mouse_button_pressed(&state->input.mouse, MouseButton::MOUSE_X2))
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    
    state->sound_output.should_play = false;
    if (is_gamepad_button_pressed(gamepad->a_button) && !state->is_shooting)
    {
        state->is_shooting = true;
        state->sound_output.should_play = true;
    }
    if (is_gamepad_button_released(gamepad->a_button))
        state->is_shooting = false;
    
    glClear(GL_COLOR_BUFFER_BIT);

    mat4 projection;
    glm_ortho(0.0f, (f32)window_width, (f32)window_height, 0.0f, -1.0f, 1.0f, projection); 
    shader_set_mat4f(state->font_renderer.shader, "projection", projection, 0);
    shader_set_mat4f(state->sprite_renderer.shader, "projection", projection, 0);

    draw_sprite(state->logo);
    draw_sprite(state->player);

    vec4 font_color = {0.6f, 0.2f, 0.2f, 1.0f};
    vec2 engine_text_pos = {500.0f, 50.0f};
    vec2 ms_text_pos = {10.0f, window_height - 10.0f};
    render_text(&state->font_renderer, "Alchemy Engine", engine_text_pos, 48, font_color);
    char buffer[512];

    FontRenderer frame_time_renderer = {0};
    sprintf_s(buffer, sizeof(buffer), "MS/frame: %.2f", delta_time * 1000.0f);
    render_text(&state->frame_time_renderer, buffer, ms_text_pos, 32, font_color);
}
