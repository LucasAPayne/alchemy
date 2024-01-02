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
    example_state_init(&state, window);

    while(window.open)
    {
        input_process(&window, &state.input);

        f32 delta_time = get_frame_seconds(&window);
        example_update_and_render(&state, window, delta_time);

        sound_output_process(&state.sound_output, &state.transient_arena);

        window_render(&window);
    }

    example_state_delete(&state);
    return 0;
}
