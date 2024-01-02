#include "window.h"
#include "input.h"
#include "renderer/renderer.h"

internal void draw_grid(Renderer* renderer, v2 start, v2 end, u32 slices, v4 color, f32 line_thickness)
{
    // NOTE(lucas): This function assumes a square viewport
    f32 cell_size = (end.x - start.x) / (f32)slices;

    f32 x = start.x;
    f32 y = start.y;

    for (u32 slice = 0; slice < slices; ++slice)
    {
        draw_line(renderer, (v2){start.x, y}, (v2){end.x, y}, color, line_thickness);
        draw_line(renderer, (v2){x, start.y}, (v2){x, end.y}, color, line_thickness);
        x += cell_size;
        y += cell_size;
    }

    // Subtract thickness before drawing final lines so they appear properly at the viewport edges
    x -= line_thickness;
    y -= line_thickness;
    draw_line(renderer, (v2){start.x, y}, (v2){end.x, y}, color, line_thickness);
    draw_line(renderer, (v2){x, start.y}, (v2){x, end.y}, color, line_thickness);
}

int main(void)
{
    Window window = {0};
    int initial_window_width = 800;
    int initial_window_height = 800;

    window_init(&window, "Snake", initial_window_width, initial_window_height);
    window_set_min_size(&window, initial_window_width, initial_window_height);
    
    void* potion_icon = window_icon_load_from_file("icons/potion.ico");
    window_icon_set_from_memory(&window, potion_icon);

    Input input = {0};
    Renderer renderer = renderer_init(initial_window_width, initial_window_height, MEGABYTES(4));
    renderer.clear_color = (v4){0.1f, 0.1f, 0.1f, 1.0f};

    while(window.open)
    {
        // Double buffer input to detect buttons held
        Input old_input = input;
        input_process(&window, &input);

        if (key_pressed(&input.keyboard, KEY_MENU) && key_released(&input.keyboard, KEY_Z))
            renderer.config.wireframe_mode = !renderer.config.wireframe_mode;

        // NOTE(lucas): Fix viewport at 800x800 for now
        rect viewport = rect_min_dim(v2_zero(), v2_full((f32)initial_window_width));
        viewport.x = ((f32)window.width - viewport.width) / 2.0f;
        viewport.y = ((f32)window.height - viewport.width) / 2.0f;
        renderer_viewport(&renderer, viewport);
        renderer_new_frame(&renderer, window);

        draw_grid(&renderer, v2_one(), (v2){(f32)viewport.width, (f32)viewport.height}, 5, color_red(), 1.0f);
        draw_circle(&renderer, v2_full(400.0f), 80.0f, color_red());

        renderer_render(&renderer);
        window_render(&window);

        // The input of this frame becomes the old input for next frame
        old_input = input;
    }

    return 0;
}
