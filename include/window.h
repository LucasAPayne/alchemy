#pragma once

#include "util/types.h"

// TEMPORARY: HINSTANCE
#include <windows.h>

typedef struct Window
{
    iv2 size;
    b32 open;
    i64 _ticks_per_second;
    i64 _prev_frame_ticks;
    void* ptr;
} Window;

Window window_init(const char* title, int width, int height, HINSTANCE instance);
void window_render(Window* window);

// Gets the number of seconds passed since the previous frame
/* IMPORTANT: Only call this function once per frame.
 * Otherwise, the incorrect time will be reported.
 */
f32 get_frame_seconds(Window* window);
