#include "example.h"

#include "alchemy/input.h"
#include "alchemy/window.h"
#include "alchemy/state.h"
#include "alchemy/util/log.h"
#include "alchemy/util/types.h"

int main(void)
{
    console_launch();

    Window* window = window_create("Alchemy", 1280, 720);

    void* potion_icon = window_icon_load_from_file("icons/potion.ico");
    window_icon_set_from_memory(window, potion_icon);

    GameMemory memory = game_memory_init(MEGABYTES(4), MEGABYTES(4));
    Renderer renderer = renderer_init(window, window->width, window->height, MEGABYTES(4));
    renderer.clear_color = (v4){0.10f, 0.18f, 0.24f, 1.0f};
    Input input = {0};

    GameCode game = game_code_load("example_dll.dll", "example_dll_temp.dll", "example_dll_lock.tmp");
    input_loop_init(&game, &memory);

    while(window->open)
    {
        game_code_update(&game);
        input_process(window, &input);
        input_loop_update(&game, &memory, &input);

        // TODO(lucas): Sizing window up looks wonky while dragging but fine after releasing mouse.
        renderer_new_frame(&renderer, window);
        f32 delta_time = get_frame_seconds(window);
        if (game.update_and_render)
            game.update_and_render(&memory, &input, &renderer, window, delta_time);

        renderer_render(&renderer);
        window_render(window);
    }

    renderer_delete(&renderer);
    return 0;
}
