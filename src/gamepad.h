#pragma once

#include "types.h"

struct ButtonState
{
    bool is_pressed;
    bool is_released;
};

// TODO(lucas): Add support for multiple gamepads
struct Gamepad
{
    b32 is_connected;

    f32 left_stick_x;
    f32 left_stick_y;
    f32 right_stick_x;
    f32 right_stick_y;

    f32 left_trigger_val;
    f32 right_trigger_val;

    u16 left_vibration;
    u16 right_vibration;

    // NOTE(lucas): This union makes it easy to loop through all buttons when necessary
    union
    {
        ButtonState buttons[32];
        struct
        {
            ButtonState a_button;
            ButtonState b_button;
            ButtonState x_button;
            ButtonState y_button;

            ButtonState dpad_up;
            ButtonState dpad_down;
            ButtonState dpad_left;
            ButtonState dpad_right;

            ButtonState left_shoulder;
            ButtonState right_shoulder;

            ButtonState start_button;
            ButtonState back_button;

            // NOTE(lucas): There are situations where it only matters whether a trigger is activated
            // (for example, when shooting a gun).
            ButtonState left_trigger;
            ButtonState right_trigger;

            // NOTE(lucas): Keep button states on sticks for situations where it is only necessary to know
            // direction and not magnitude (for example, navigating a menu with the stick or moving twice to dash).
            // Also, keep states for pressing sticks in
            ButtonState left_stick_press;
            ButtonState left_stick_up;
            ButtonState left_stick_down;
            ButtonState left_stick_left;
            ButtonState left_stick_right;
            ButtonState left_stick_upleft;
            ButtonState left_stick_upright;
            ButtonState left_stick_downleft;
            ButtonState left_stick_downright;

            ButtonState right_stick_press;
            ButtonState right_stick_up;
            ButtonState right_stick_down;
            ButtonState right_stick_left;
            ButtonState right_stick_right;
            ButtonState right_stick_upleft;
            ButtonState right_stick_upright;
            ButtonState right_stick_downleft;
            ButtonState right_stick_downright;
        };
    };  
};

inline bool is_gamepad_button_pressed(ButtonState button)
{
    return button.is_pressed;
}

inline bool is_gamepad_button_released(ButtonState button)
{
    return button.is_released;
}

inline void gamepad_set_vibration(Gamepad* pad, u16 left_vibration, u16 right_vibration)
{
    pad->left_vibration = left_vibration;
    pad->right_vibration = right_vibration;
}
