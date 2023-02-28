#pragma once

#include "keyboard.h"

#include <windows.h>

void win32_process_keyboard_input(HWND window, KeyboardInput* key_input);
