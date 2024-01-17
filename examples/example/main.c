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

    GameMemory memory = game_memory_init(MEGABYTES(4), MEGABYTES(4));
    Renderer renderer = renderer_init(window, window.width, window.height, MEGABYTES(4));
    Input input = {0};
    
    // TODO(lucas): For now, fonts for nuklear must be specified here because the font setup contains
    // an OpenGL call, which cannot be called from inside the game DLL.
    // Use a different font API in the future.
    struct nk_font* immortal;
    struct nk_font_atlas* atlas = &renderer.ui_render_state.atlas;
    ui_font_stash_begin(&renderer, &atlas);
    immortal = nk_font_atlas_add_from_file(atlas, "fonts/immortal.ttf", 14, 0);
    ui_font_stash_end(&renderer);
    nk_style_set_font(&renderer.ui_render_state.ctx, &immortal->handle);

    GameCode game = game_code_load("example_dll.dll", "example_dll_temp.dll", "example_dll_lock.tmp");
    input_loop_init(&game, &memory);

    while(window.open)
    {
        game_code_update(&game);

        input_process(&window, &input);
        input_loop_update(&game, &memory, &input);

        // TODO(lucas): Sizing window up looks wonky while dragging but fine after releasing mouse.
        renderer_new_frame(&renderer, window);
        f32 delta_time = get_frame_seconds(&window);
        if (game.update_and_render)
            game.update_and_render(&memory, &input, &renderer, window, delta_time);

        renderer_render(&renderer);
        window_render(&window);
    }

    renderer_delete(&renderer);
    return 0;
}
