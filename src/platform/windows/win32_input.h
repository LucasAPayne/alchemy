#pragma once

#include "input.h"

#include <windows.h>

void win32_process_keyboard_input(HWND window, Keyboard* key_input);
void win32_process_mouse_input(Mouse* mouse);
void win32_process_xinput_gamepad_input(Input* input);
