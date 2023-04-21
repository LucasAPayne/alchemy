#include "win32_mouse.h"
#include "mouse.h"
#include "types.h"

#include <windows.h>

internal void win32_process_mouse_button(MouseButtonState* button, bool is_down)
{
    // Detect release
    if (button->is_pressed && !is_down)
        button->is_released = true;

    // Detect press
    if (button->is_pressed != is_down)
        button->is_pressed = is_down;
}

void win32_process_mouse_input(Mouse* mouse)
{
    // Release tate should not persist, so set it to false
    for (int button = 0; button < ARRAY_COUNT(mouse->buttons); button++)
        mouse->buttons[button].is_released = false;

    // TODO(lucas): Support mouse scrolling
    POINT mouse_pos = {0};
    GetCursorPos(&mouse_pos);
    mouse->x = mouse_pos.x;
    mouse->y = mouse_pos.y;

    // TODO(lucas): Revisit in the future and see if there is a better way to handle mouse input
    // It can't be handled the same way as keyboard input since mouse input seems to always go
    // through the window callback, and I do not want to pass the input struct to the callback.
    int pressed = 0x80;
    bool mouse_left_down   = (GetKeyState(VK_LBUTTON)  & pressed) != 0;
    bool mouse_middle_down = (GetKeyState(VK_MBUTTON)  & pressed) != 0;
    bool mouse_right_down  = (GetKeyState(VK_RBUTTON)  & pressed) != 0;
    bool mouse_x1_down     = (GetKeyState(VK_XBUTTON1) & pressed) != 0;
    bool mouse_x2_down     = (GetKeyState(VK_XBUTTON2) & pressed) != 0;

    win32_process_mouse_button(&mouse->buttons[MOUSE_LEFT],   mouse_left_down);
    win32_process_mouse_button(&mouse->buttons[MOUSE_MIDDLE], mouse_middle_down);
    win32_process_mouse_button(&mouse->buttons[MOUSE_RIGHT],  mouse_right_down);
    win32_process_mouse_button(&mouse->buttons[MOUSE_X1],     mouse_x1_down);
    win32_process_mouse_button(&mouse->buttons[MOUSE_X2],     mouse_x2_down);
}
