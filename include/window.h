#pragma once

#include "util/types.h"

typedef struct Window
{
    iv2 size;
    b32 open;

    // Timing information used to calculate delta seconds for each frame.
    // Not intended to be accessed
    i64 _ticks_per_second;
    i64 _prev_frame_ticks;

    void* ptr; // OS handle to window
} Window;

// TODO(lucas): Window styles
void window_init(Window* window, const char* title, int width, int height);
void window_render(Window* window);

// TODO(lucas): Set window attributes
void* window_icon_load_from_file(const char* filename);
void  window_icon_set_from_memory(Window* window, void* icon);

// Gets the number of seconds passed since the previous frame
/* IMPORTANT: Only call this function once per frame.
 * Otherwise, the incorrect time will be reported.
 */
f32 get_frame_seconds(Window* window);
