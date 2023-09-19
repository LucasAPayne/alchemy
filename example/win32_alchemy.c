#include "example.h"

#include "input.h"
#include "window.h"
#include "util/types.h"

#include "platform/windows/win32_sound.h"

// TODO(lucas): Switch from WinMain() to main()
#include <windows.h>

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int show_cmd)
{
    Window window = window_init("Alchemy", 1280, 720, instance);
    
    // TODO(lucas): The main application should not interact directly with XAudio
    XAudio2State xaudio2_state = xaudio2_state_init();

    ExampleState state = example_state_init();

    while(window.open)
    {
        // Double buffer input to detect buttons held
        Input old_input = state.input;
        keyboard_mouse_process_input(&window, &state.input.keyboard, &state.input.mouse);
        gamepad_process_input(&state.input);

        f32 delta_time = get_frame_seconds(&window);
        example_update_and_render(&state, delta_time, window.size.x, window.size.y);

        win32_process_sound_output(&state.sound_output, &xaudio2_state);

        window_render(&window);

        // The input of this frame becomes the old input for next frame
        old_input = state.input;
    }

    example_state_delete(&state);
    return 0;
}
