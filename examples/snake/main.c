#include "window.h"
#include "input.h"
#include "renderer/renderer.h"

internal void draw_grid(Renderer* renderer, v2 start, v2 end, u32 slices, v4 color)
{
    // NOTE(lucas): This function assumes a square viewport
    f32 cell_size = (end.x - start.x) / (f32)slices;

    f32 x = start.x;
    f32 y = start.y;

    for (u32 slice = 0; slice <= slices; ++slice)
    {
        draw_line(renderer, (v2){start.x, y}, (v2){end.x, y}, color);
        draw_line(renderer, (v2){x, start.y}, (v2){x, end.y}, color);
        x += cell_size;
        y += cell_size;
    }
}

int main(void)
{
    Window window = {0};
    // TODO(lucas): Allow 16:9 aspect ratio, but confine grid/play area to a square aspect ratio
    int initial_window_width = 800;
    int initial_window_height = 800;
    window_init(&window, "Alchemy", initial_window_width, initial_window_height);
    
    void* potion_icon = window_icon_load_from_file("icons/potion.ico");
    window_icon_set_from_memory(&window, potion_icon);

    Input input = {0};
    Renderer renderer = renderer_init(initial_window_width, initial_window_height);
    renderer.clear_color = (v4){0.1f, 0.1f, 0.1f, 1.0f};

    while(window.open)
    {
        // Double buffer input to detect buttons held
        Input old_input = input;
        input_process(&window, &input);

        renderer_new_frame(&renderer, window);

        draw_grid(&renderer, v2_one(), (v2){(f32)window.width, (f32)window.height}, 5, (v4){1.0f, 0.0f, 0.0f, 1.0f});
        draw_circle(&renderer, (v2){400.0f, 400.0f}, 80.0f, (v4){1.0f, 0.0f, 0.0f, 1.0f});
        // draw_rect(&renderer, v2_zero(), (v2){(f32)window.width, (f32)window.height}, (v4){0.3f, 0.0f, 0.0f, 1.0f}, 0.0f);

        renderer_render(&renderer);
        window_render(&window);

        // The input of this frame becomes the old input for next frame
        old_input = input;
    }

    return 0;
}
