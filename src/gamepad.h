#pragma once

#include "types.h"

struct button_state
{
    bool is_pressed;
    bool is_released;
};

// TODO(lucas): Add support for multiple gamepads
struct gamepad
{
    bool32 is_connected;

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
        button_state buttons[32];
        struct
        {
            button_state a_button;
            button_state b_button;
            button_state x_button;
            button_state y_button;

            button_state dpad_up;
            button_state dpad_down;
            button_state dpad_left;
            button_state dpad_right;

            button_state left_shoulder;
            button_state right_shoulder;

            button_state start_button;
            button_state back_button;

            // NOTE(lucas): There are situations where it only matters whether a trigger is activated
            // (for example, when shooting a gun).
            button_state left_trigger;
            button_state right_trigger;

            // NOTE(lucas): Keep button states on sticks for situations where it is only necessary to know
            // direction and not magnitude (for example, navigating a menu with the stick or moving twice to dash).
            // Also, keep states for pressing sticks in
            button_state left_stick_press;
            button_state left_stick_up;
            button_state left_stick_down;
            button_state left_stick_left;
            button_state left_stick_right;
            button_state left_stick_upleft;
            button_state left_stick_upright;
            button_state left_stick_downleft;
            button_state left_stick_downright;

            button_state right_stick_press;
            button_state right_stick_up;
            button_state right_stick_down;
            button_state right_stick_left;
            button_state right_stick_right;
            button_state right_stick_upleft;
            button_state right_stick_upright;
            button_state right_stick_downleft;
            button_state right_stick_downright;
        };
    };  
};

inline bool is_gamepad_button_pressed(button_state button)
{
    return button.is_pressed;
}

inline bool is_gamepad_button_released(button_state button)
{
    return button.is_released;
}

inline void gamepad_set_vibration(gamepad* pad, u16 left_vibration, u16 right_vibration)
{
    pad->left_vibration = left_vibration;
    pad->right_vibration = right_vibration;
}
