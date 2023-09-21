#include "window.h"
#include "input.h"

int main(void)
{
    Window window = {0};
    window_init(&window, "Alchemy", 1280, 720);
    
    void* potion_icon = window_icon_load_from_file("icons/potion.ico");
    window_icon_set_from_memory(&window, potion_icon);

    Input input = {0};

    while(window.open)
    {
        // Double buffer input to detect buttons held
        Input old_input = input;
        input_process(&window, &input);

        window_render(&window);

        // The input of this frame becomes the old input for next frame
        old_input = input;
    }

    return 0;
}
