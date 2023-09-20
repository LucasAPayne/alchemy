#include "example.h"

#include "input.h"
#include "window.h"
#include "util/types.h"

// TODO(lucas): Switch from WinMain() to main()
#include <windows.h>

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int show_cmd)
{
    Window window = window_init("Alchemy", 1280, 720, instance);
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
