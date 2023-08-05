#include "example.h"
#include "renderer/font.h"
#include "renderer/shader.h"
#include "renderer/sprite.h"
#include "renderer/texture.h"
#include "util/types.h"

#include <glad/glad.h>

#include <stdlib.h> // rand
#include <stdio.h>  // sprintf_s
#include <string.h> // Temporary

#define INCLUDE_OVERVIEW
#ifdef INCLUDE_OVERVIEW
  #include "nuklear_overview.c"
#endif

static void error_callback(int e, const char *d)
{printf("Error %d: %s\n", e, d);}

global_variable vec3s colors[7] = {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
                          {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f},
                          {0.0f, 1.0f, 1.0f}};

// Calculate width of a string
// f32 calculate_text_width(nk_handle handle, f32 height, const char* text, int len)
// {
//     FontRenderer* font = handle.ptr;
//     f32 text_width = 0.0f;

//     const char *c;
//     for (c = text; *c; ++c)
//     {
//         if (!FT_Load_Char(font->face, *c, FT_LOAD_RENDER))
//         {
//             // TODO(lucas): Diagnostic, could not load character
//         }

//         text_width += (f32)font->face->glyph->bitmap.width;
//     }

//     return text_width;
// }

// void query_font_glyph(nk_handle handle, f32 font_height, nk_user_font_glyph* glyph, nk_rune codepoint, nk_rune next_codepoint)
// {
//     FontRenderer* font = handle.ptr;
//     glyph->width = (f32)font->face->glyph->bitmap.width;
//     glyph->height = (f32)font->face->glyph->bitmap.rows;
//     glyph->xadvance = (f32)font->face->glyph->advance.x/64;
//     glyph->uv[0].x = 0.0f;
//     glyph->uv[0].y = 0.0f;
//     glyph->uv[1].x = 1.0f;
//     glyph->uv[1].y = 1.0f;
//     // glyph->offset.x = font->face->glyph->bitmap_left;
//     // glyph->offset.y = font->face->glyph->bitmap_top;
// }

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
        state->logo.color = colors[color_index];
        state->last_color_index = color_index;
    }
    if (state->logo.position.y > (window_height - state->logo.size.y) || state->logo.position.y < 0)
    {
        state->logo_y_direction *= -1.0f;

        int color_index = rand() % 7;
        // Make sure new color is different. Incredibly efficient
        while (color_index == state->last_color_index)
            color_index = rand() % 7;
        state->logo.color = colors[color_index];
        state->last_color_index = color_index;
    }
}

internal void update_player(ExampleState* state, f32 delta_time, u32 window_width, u32 window_height)
{
    timer_update(&state->dash_cooldown, delta_time, true);

    Gamepad* gamepad = &state->input.gamepads[0];
    f32 speed = 250.0f; // pixels per second
    // Update player position
    state->player.position.x += speed * delta_time * gamepad->left_stick_x;
    state->player.position.y += speed * delta_time * gamepad->left_stick_y;

    // Dash
    if (!state->dash_cooldown.is_active)
    {
        if (is_gamepad_button_released(gamepad->left_shoulder) && state->dash_counter == 0)
        {
            state->dash_counter = state->dash_frames;
            state->dash_direction = -1.0f;
            timer_start(&state->dash_cooldown);
        }
        if (is_gamepad_button_released(gamepad->right_shoulder) && state->dash_counter == 0)
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
    *state = (ExampleState){0};

    // Compile and Load shaders
    u32 sprite_shader = shader_init("shaders/sprite.vert", "shaders/sprite.frag");
    u32 font_shader = shader_init("shaders/font.vert", "shaders/font.frag");

    init_sprite_renderer(&state->sprite_renderer, sprite_shader);
    init_font_renderer(&state->font_renderer, font_shader, "fonts/cardinal.ttf");
    init_font_renderer(&state->frame_time_renderer, font_shader, "fonts/immortal.ttf");

    u32 logo_tex = generate_texture_from_file("textures/dvd.png");
    state->logo.renderer = &state->sprite_renderer;
    state->logo.position = (vec2s){0.0f, 0.0f};
    state->logo.texture = logo_tex;
    state->logo.color = colors[0];
    state->logo.size = (vec2s){300.0f, 150.0f};
    state->logo.rotation = 0.0f;
    state->logo_x_direction = 1.0f;
    state->logo_y_direction = 1.0f;
    state->clear_color = (vec3s){0.2f, 0.2f, 0.2f};

    u32 player_tex = generate_texture_from_file("textures/white_pixel.png");
    state->player.renderer = &state->sprite_renderer;
    state->player.position = (vec2s){0.0f, 0.0f};
    state->player.texture = player_tex;
    state->player.color = (vec3s){1.0f, 1.0f, 1.0f};
    state->player.size = (vec2s){50.0f, 50.0f};
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

    timer_init(&state->dash_cooldown, 2.0f, false);
    stopwatch_init(&state->stopwatch, false);

    state->alchemy_state = (nk_alchemy_state){0};
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
    stopwatch_update(&state->stopwatch, delta_time);
    
    Gamepad* gamepad = &state->input.gamepads[0];
    update_dvd(state, delta_time, window_width, window_height);
    update_player(state, delta_time, window_width, window_height);  
    
    // TODO(lucas): Sizing window up looks wonky while dragging but fine after releasing mouse.
    glViewport(0, 0, window_width, window_height);
    glClearColor(state->clear_color.r, state->clear_color.g, state->clear_color.b, 1.0f);

    // if (is_key_released(&state->keyboard, Key::A))
    if (is_mouse_button_pressed(&state->input.mouse, MOUSE_X2))
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    
    state->sound_output.should_play = false;
    if (is_gamepad_button_pressed(gamepad->a_button) && !state->is_shooting)
    {
        state->is_shooting = true;
        state->sound_output.should_play = true;
    }
    if (is_gamepad_button_released(gamepad->a_button))
        state->is_shooting = false;
    
    if (is_gamepad_button_released(gamepad->y_button))
    {
        if (state->stopwatch.is_active)
            stopwatch_stop(&state->stopwatch);
        else
            stopwatch_start(&state->stopwatch);
    }

    if (is_gamepad_button_released(gamepad->b_button))
        stopwatch_reset(&state->stopwatch);
    
    glClear(GL_COLOR_BUFFER_BIT);

    mat4s projection = glms_ortho(0.0f, (f32)window_width, (f32)window_height, 0.0f, -1.0f, 1.0f); 
    shader_set_mat4f(state->font_renderer.shader, "projection", projection, 0);
    shader_set_mat4f(state->sprite_renderer.shader, "projection", projection, 0);

    draw_sprite(state->logo);
    draw_sprite(state->player);

    vec4s font_color = {0.6f, 0.2f, 0.2f, 1.0f};
    vec2s engine_text_pos = {500.0f, 50.0f};
    render_text(&state->font_renderer, "Alchemy Engine", engine_text_pos, 48, font_color);
    char buffer[512];

    FontRenderer frame_time_renderer = {0};
    vec2s ms_text_pos = {10.0f, window_height - 10.0f};
    sprintf_s(buffer, sizeof(buffer), "MS/frame: %.2f", delta_time * 1000.0f);
    render_text(&state->frame_time_renderer, buffer, ms_text_pos, 32, font_color);

    char cooldown_buffer[512];
    vec2s cooldown_text_pos = {1050.0f, window_height - 10.0f};
    sprintf_s(cooldown_buffer, sizeof(cooldown_buffer), "Cooldown: %.1f", timer_seconds(&state->dash_cooldown));
    if (state->dash_cooldown.is_active)
        render_text(&state->frame_time_renderer, cooldown_buffer, cooldown_text_pos, 32, font_color);
    
    char stopwatch_buffer[512];
    vec2s stopwatch_text_pos = {10.0f, 40.0f};
    sprintf_s(stopwatch_buffer, sizeof(stopwatch_buffer), "Stopwatch: %.1f", stopwatch_seconds(&state->stopwatch));
    render_text(&state->frame_time_renderer, stopwatch_buffer, stopwatch_text_pos, 32, font_color);
}
