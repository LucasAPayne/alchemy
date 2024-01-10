#include "example.h"

#include "input.h"
#include "window.h"
#include "state.h"
#include "util/types.h"

int main(void)
{
    Window window = {0};
    window_init(&window, "Alchemy", 1280, 720);
    
    void* potion_icon = window_icon_load_from_file("icons/potion.ico");
    window_icon_set_from_memory(&window, potion_icon);

    // ExampleState state = {0};
    // example_state_init(&state, window);

    GameMemory memory = game_memory_init(MEGABYTES(4), MEGABYTES(4));
    Renderer renderer = renderer_init(window, window.width, window.height, MEGABYTES(4));
    
    GameCode game = game_code_load("example_dll.dll", "example_dll_temp.dll", "example_dll_lock.tmp");
    while(window.open)
    {
        // TODO(lucas): Sizing window up looks wonky while dragging but fine after releasing mouse.
        // TODO(lucas): Combine these calls?
        // nk_alchemy_new_frame(&state.alchemy_state, window.width, window.height);
        renderer_new_frame(&renderer, window);
        f32 delta_time = get_frame_seconds(&window);
        if (game.update_and_render)
            game.update_and_render(&memory, &renderer, window, delta_time);

        // TODO(lucas): Combine these calls?
        // nk_alchemy_render(&state.alchemy_state, NK_ANTI_ALIASING_ON);
        renderer_render(&renderer);
        window_render(&window);
    }

    renderer_delete(&renderer);
    // nk_alchemy_shutdown(&state->alchemy_state);
    return 0;
}
