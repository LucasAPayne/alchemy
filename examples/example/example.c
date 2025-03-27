#include "example.h"
#include "alchemy/renderer/renderer.h"
#include "alchemy/state.h"
#include "alchemy/util/math.h"
#include "alchemy/util/types.h"

#include "ui_overview.h"

#include <stdlib.h> // rand

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

internal void update_player(ExampleState* state, Input* input, f32 delta_time, u32 window_width, u32 window_height)
{
    Player* player = &state->player;

    timer_update(&player->dash_cooldown, delta_time, true);

    Gamepad* gamepad = &input->gamepads[0];
    f32 speed = 250.0f; // pixels per second

    rect window_bounds = rect_min_max((v2){0.0f, 0.0f},
                                      (v2){(f32)(window_width - player->size.x),
                                           (f32)(window_height - player->size.y)});

    // Update player position
    v2 player_delta = v2_scale((v2){gamepad->left_stick_x, -gamepad->left_stick_y}, speed*delta_time);
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

internal void example_state_init(ExampleState* state, GameMemory* memory, Input* input, Renderer* renderer, Window* window)
{
    srand(0);

    state->transient_arena = memory_arena_init_from_base(memory->transient_storage, memory->transient_storage_bytes);
    state->permanent_arena = memory_arena_init_from_base(memory->permanent_storage, memory->permanent_storage_bytes);

    state->cardinal_font = font_load_from_file("fonts/cardinal.ttf");
    state->immortal_font = font_load_from_file("fonts/immortal.ttf");
    state->matrix_font = font_load_from_file("fonts/matrix_book.ttf");

    state->logo_tex = texture_load_from_file(renderer, "textures/dvd.png");
    state->logo = sprite_init(&state->logo_tex);
    state->logo.size = (v2){300.0f, 150.0f};
    state->logo.position = v2_zero();
    state->logo_direction = (v2){1.0f, -1.0f};

    state->colors[0] = color_white();
    state->colors[1] = color_red();
    state->colors[2] = color_green();
    state->colors[3] = color_blue();
    state->colors[4] = color_yellow();
    state->colors[5] = color_magenta();
    state->colors[6] = color_cyan();

    state->player.size = (v2){50.0f, 50.0f};
    state->player.position = (v2){0.0f, (f32)window->height + state->player.size.y};
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

    // nuklear example
    ui_state_init(renderer, state->matrix_font, 14, &state->permanent_arena);
    UIState* ui_state = &renderer->ui_state;
    ui_state->keyboard = &input->keyboard;
    ui_state->mouse = &input->mouse;
}

UPDATE_AND_RENDER(update_and_render)
{
    ExampleState* state = (ExampleState*)memory->permanent_storage;
    if (!memory->is_initialized)
    {
        example_state_init(state, memory, input, renderer, window);
        memory->is_initialized = true;
    }

    memory_arena_clear(&state->transient_arena);

    stopwatch_update(&state->stopwatch, delta_time);
    Gamepad* gamepad = &input->gamepads[0];
    Keyboard* keyboard = &input->keyboard;
    update_dvd(state, delta_time, window->width, window->height);
    update_player(state, input, delta_time, window->width, window->height);

    if (key_pressed(keyboard, KEY_LBRACKET))
        cursor_set_from_memory(state->sword_cursor);

    if (key_pressed(keyboard, KEY_RBRACKET))
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
    Player* player = &state->player;
    draw_quad(renderer, player->position, player->size, player->color, player->rotation);
    // draw_quad_outline(renderer, player->position, player->size, color_red(), player->rotation, 5.0f);
    // draw_quad_gradient(renderer, player->position, player->size, color_black(), color_black(), color_red(), color_red(),
                    //    player->rotation);

    // v2 a = player->position;
    // v2 b = v2_add(player->position, (v2){200.0f, 50.0f});
    // v2 c = v2_add(player->position, (v2){-100.0f, -100.0f});
    // draw_triangle(renderer, a, b, c, player->color, player->rotation);
    // draw_triangle_outline(renderer, a, b, c, color_red(), player->rotation, 5.0f);
    // draw_triangle_gradient(renderer, a, b, c, color_red(), color_green(), color_blue(), player->rotation);

    // draw_line(renderer, player->position, v2_add(player->position, player->size), player->color, 5.0f, player->rotation);

    // renderer->config.wireframe_mode = true;

    // v2 center = v2_add(player->position, (v2){300.0f, -300.0f});
    // draw_circle_outline(renderer, center, player->size.x, player->color, 5.0f);

    // f32 in_rad = player->size.x*4.0f;
    // f32 out_rad = player->size.x*2.0f;
    // persist f32 time = 0.0f;
    // f32 freq = 2.0f;
    // f32 start_angle = 45.0f - 22.5f*(1 + sin_f32(2.0f*(f32)GLM_PI*freq*time));
    // f32 end_angle = 315.0f + 22.5f*(1 + sin_f32(2.0f*(f32)GLM_PI*freq*time));
    // time += delta_time;
    // draw_circle_sector(renderer, center, out_rad, start_angle, end_angle, player->color, 0.0f);
    // draw_ring(renderer, center, out_rad, in_rad, -90.0f, 180.0f, player->color, player->rotation);
    // draw_ring_outline(renderer, center, out_rad, in_rad, -90.0f, 180.0f, player->color, player->rotation, 5.0f);

    draw_sprite(renderer, state->logo);

    v4 font_color = {0.6f, 0.2f, 0.2f, 1.0f};
    Text engine_text = text_init(s8("Alchemy Engine"), &state->cardinal_font, (v2){500.0f, 50.0f}, 48);
    engine_text.color = font_color;
    draw_text(renderer, engine_text);

    s8 ms_frame = s8_format(&state->transient_arena, "MS/frame: %.2f", delta_time*1000.0f);
    Text frame_time = text_init(ms_frame, &state->immortal_font, (v2){10.0f, (f32)window->height - 10.0f}, 32);
    frame_time.color = font_color;
    draw_text(renderer, frame_time);

    s8 cooldown = s8_format(&state->transient_arena, "Cooldown: %.1f", timer_seconds(&player->dash_cooldown));
    Text cooldown_text = text_init(cooldown, &state->immortal_font, (v2){1100.0f, (f32)window->height - 10.0f}, 32);
    cooldown_text.color = font_color;
    if (player->dash_cooldown.is_active)
        draw_text(renderer, cooldown_text);
    
    s8 stopwatch = s8_format(&state->transient_arena, "Stopwatch: %.1f", stopwatch_seconds(&state->stopwatch));
    Text stopwatch_text = text_init(stopwatch, &state->immortal_font, (v2){10.0f, 30.0f}, 32);
    stopwatch_text.color = font_color;
    draw_text(renderer, stopwatch_text);

    /* Text justification Test */
    rect text_bounds = rect_min_dim((v2){300.0f, 300.0f}, (v2){250.0f, 100.0f});
    draw_quad(renderer, text_bounds.position, text_bounds.size, color_white(), 0.0f);

    s8 str = s8("●If you have \"Right Leg of the Forbidden One\", \"Left Leg of the Forbidden One\", \"Right Arm of the "
                "Forbidden One\" and \"Left Arm of the Forbidden One\" in addition to this card in your hand, you win "
                "the Duel.");

    TextArea text_area = text_area_init(renderer, text_bounds, str, &state->matrix_font, 18);
    text_area.text.color = color_black();
    text_area.horiz_alignment = TEXT_ALIGN_HORIZ_JUSTIFIED;
    text_area.vert_alignment = TEXT_ALIGN_VERT_TOP;
    text_area.style |= TEXT_AREA_WRAP|TEXT_AREA_SHRINK_TO_FIT;
    draw_text_area(renderer, &text_area);
    
    Texture* logo_tex = push_struct(&state->transient_arena, Texture);
    *logo_tex = state->logo_tex;
    struct nk_context* ctx = &renderer->ui_state.ctx;
    ui_overview(renderer, ctx, window->width, logo_tex);

    sound_output_process(&state->sound_output, &state->transient_arena);
}
