#include "example.h"
#include "renderer/renderer.h"
#include "util/alchemy_math.h"
#include "util/types.h"

#include "ui_overview.h"

#include <stdlib.h> // rand
#include <stdio.h>  // Temporary: sprintf_s
#include <string.h> // Temporary

internal void bounce_dvd(ExampleState* state, f32* direction)
{
    // Bounce off screen boundary
    *direction *= -1.0f;

    int color_index = rand() % 7;
    // Make sure new color is different. Incredibly efficient
    while (color_index == state->last_color_index)
        color_index = rand() % 7;
    state->logo.color = state->colors[color_index];
    state->last_color_index = color_index;
}

internal void update_dvd(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height)
{
    f32 speed = 100.0f; // pixels per second
    rect window_bounds = rect_min_max((v2){0.0f, 0.0f},
                                      (v2){(f32)(window_width - state->logo.size.x),
                                           (f32)(window_height - state->logo.size.y)});

    state->logo.position = v2_add(state->logo.position, v2_scale(state->logo_direction, speed*delta_time));

    v2 min = window_bounds.position;
    v2 max = v2_add(min, window_bounds.size);
    if ((state->logo.position.x > max.x) || (state->logo.position.x < min.x))
        bounce_dvd(state, &state->logo_direction.x);
    if ((state->logo.position.y > max.y) || (state->logo.position.y < min.y))
        bounce_dvd(state, &state->logo_direction.y);

    state->logo.position = v2_clamp_to_rect(state->logo.position, window_bounds);
}

internal void update_player(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height)
{
    Player* player = &state->player;

    timer_update(&player->dash_cooldown, delta_time, true);

    Gamepad* gamepad = &state->input.gamepads[0];
    f32 speed = 250.0f; // pixels per second

    rect window_bounds = rect_min_max((v2){0.0f, 0.0f},
                                      (v2){(f32)(window_width - player->size.x),
                                           (f32)(window_height - player->size.y)});

    // Update player position
    v2 player_delta = v2_scale((v2){gamepad->left_stick_x, gamepad->left_stick_y}, speed*delta_time);
    player->position = v2_add(player->position, player_delta);

    // Dash
    if (!player->dash_cooldown.is_active)
    {
        if (gamepad_button_released(gamepad->left_shoulder) && player->dash_counter == 0)
        {
            player->dash_counter = player->dash_frames;
            player->dash_direction = -1.0f;
            timer_start(&player->dash_cooldown);
        }
        if (gamepad_button_released(gamepad->right_shoulder) && player->dash_counter == 0)
        {
            player->dash_counter = player->dash_frames;
            player->dash_direction = 1.0f;
            timer_start(&player->dash_cooldown);
        }
    }
    if (player->dash_counter > 0)
    {
        f32 dash_delta = player->dash_distance / (f32)(player->dash_frames);
        player->position.x += dash_delta * player->dash_direction;
        player->dash_counter--;
    }

    // Bounds checking
    player->position = v2_clamp_to_rect(player->position, window_bounds);

    // Update player rotation
    player->rotation -= 2.0f * gamepad->right_trigger_val;
    player->rotation += 2.0f * gamepad->left_trigger_val;
    player->rotation = clamp_f32(player->rotation, -45.0f, 45.0f);
    if (!gamepad_button_pressed(gamepad->left_trigger) &&
        !gamepad_button_pressed(gamepad->right_trigger))
    {
        if (player->rotation > 0.0f)
            player->rotation -= 2.0f;
        if (player->rotation < 0.0f)
            player->rotation += 2.0f;
        if (fabs(player->rotation) < 2.0f)
            player->rotation = 0.0f;
    }

    // Vibration test
    if (gamepad_button_pressed(gamepad->x_button))
        gamepad_set_vibration(gamepad, 16000, 16000);
}

void example_state_init(ExampleState* state, Window window)
{
    srand(0);

    // Compile and Load shaders
    u32 font_shader = shader_init("shaders/font.vert", "shaders/font.frag");
    u32 ui_shader = shader_init("shaders/ui.vert", "shaders/ui.frag");

    state->renderer = renderer_init(window.width, window.height);
    state->renderer.clear_color = (v4){0.10f, 0.18f, 0.24f, 1.0f};

    state->cardinal_font = font_load_from_file("fonts/cardinal.ttf");
    state->immortal_font = font_load_from_file("fonts/immortal.ttf");
    state->matrix_font = font_load_from_file("fonts/matrix_regular.ttf");

    state->logo_tex = texture_load_from_file("textures/dvd.png");
    state->logo = sprite_init(&state->logo_tex);
    state->logo.position = (v2){0.0f, (f32)window.height};
    state->logo.size = (v2){300.0f, 150.0f};

    state->logo_direction = (v2){-1.0f, -1.0f};

    state->colors[0] = color_white();
    state->colors[1] = color_red();
    state->colors[2] = color_green();
    state->colors[3] = color_blue();
    state->colors[4] = color_yellow();
    state->colors[5] = color_magenta();
    state->colors[6] = color_cyan();

    state->player.size = (v2){50.0f, 50.0f};
    state->player.color = color_white();

    state->player.dash_counter = 0;
    state->player.dash_frames = 15;
    state->player.dash_direction = 0.0f;
    state->player.dash_distance = 300.0f;

    const char* test_sound_filename = "sounds/pew.wav";
    strncpy_s(state->sound_output.filename, sizeof(state->sound_output.filename), test_sound_filename,
              strlen(test_sound_filename));
    sound_output_set_volume(&state->sound_output, 0.5f);
    state->sound_output.should_play = false;
    state->is_shooting = false;

    timer_init(&state->player.dash_cooldown, 2.0f, false);
    stopwatch_init(&state->stopwatch, false);

    cursor_set_from_system(CURSOR_ARROW);
    state->sword_cursor = cursor_load_from_file("cursors/sword.ani");

    state->transient_arena = memory_arena_alloc(MEGABYTES(1));

    // nuklear example
    state->alchemy_state = (nk_alchemy_state){0};
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
    renderer_delete(&state->renderer);
    texture_delete(&state->logo_tex);
    nk_alchemy_shutdown(&state->alchemy_state);
}

void example_update_and_render(ExampleState* state, Window window, f32 delta_time)
{
    memory_arena_clear(&state->transient_arena);

    stopwatch_update(&state->stopwatch, delta_time);
    
    Gamepad* gamepad = &state->input.gamepads[0];
    // update_dvd(state, delta_time, window.width, window.height);
    // update_player(state, delta_time, window.width, window.height);  

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
    nk_alchemy_new_frame(&state->alchemy_state, window.width, window.height);
    struct nk_context* ctx = &state->alchemy_state.ctx;

    // TODO(lucas): Sizing window up looks wonky while dragging but fine after releasing mouse.
    renderer_new_frame(&state->renderer, window);

    draw_sprite(&state->renderer, state->logo);

    Player* player = &state->player;
    draw_quad(&state->renderer, player->position, player->size, player->color, player->rotation);

    v4 font_color = {0.6f, 0.2f, 0.2f, 1.0f};
    Text engine_text = text_init("Alchemy Engine", &state->cardinal_font, (v2){500.0f, window.height - 50.0f}, 48);
    engine_text.color = font_color;
    draw_text(&state->renderer, engine_text);
    
    char buffer[512];
    sprintf_s(buffer, sizeof(buffer), "MS/frame: %.2f", delta_time * 1000.0f);
    Text frame_time = text_init(buffer, &state->immortal_font, (v2){10.0f, 10.0f}, 32);
    frame_time.color = font_color;
    draw_text(&state->renderer, frame_time);

    char cooldown_buffer[512];
    sprintf_s(cooldown_buffer, sizeof(cooldown_buffer), "Cooldown: %.1f", timer_seconds(&player->dash_cooldown));
    Text cooldown_text = text_init(cooldown_buffer, &state->immortal_font, (v2){1050.0f, 10.0f}, 32);
    cooldown_text.color = font_color;
    if (player->dash_cooldown.is_active)
        draw_text(&state->renderer, cooldown_text);
    
    char stopwatch_buffer[512];
    sprintf_s(stopwatch_buffer, sizeof(stopwatch_buffer), "Stopwatch: %.1f", stopwatch_seconds(&state->stopwatch));
    Text stopwatch_text = text_init(stopwatch_buffer, &state->immortal_font, (v2){10.0f, window.height - 30.0f}, 32);
    stopwatch_text.color = font_color;
    draw_text(&state->renderer, stopwatch_text);

    /* Text justification Test */
    rect text_bounds = rect_min_dim((v2){350.0f, 100.0f}, v2_full(300.0f));
    draw_quad(&state->renderer, text_bounds.position, text_bounds.size, color_white(), 0.0f);

    u32 text_size = 24;
    v2 text_begin = {text_bounds.position.x, text_bounds.position.y + text_bounds.height - (f32)text_size};
    char* str = "If you have \"Right Leg of the Forbidden One\", \"Left Leg of the Forbidden One\", \"Right Arm of the "
                "Forbidden One\" and \"Left Arm of the Forbidden One\" in addition to this card in your hand, you win "
                "the Duel.";

    Text text = text_init(str, &state->matrix_font, text_begin, text_size);
    text.color = color_black();

    TextArea text_area = text_area_init(text_bounds, text);
    text_area.alignment = TEXT_ALIGN_JUSTIFIED;
    text_area.style |= TEXT_AREA_WRAP|TEXT_AREA_SHRINK_TO_FIT;
    draw_text_area(&state->renderer, text_area, &state->transient_arena);

    ui_overview(ctx, window.width);
    nk_alchemy_render(&state->alchemy_state, NK_ANTI_ALIASING_ON);
    renderer_render(&state->renderer);
}
