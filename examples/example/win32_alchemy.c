#include "example.h"

#include "input.h"
#include "window.h"
#include "util/types.h"

int main(void)
{
    Window window = {0};
    window_init(&window, "Alchemy", 1280, 720);
    
    void* potion_icon = window_icon_load_from_file("icons/potion.ico");
    window_icon_set_from_memory(&window, potion_icon);

    ExampleState state = {0};
    example_state_init(&state);

    while(window.open)
    {
        // Double buffer input to detect buttons held
        Input old_input = state.input;
        input_process(&window, &state.input);

        f32 delta_time = get_frame_seconds(&window);
        example_update_and_render(&state, delta_time, window.size.x, window.size.y);

        sound_output_process(&state.sound_output);

        window_render(&window);

        // The input of this frame becomes the old input for next frame
        old_input = state.input;
    }

    example_state_delete(&state);
    return 0;
}
