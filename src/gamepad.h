#pragma once

#include "types.h"

struct GamepadButtonState
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
        GamepadButtonState buttons[32];
        struct
        {
            GamepadButtonState a_button;
            GamepadButtonState b_button;
            GamepadButtonState x_button;
            GamepadButtonState y_button;

            GamepadButtonState dpad_up;
            GamepadButtonState dpad_down;
            GamepadButtonState dpad_left;
            GamepadButtonState dpad_right;

            GamepadButtonState left_shoulder;
            GamepadButtonState right_shoulder;

            GamepadButtonState start_button;
            GamepadButtonState back_button;

            // NOTE(lucas): There are situations where it only matters whether a trigger is activated
            // (for example, when shooting a gun).
            GamepadButtonState left_trigger;
            GamepadButtonState right_trigger;

            // NOTE(lucas): Keep button states on sticks for situations where it is only necessary to know
            // direction and not magnitude (for example, navigating a menu with the stick or moving twice to dash).
            // Also, keep states for pressing sticks in
            GamepadButtonState left_stick_press;
            GamepadButtonState left_stick_up;
            GamepadButtonState left_stick_down;
            GamepadButtonState left_stick_left;
            GamepadButtonState left_stick_right;
            GamepadButtonState left_stick_upleft;
            GamepadButtonState left_stick_upright;
            GamepadButtonState left_stick_downleft;
            GamepadButtonState left_stick_downright;

            GamepadButtonState right_stick_press;
            GamepadButtonState right_stick_up;
            GamepadButtonState right_stick_down;
            GamepadButtonState right_stick_left;
            GamepadButtonState right_stick_right;
            GamepadButtonState right_stick_upleft;
            GamepadButtonState right_stick_upright;
            GamepadButtonState right_stick_downleft;
            GamepadButtonState right_stick_downright;
        };
    };  
};

inline bool is_gamepad_button_pressed(GamepadButtonState button)
{
    return button.is_pressed;
}

inline bool is_gamepad_button_released(GamepadButtonState button)
{
    return button.is_released;
}

inline void gamepad_set_vibration(Gamepad* pad, u16 left_vibration, u16 right_vibration)
{
    pad->left_vibration = left_vibration;
    pad->right_vibration = right_vibration;
}
