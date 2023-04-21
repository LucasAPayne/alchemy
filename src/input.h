#pragma once

#define MAX_GAMEPADS 4

// TODO(lucas): Consolidate all input files into this one
#include "gamepad.h"
#include "keyboard.h"
#include "mouse.h"

struct Input
{
    Gamepad gamepads[MAX_GAMEPADS];
    Keyboard keyboard;
    Mouse mouse;
};
