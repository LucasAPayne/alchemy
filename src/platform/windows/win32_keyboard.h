#pragma once

#include "keyboard.h"

#include <windows.h>

void win32_process_keyboard_input(HWND window, keyboard_input* key_input);
