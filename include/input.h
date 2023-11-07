#pragma once

#include "window.h"
#include "util/types.h"

#define MAX_GAMEPADS 4

typedef struct ButtonState
{
    b32 pressed;
    b32 released;
} ButtonState;

typedef struct MouseButtonState
{
    b32 pressed;
    b32 released;
    b32 double_clicked;
} MouseButtonState;

/* Keyboard */
/* NOTE(lucas): IMPORTANT(lucas): This enum is used for array indexing.
* The last value NUM_KEYS is used for declaring arrays of the proper size
* Careful when changing this enum!
*/ 
enum Key
{
    KEY_UNKNOWN = 0,
    KEY_A, KEY_B, KEY_C,
    KEY_D, KEY_E, KEY_F, KEY_G,
    KEY_H, KEY_I, KEY_J, KEY_K,
    KEY_L, KEY_M, KEY_N, KEY_O,
    KEY_P, KEY_Q, KEY_R, KEY_S,
    KEY_T, KEY_U, KEY_V, KEY_W,
    KEY_X, KEY_Y, KEY_Z,

    KEY_NUM0, KEY_NUM1, KEY_NUM2, KEY_NUM3, KEY_NUM4,
    KEY_NUM5, KEY_NUM6, KEY_NUM7, KEY_NUM8, KEY_NUM9,

    KEY_LBRACKET, KEY_RBRACKET, KEY_SEMICOLON, KEY_QUOTE, KEY_COMMA,
    KEY_PERIOD, KEY_SLASH, KEY_BACKSLASH, KEY_TILDE, KEY_EQUAL, KEY_HYPHEN,

    KEY_SPACE, KEY_ENTER, KEY_ESCAPE, KEY_BACKSPACE, KEY_TAB,

    // SHIFT, CONTROL, ALT, and SYSTEM are used when it does not matter
    // if the key is on the left or right of the keyboard.
    // The L/R variants are used for specific left/right keys.
    KEY_SHIFT, KEY_LSHIFT, KEY_RSHIFT, KEY_CONTROL, KEY_LCONTROL, KEY_RCONTROL,
    KEY_ALT, KEY_LALT, KEY_RALT, KEY_SYSTEM, KEY_LSYSTEM, KEY_RSYSTEM, KEY_MENU,
    KEY_PAGEUP, KEY_PAGEDOWN, KEY_END, KEY_HOME, KEY_INSERT, KEY_DEL,
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,

    KEY_NUMPAD0, KEY_NUMPAD1, KEY_NUMPAD2, KEY_NUMPAD3, KEY_NUMPAD4,
    KEY_NUMPAD5, KEY_NUMPAD6, KEY_NUMPAD7, KEY_NUMPAD8, KEY_NUMPAD9,

    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6,
    KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12,
    KEY_F13, KEY_F14, KEY_F15, KEY_F16, KEY_F17, KEY_F18,
    KEY_F19, KEY_F20, KEY_F21, KEY_F22, KEY_F23, KEY_F24,

    KEY_NUM_KEYS
};

// typedef void (*char_callback_func)(void* state, u64 code);
typedef struct Keyboard
{
    ButtonState keys[KEY_NUM_KEYS];
    u64 current_char;
    // char_callback_func char_callback;
} Keyboard;

inline b32 key_pressed(Keyboard* input, int key)
{
    return input->keys[key].pressed;
}

inline b32 key_released(Keyboard* input, int key)
{
    return input->keys[key].released;
}

/* Mouse */
/* NOTE(lucas): IMPORTANT(lucas): This enum is used for array indexing.
* The last value NUM_KEYS is used for declaring arrays of the proper size
* Careful when changing this enum!
*/ 
typedef enum MouseButton
{
    MOUSE_UNKONWN = 0,
    MOUSE_LEFT,
    MOUSE_MIDDLE,
    MOUSE_RIGHT,
    MOUSE_X1,
    MOUSE_X2,
    MOUSE_NUM_BUTTONS
} MouseButton;

typedef struct Mouse
{
    i32 x;
    i32 y;
    i32 scroll;
    MouseButtonState buttons[MOUSE_NUM_BUTTONS];
} Mouse;

inline b32 mouse_button_pressed(Mouse* mouse, int button)
{
    return mouse->buttons[button].pressed;
}

inline b32 mouse_button_released(Mouse* mouse, int button)
{
    return mouse->buttons[button].released;
}

inline b32 mouse_button_double_clicked(Mouse* mouse, int button)
{
    return mouse->buttons[button].double_clicked;
}

/* Cursor */
typedef enum CursorType
{
    CURSOR_ARROW = 0,
    CURSOR_ARROW_WAIT,
    CURSOR_WAIT,
    CURSOR_TEXT,
    CURSOR_HAND,
    CURSOR_SIZE_HORIZONTAL,
    CURSOR_SIZE_VERTICAL,
    CURSOR_SIZE_TOP_LEFT_BOTTOM_RIGHT,
    CURSOR_SIZE_TOP_RIGHT_BOTTOM_LEFT,
    CURSOR_SIZE_LEFT,
    CURSOR_SIZE_RIGHT,
    CURSOR_SIZE_TOP,
    CURSOR_SIZE_BOTTOM,
    CURSOR_SIZE_TOP_LEFT,
    CURSOR_SIZE_TOP_RIGHT,
    CURSOR_SIZE_BOTTOM_LEFT,
    CURSOR_SIZE_BOTTOM_RIGHT,
    CURSOR_SIZE_ALL,
    CURSOR_CROSS,
    CURSOR_HELP,
    CURSOR_NOT_ALLOWED
} CursorType;

/* Wrappers for platform-specific functions */
void  cursor_show(b32 show);
void  cursor_set_from_system(CursorType type);
void* cursor_load_from_file(const char* filename);
void  cursor_set_from_memory(void* cursor);

/* Gamepad */
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

inline b32 gamepad_button_pressed(ButtonState button)
{
    return button.pressed;
}

inline b32 gamepad_button_released(ButtonState button)
{
    return button.released;
}

inline void gamepad_set_vibration(Gamepad* pad, u16 left_vibration, u16 right_vibration)
{
    pad->left_vibration = left_vibration;
    pad->right_vibration = right_vibration;
}

/* Struct containing collection of input devices */
typedef struct Input
{
    Gamepad gamepads[MAX_GAMEPADS];
    Keyboard keyboard;
    Mouse mouse;
} Input;

void input_process(Window* window, Input* input);

/* Clipboard */
b32  clipboard_write_string(char* text);
char* clipboard_read_string(void);
