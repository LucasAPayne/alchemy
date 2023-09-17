#include "example.h"
#include "renderer/renderer.h"
#include "util/alchemy_math.h"
#include "util/types.h"

#include "ui_overview.h"

#include <stdlib.h> // rand
#include <stdio.h>  // Temporary: sprintf_s
#include <string.h> // Temporary

global_variable v4 colors[7] = {{1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f},
                                {0.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.0f},
                                {0.0f, 1.0f, 1.0f, 1.0f}};

internal void bounce_dvd(ExampleState* state, f32* direction)
{
    // Bounce off screen boundary
    *direction *= -1.0f;

    int color_index = rand() % 7;
    // Make sure new color is different. Incredibly efficient
    while (color_index == state->last_color_index)
        color_index = rand() % 7;
    state->logo.color = colors[color_index];
    state->last_color_index = color_index;
}

internal void update_dvd(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height)
{
    f32 speed = 100.0f; // pixels per second
    rect window_bounds = rect_min_max((v2){0.0f, 0.0f},
                                      (v2){(f32)(window_width - state->logo.size.x),
                                           (f32)(window_height - state->logo.size.y)});

    state->logo.position = v2_add(state->logo.position, v2_scale(state->logo_direction, speed*delta_time));

    if ((state->logo.position.x > window_bounds.max.x) || (state->logo.position.x < window_bounds.min.x))
        bounce_dvd(state, &state->logo_direction.x);
    if ((state->logo.position.y > window_bounds.max.y) || (state->logo.position.y < 0.0f))
        bounce_dvd(state, &state->logo_direction.y);

    state->logo.position = v2_clamp_to_rect(state->logo.position, window_bounds);
}

internal void update_player(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height)
{
    timer_update(&state->dash_cooldown, delta_time, true);

    Gamepad* gamepad = &state->input.gamepads[0];
    f32 speed = 250.0f; // pixels per second

    rect window_bounds = rect_min_max((v2){0.0f, 0.0f},
                                      (v2){(f32)(window_width - state->player.size.x),
                                           (f32)(window_height - state->player.size.y)});

    // Update player position
    v2 player_delta = v2_scale((v2){gamepad->left_stick_x, gamepad->left_stick_y}, speed*delta_time);
    state->player.position = v2_add(state->player.position, player_delta);

    // Dash
    if (!state->dash_cooldown.is_active)
    {
        if (gamepad_button_released(gamepad->left_shoulder) && state->dash_counter == 0)
        {
            state->dash_counter = state->dash_frames;
            state->dash_direction = -1.0f;
            timer_start(&state->dash_cooldown);
        }
        if (gamepad_button_released(gamepad->right_shoulder) && state->dash_counter == 0)
        {
            state->dash_counter = state->dash_frames;
            state->dash_direction = 1.0f;
            timer_start(&state->dash_cooldown);
        }
    }
    if (state->dash_counter > 0)
    {
        f32 dash_delta = state->dash_distance / (f32)(state->dash_frames);
        state->player.position.x += dash_delta * state->dash_direction;
        state->dash_counter--;
    }

    // Bounds checking
    state->player.position = v2_clamp_to_rect(state->player.position, window_bounds);

    // Update player rotation
    state->player.rotation += 2.0f * gamepad->right_trigger_val;
    state->player.rotation -= 2.0f * gamepad->left_trigger_val;
    state->player.rotation = f32_clamp(state->player.rotation, -45.0f, 45.0f);
    if (!gamepad_button_pressed(gamepad->left_trigger) &&
        !gamepad_button_pressed(gamepad->right_trigger))
    {
        if (state->player.rotation > 0.0f)
            state->player.rotation -= 2.0f;
        if (state->player.rotation < 0.0f)
            state->player.rotation += 2.0f;
        if (fabs(state->player.rotation) < 2.0f)
            state->player.rotation = 0.0f;
    }

    // Vibration test
    if (gamepad_button_pressed(gamepad->x_button))
        gamepad_set_vibration(gamepad, 16000, 16000);
}

void example_state_init(ExampleState* state)
{
    srand(0);
    *state = (ExampleState){0};

    // Compile and Load shaders
    u32 sprite_shader = shader_init("shaders/sprite.vert", "shaders/sprite.frag");
    u32 font_shader = shader_init("shaders/font.vert", "shaders/font.frag");
    u32 ui_shader = shader_init("shaders/ui.vert", "shaders/ui.frag");

    sprite_renderer_init(&state->sprite_renderer, sprite_shader);
    font_renderer_init(&state->font_renderer, font_shader, "fonts/cardinal.ttf");
    font_renderer_init(&state->frame_time_renderer, font_shader, "fonts/immortal.ttf");

    state->logo_tex = texture_load_from_file("textures/dvd.png");
    state->logo = sprite_init(&state->sprite_renderer, &state->logo_tex);
    state->logo.size = (iv2){300, 150};

    state->logo_direction = (v2){1.0f, 1.0f};

    state->player_tex = texture_load_from_file("textures/white_pixel.png");
    state->player = sprite_init(&state->sprite_renderer, &state->player_tex);
    state->player.size = (iv2){50, 50};

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

    timer_init(&state->dash_cooldown, 2.0f, false);
    stopwatch_init(&state->stopwatch, false);

    cursor_set_from_system(CURSOR_ARROW);
    state->sword_cursor = cursor_load_from_file("cursors/sword.ani");

    // nuklear example
    state->alchemy_state = (nk_alchemy_state){0};
    state->clear_color = (v4){0.10f, 0.18f, 0.24f, 1.0f};
    state->alchemy_state.ctx = nk_alchemy_init(&state->alchemy_state, ui_shader);
    struct nk_font_atlas* atlas = &state->alchemy_state.atlas;
    nk_alchemy_font_stash_begin(&state->alchemy_state, &atlas);
    state->immortal = nk_font_atlas_add_from_file(atlas, "fonts/immortal.ttf", 14, 0);
    nk_alchemy_font_stash_end(&state->alchemy_state);
    nk_style_set_font(&state->alchemy_state.ctx, &state->immortal->handle);
    state->alchemy_state.keyboard = &state->input.keyboard;
    state->alchemy_state.mouse = &state->input.mouse;
}

void example_state_delete(ExampleState* state)
{
    font_renderer_delete(&state->font_renderer);
    font_renderer_delete(&state->frame_time_renderer);
    sprite_renderer_delete(&state->sprite_renderer);
    texture_delete(&state->logo_tex);
    texture_delete(&state->player_tex);
    nk_alchemy_shutdown(&state->alchemy_state);
}

void example_update_and_render(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height)
{
    stopwatch_update(&state->stopwatch, delta_time);
    
    Gamepad* gamepad = &state->input.gamepads[0];
    update_dvd(state, delta_time, window_width, window_height);
    update_player(state, delta_time, window_width, window_height);  

    if (key_pressed(&state->input.keyboard, KEY_LBRACKET))
        cursor_set_from_memory(state->sword_cursor);

    if (key_pressed(&state->input.keyboard, KEY_RBRACKET))
        cursor_set_from_system(CURSOR_ARROW);

    state->sound_output.should_play = false;
    if (gamepad_button_pressed(gamepad->a_button) && !state->is_shooting)
    {
        state->is_shooting = true;
        state->sound_output.should_play = true;
    }
    if (gamepad_button_released(gamepad->a_button))
        state->is_shooting = false;
    
    if (gamepad_button_released(gamepad->y_button))
    {
        if (state->stopwatch.is_active)
            stopwatch_stop(&state->stopwatch);
        else
            stopwatch_start(&state->stopwatch);
    }

    if (gamepad_button_released(gamepad->b_button))
        stopwatch_reset(&state->stopwatch);

    /* Draw */
    nk_alchemy_new_frame(&state->alchemy_state, window_width, window_height);
    struct nk_context* ctx = &state->alchemy_state.ctx;

    // TODO(lucas): Sizing window up looks wonky while dragging but fine after releasing mouse.
    renderer_viewport(0, 0, window_width, window_height);
    renderer_clear(state->clear_color);
    m4 projection = m4_ortho(0.0f, (f32)window_width, (f32)window_height, 0.0f, -1.0f, 1.0f); 
    shader_set_m4(state->font_renderer.shader, "projection", projection, 0);
    shader_set_m4(state->sprite_renderer.shader, "projection", projection, 0);

    sprite_draw(state->logo);
    sprite_draw(state->player);

    v4 font_color = {0.6f, 0.2f, 0.2f, 1.0f};
    v2 engine_text_pos = {500.0f, 50.0f};
    text_draw(&state->font_renderer, "Alchemy Engine", engine_text_pos, 48, font_color);
    char buffer[512];

    FontRenderer frame_time_renderer = {0};
    v2 ms_text_pos = {10.0f, window_height - 10.0f};
    sprintf_s(buffer, sizeof(buffer), "MS/frame: %.2f", delta_time * 1000.0f);
    text_draw(&state->frame_time_renderer, buffer, ms_text_pos, 32, font_color);

    char cooldown_buffer[512];
    v2 cooldown_text_pos = {1050.0f, window_height - 10.0f};
    sprintf_s(cooldown_buffer, sizeof(cooldown_buffer), "Cooldown: %.1f", timer_seconds(&state->dash_cooldown));
    if (state->dash_cooldown.is_active)
        text_draw(&state->frame_time_renderer, cooldown_buffer, cooldown_text_pos, 32, font_color);
    
    char stopwatch_buffer[512];
    v2 stopwatch_text_pos = {10.0f, 40.0f};
    sprintf_s(stopwatch_buffer, sizeof(stopwatch_buffer), "Stopwatch: %.1f", stopwatch_seconds(&state->stopwatch));
    text_draw(&state->frame_time_renderer, stopwatch_buffer, stopwatch_text_pos, 32, font_color);

    ui_overview(ctx, window_width);
    nk_alchemy_render(&state->alchemy_state, NK_ANTI_ALIASING_ON);
}
