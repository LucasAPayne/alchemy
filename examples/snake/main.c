#include "window.h"
#include "input.h"
#include "renderer/renderer.h"

internal void draw_grid(Renderer* renderer, v2 start, v2 end, u32 slices, v4 color)
{
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
    window_init(&window, "Alchemy", 800, 800);
    
    void* potion_icon = window_icon_load_from_file("icons/potion.ico");
    window_icon_set_from_memory(&window, potion_icon);

    Input input = {0};
    Renderer renderer = renderer_init();

    Font font = font_load_from_file("fonts/cardinal.ttf");

    Text text = text_init("This is a string!", &font, (v2){250.0f, 250.f}, 48);
    text.color = (v4){1.0f, 0.0f, 0.0f, 1.0f};

    while(window.open)
    {
        // Double buffer input to detect buttons held
        Input old_input = input;
        input_process(&window, &input);

        renderer_viewport(&renderer, 0, 0, window.width, window.height);
        renderer_clear((v4){0.2f, 0.2f, 0.2f, 1.0f});

        draw_grid(&renderer, (v2){1.0f, 1.0f}, (v2){(f32)window.width, (f32)window.height}, 5, (v4){1.0f, 0.0f, 0.0f, 1.0f});

        window_render(&window);

        // The input of this frame becomes the old input for next frame
        old_input = input;
    }

    return 0;
}
