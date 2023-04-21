#pragma once

#include "types.h"

struct KeyState
{
    bool is_pressed;
    bool is_released;
};

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

struct Keyboard
{
    KeyState keys[Key::NUM_KEYS];
};

inline bool is_key_pressed(Keyboard* input, int key)
{
    return input->keys[key].is_pressed;
}

inline bool is_key_released(Keyboard* input, int key)
{
    return input->keys[key].is_released;
}
