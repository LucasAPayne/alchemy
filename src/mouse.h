#pragma once

#include "types.h"

struct MouseButtonState
{
    bool is_pressed;
    bool is_released;
};

/* IMPORTANT(lucas): This enum is used for array indexing.
* The last value NUM_KEYS is used for declaring arrays of the proper size
* Careful when changing this enum!
*/ 
enum MouseButton
{
    UNKONWN = 0,
    MOUSE_LEFT,
    MOUSE_MIDDLE,
    MOUSE_RIGHT,
    MOUSE_X1,
    MOUSE_X2,
    NUM_BUTTONS
};

struct MouseInput
{
    i32 x;
    i32 y;
    MouseButtonState buttons[MouseButton::NUM_BUTTONS];
};

inline bool is_mouse_button_pressed(MouseInput* mouse, int button)
{
    return mouse->buttons[button].is_pressed;
}

inline bool is_mouse_button_released(MouseInput* mouse, int button)
{
    return mouse->buttons[button].is_released;
}
