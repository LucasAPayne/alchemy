#pragma once

#include "types.h"

#define MAX_GAMEPADS 4

typedef struct ButtonState
{
    bool is_pressed;
    bool is_released;
} ButtonState;

/* IMPORTANT(lucas): This enum is used for array indexing.
* The last value NUM_KEYS is used for declaring arrays of the proper size
* Careful when changing this enum!
*/ 
enum Key
{
    UNKNOWN = 0,
    A, B, C,
    D, E, F, G,
    H, I, J, K,
    L, M, N, O,
    P, Q, R, S,
    T, U, V, W,
    X, Y, Z,

    NUM0, NUM1, NUM2, NUM3, NUM4,
    NUM5, NUM6, NUM7, NUM8, NUM9,

    LBRACKET, RBRACKET, SEMICOLON, QUOTE, COMMA,
    PERIOD, SLASH, BACKSLASH, TILDE, EQUAL, HYPHEN,

    SPACE, ENTER, ESCAPE, BACKSPACE, TAB,
    LSHIFT, RSHIFT, LCONTROL, RCONTROL,
    LALT, RALT, LSYSTEM, RSYSTEM, MENU,
    PAGEUP, PAGEDOWN, END, HOME, INSERT, DEL,
    UP, DOWN, LEFT, RIGHT,

    NUMPAD0, NUMPAD1, NUMPAD2, NUMPAD3, NUMPAD4,
    NUMPAD5, NUMPAD6, NUMPAD7, NUMPAD8, NUMPAD9,

    F1, F2, F3, F4, F5, F6, 
    F7, F8, F9, F10, F11, F12,

    NUM_KEYS
};

typedef struct Keyboard
{
    ButtonState keys[Key::NUM_KEYS];
} Keyboard;

inline bool is_key_pressed(Keyboard* input, int key)
{
    return input->keys[key].is_pressed;
}

inline bool is_key_released(Keyboard* input, int key)
{
    return input->keys[key].is_released;
}

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

typedef struct Mouse
{
    i32 x;
    i32 y;
    ButtonState buttons[MouseButton::NUM_BUTTONS];
} Mouse;

inline bool is_mouse_button_pressed(Mouse* mouse, int button)
{
    return mouse->buttons[button].is_pressed;
}

inline bool is_mouse_button_released(Mouse* mouse, int button)
{
    return mouse->buttons[button].is_released;
}

// TODO(lucas): Add support for multiple gamepads
typedef struct Gamepad
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
} Gamepad;

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

typedef struct Input
{
    Gamepad gamepads[MAX_GAMEPADS];
    Keyboard keyboard;
    Mouse mouse;
} Input;
