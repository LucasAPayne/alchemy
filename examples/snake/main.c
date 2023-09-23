#include "window.h"
#include "input.h"
#include "renderer/renderer.h"

int main(void)
{
    Window window = {0};
    window_init(&window, "Alchemy", 1280, 720);
    
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

        draw_rect(&renderer, (v2){100.0f, 100.0f}, (v2){50.0f, 50.0f}, (v4){1.0f, 0.0f, 0.0f, 1.0f}, 0.0f);
        draw_text(&renderer, text);

        window_render(&window);

        // The input of this frame becomes the old input for next frame
        old_input = input;
    }

    return 0;
}
