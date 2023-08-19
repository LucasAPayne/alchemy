#include "win32_input.h"
#include "input.h"
#include "util/types.h"

#include <windows.h>
#include <Windowsx.h>
#include <Xinput.h>


internal WORD map_extended_keys(WPARAM wparam, LPARAM lparam)
{
    /* NOTE(lucas): Processing keys through PeekMessage does not distinguish between left and right keys.
     * For example, VK_LSHIFT and VK_RSHIFT are just converted to VK_SHIFT, which is true if either shift key is
     * pressed.
     * This function converts generic virtual-key codes to specific left and right ones.
     */ 
    WORD vk = LOWORD(wparam); // virtual-key code
    WORD key_flags = HIWORD(lparam);
    WORD scan_code = LOBYTE(key_flags); // scancode
    BOOL extended = (key_flags & KF_EXTENDED) == KF_EXTENDED; // extended-key flag, 1 if scancode has 0xE0 prefix
    
    if (extended)
        scan_code = MAKEWORD(scan_code, 0xE0);

    switch (vk)
    {
        case VK_SHIFT:   // converts to VK_LSHIFT or VK_RSHIFT
        case VK_CONTROL: // converts to VK_LCONTROL or VK_RCONTROL
        case VK_MENU:    // converts to VK_LMENU or VK_RMENU
            vk = LOWORD(MapVirtualKeyW(scan_code, MAPVK_VSC_TO_VK_EX));
            break;

        default: break;
    }

    return vk;
}

internal void win32_process_key(ButtonState* key, b32 is_down, b32 was_down)
{
    key->is_pressed = is_down;
    key->is_released = was_down && !is_down;
}

internal int win32_get_mouse_xbutton(MSG msg)
{
    int result = 0;
    int xbutton = GET_XBUTTON_WPARAM(msg.wParam);

    if (xbutton == XBUTTON1)
        result = MOUSE_X1;
    else if (xbutton == XBUTTON2)
        result = MOUSE_X2;

    return result;
}

internal void win32_process_mouse_button(ButtonState* button, int clicks)
{
    // NOTE(lucas): Clicks is the number of times key was pressed:
    // 0: released, 1: pressed, 2: double click
    switch(clicks)
    {
        case 0: // released
        {
            button->is_released = true;
            button->is_pressed = false;
        } break;

        case 1: // pressed
        {
            button->is_pressed = true;
        } break;

        case 2: // double clicked
        {
            button->is_double_clicked = true;
        } break;
    }
}

// TODO(lucas): Support for non-US keyboard layouts
void win32_process_keyboard_mouse_input(HWND window, Keyboard* key_input, Mouse* mouse)
{
    // Reset any character that was entered last frame
    key_input->current_char = 0;

    // Release state should not persist, so make sure it is false for each button
    for (int key = 0; key < ARRAY_COUNT(key_input->keys); key++)
        key_input->keys[key].is_released = false;

    // Release and double click state should not persist, so set it to false
    for (int button = 0; button < ARRAY_COUNT(mouse->buttons); button++)
    {
        mouse->buttons[button].is_released = false;
        mouse->buttons[button].is_double_clicked = false;
    }
    mouse->scroll = 0;

    MSG msg;
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        switch(msg.message)
        {
            /* Kyeboard Input */
            // Handle all key messages in the same block
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                /* FIXME: For keys that have extended variants (e.g., left/right shift), the input can be tricked into
                 * holding the keydown state. For example, if a game tests whether right shift is down, and both right
                 * and left shift are held down and then released, then the input system will think right shift is still
                 * down until right shift is pressed again.
                 * This bug was found by setting the clear color to magenta when an extended key is down. It is meant
                 * to change the clear color while the key is held, then change it back to the original color when the
                 * key is released.
                 */
                 u32 virtual_key_code = (u32)map_extended_keys(msg.wParam, msg.lParam);

                // NOTE(lucas): lparam is a bitfield that gives extra information about the message
                // Could just grab the value of the bit, but comparison forces result to be bool
                b32 was_down = (msg.lParam & (1 << 30)) != 0; // 30th bit is previous state (1 for down, 0 for up)
                b32 is_down = (msg.lParam & (1 << 31)) == 0; // 31st bit is transition, always 1 for keyup, 0 for keydown

                // Disregard key repeats
                // if (was_down == is_down)
                //     break;

                // NOTE(lucas): Most keycodes map directly to their ANSI equivalent (letters are capital)
                // For keys that have no ANSI equivalent, Windows provides defines for virtual keycodes
                switch(virtual_key_code)
                {
                    case 'A': win32_process_key(&key_input->keys[KEY_A], is_down, was_down); break;
                    case 'B': win32_process_key(&key_input->keys[KEY_B], is_down, was_down); break;
                    case 'C': win32_process_key(&key_input->keys[KEY_C], is_down, was_down); break;
                    case 'D': win32_process_key(&key_input->keys[KEY_D], is_down, was_down); break;
                    case 'E': win32_process_key(&key_input->keys[KEY_E], is_down, was_down); break;
                    case 'F': win32_process_key(&key_input->keys[KEY_F], is_down, was_down); break;
                    case 'G': win32_process_key(&key_input->keys[KEY_G], is_down, was_down); break;
                    case 'H': win32_process_key(&key_input->keys[KEY_H], is_down, was_down); break;
                    case 'I': win32_process_key(&key_input->keys[KEY_I], is_down, was_down); break;
                    case 'J': win32_process_key(&key_input->keys[KEY_J], is_down, was_down); break;
                    case 'K': win32_process_key(&key_input->keys[KEY_K], is_down, was_down); break;
                    case 'L': win32_process_key(&key_input->keys[KEY_L], is_down, was_down); break;
                    case 'M': win32_process_key(&key_input->keys[KEY_M], is_down, was_down); break;
                    case 'N': win32_process_key(&key_input->keys[KEY_N], is_down, was_down); break;
                    case 'O': win32_process_key(&key_input->keys[KEY_O], is_down, was_down); break;
                    case 'P': win32_process_key(&key_input->keys[KEY_P], is_down, was_down); break;
                    case 'Q': win32_process_key(&key_input->keys[KEY_Q], is_down, was_down); break;
                    case 'R': win32_process_key(&key_input->keys[KEY_R], is_down, was_down); break;
                    case 'S': win32_process_key(&key_input->keys[KEY_S], is_down, was_down); break;
                    case 'T': win32_process_key(&key_input->keys[KEY_T], is_down, was_down); break;
                    case 'U': win32_process_key(&key_input->keys[KEY_U], is_down, was_down); break;
                    case 'V': win32_process_key(&key_input->keys[KEY_V], is_down, was_down); break;
                    case 'W': win32_process_key(&key_input->keys[KEY_W], is_down, was_down); break;
                    case 'X': win32_process_key(&key_input->keys[KEY_X], is_down, was_down); break;
                    case 'Y': win32_process_key(&key_input->keys[KEY_Y], is_down, was_down); break;
                    case 'Z': win32_process_key(&key_input->keys[KEY_Z], is_down, was_down); break;

                    case '0': win32_process_key(&key_input->keys[KEY_NUM0], is_down, was_down); break;
                    case '1': win32_process_key(&key_input->keys[KEY_NUM1], is_down, was_down); break;
                    case '2': win32_process_key(&key_input->keys[KEY_NUM2], is_down, was_down); break;
                    case '3': win32_process_key(&key_input->keys[KEY_NUM3], is_down, was_down); break;
                    case '4': win32_process_key(&key_input->keys[KEY_NUM4], is_down, was_down); break;
                    case '5': win32_process_key(&key_input->keys[KEY_NUM5], is_down, was_down); break;
                    case '6': win32_process_key(&key_input->keys[KEY_NUM6], is_down, was_down); break;
                    case '7': win32_process_key(&key_input->keys[KEY_NUM7], is_down, was_down); break;
                    case '8': win32_process_key(&key_input->keys[KEY_NUM8], is_down, was_down); break;
                    case '9': win32_process_key(&key_input->keys[KEY_NUM9], is_down, was_down); break;

                    case VK_OEM_4:      win32_process_key(&key_input->keys[KEY_LBRACKET],  is_down, was_down); break;
                    case VK_OEM_6:      win32_process_key(&key_input->keys[KEY_RBRACKET],  is_down, was_down); break;
                    case VK_OEM_1:      win32_process_key(&key_input->keys[KEY_SEMICOLON], is_down, was_down); break;
                    case VK_OEM_7:      win32_process_key(&key_input->keys[KEY_QUOTE],     is_down, was_down); break;
                    case VK_OEM_COMMA:  win32_process_key(&key_input->keys[KEY_COMMA],     is_down, was_down); break;
                    case VK_OEM_PERIOD: win32_process_key(&key_input->keys[KEY_PERIOD],    is_down, was_down); break;
                    case VK_OEM_2:      win32_process_key(&key_input->keys[KEY_SLASH],     is_down, was_down); break;
                    case VK_OEM_5:      win32_process_key(&key_input->keys[KEY_BACKSLASH], is_down, was_down); break;
                    case VK_OEM_3:      win32_process_key(&key_input->keys[KEY_TILDE],     is_down, was_down); break;
                    case VK_OEM_PLUS:   win32_process_key(&key_input->keys[KEY_EQUAL],     is_down, was_down); break;
                    case VK_OEM_MINUS:  win32_process_key(&key_input->keys[KEY_HYPHEN],    is_down, was_down); break;

                    case VK_SPACE:      win32_process_key(&key_input->keys[KEY_SPACE],     is_down, was_down); break;
                    case VK_RETURN:     win32_process_key(&key_input->keys[KEY_ENTER],     is_down, was_down); break;
                    case VK_ESCAPE:     win32_process_key(&key_input->keys[KEY_ESCAPE],    is_down, was_down); break;
                    case VK_BACK:       win32_process_key(&key_input->keys[KEY_BACKSPACE], is_down, was_down); break;
                    case VK_TAB:        win32_process_key(&key_input->keys[KEY_TAB],       is_down, was_down); break;
                    case VK_LSHIFT:     win32_process_key(&key_input->keys[KEY_LSHIFT],    is_down, was_down); break;
                    case VK_RSHIFT:     win32_process_key(&key_input->keys[KEY_RSHIFT],    is_down, was_down); break;
                    case VK_LCONTROL:   win32_process_key(&key_input->keys[KEY_LCONTROL],  is_down, was_down); break;
                    case VK_RCONTROL:   win32_process_key(&key_input->keys[KEY_RCONTROL],  is_down, was_down); break;
                    case VK_LMENU:      win32_process_key(&key_input->keys[KEY_LALT],      is_down, was_down); break;
                    case VK_RMENU:      win32_process_key(&key_input->keys[KEY_RALT],      is_down, was_down); break;
                    case VK_LWIN:       win32_process_key(&key_input->keys[KEY_LSYSTEM],   is_down, was_down); break;
                    case VK_RWIN:       win32_process_key(&key_input->keys[KEY_RSYSTEM],   is_down, was_down); break;
                    case VK_APPS:       win32_process_key(&key_input->keys[KEY_MENU],      is_down, was_down); break;
                    case VK_PRIOR:      win32_process_key(&key_input->keys[KEY_PAGEUP],    is_down, was_down); break;
                    case VK_NEXT:       win32_process_key(&key_input->keys[KEY_PAGEDOWN],  is_down, was_down); break;
                    case VK_END:        win32_process_key(&key_input->keys[KEY_END],       is_down, was_down); break;
                    case VK_HOME:       win32_process_key(&key_input->keys[KEY_HOME],      is_down, was_down); break;
                    case VK_INSERT:     win32_process_key(&key_input->keys[KEY_INSERT],    is_down, was_down); break;
                    case VK_DELETE:     win32_process_key(&key_input->keys[KEY_DEL],       is_down, was_down); break;
                    case VK_UP:         win32_process_key(&key_input->keys[KEY_UP],        is_down, was_down); break;
                    case VK_DOWN:       win32_process_key(&key_input->keys[KEY_DOWN],      is_down, was_down); break;
                    case VK_LEFT:       win32_process_key(&key_input->keys[KEY_LEFT],      is_down, was_down); break;
                    case VK_RIGHT:      win32_process_key(&key_input->keys[KEY_RIGHT],     is_down, was_down); break;

                    case VK_NUMPAD0:    win32_process_key(&key_input->keys[KEY_NUMPAD0], is_down, was_down); break;
                    case VK_NUMPAD1:    win32_process_key(&key_input->keys[KEY_NUMPAD1], is_down, was_down); break;
                    case VK_NUMPAD2:    win32_process_key(&key_input->keys[KEY_NUMPAD2], is_down, was_down); break;
                    case VK_NUMPAD3:    win32_process_key(&key_input->keys[KEY_NUMPAD3], is_down, was_down); break;
                    case VK_NUMPAD4:    win32_process_key(&key_input->keys[KEY_NUMPAD4], is_down, was_down); break;
                    case VK_NUMPAD5:    win32_process_key(&key_input->keys[KEY_NUMPAD5], is_down, was_down); break;
                    case VK_NUMPAD6:    win32_process_key(&key_input->keys[KEY_NUMPAD6], is_down, was_down); break;
                    case VK_NUMPAD7:    win32_process_key(&key_input->keys[KEY_NUMPAD7], is_down, was_down); break;
                    case VK_NUMPAD8:    win32_process_key(&key_input->keys[KEY_NUMPAD8], is_down, was_down); break;
                    case VK_NUMPAD9:    win32_process_key(&key_input->keys[KEY_NUMPAD9], is_down, was_down); break;

                    case VK_F1:         win32_process_key(&key_input->keys[KEY_F1],  is_down, was_down); break;
                    case VK_F2:         win32_process_key(&key_input->keys[KEY_F2],  is_down, was_down); break;
                    case VK_F3:         win32_process_key(&key_input->keys[KEY_F3],  is_down, was_down); break;
                    case VK_F4:         win32_process_key(&key_input->keys[KEY_F4],  is_down, was_down); break;
                    case VK_F5:         win32_process_key(&key_input->keys[KEY_F5],  is_down, was_down); break;
                    case VK_F6:         win32_process_key(&key_input->keys[KEY_F6],  is_down, was_down); break;
                    case VK_F7:         win32_process_key(&key_input->keys[KEY_F7],  is_down, was_down); break;
                    case VK_F8:         win32_process_key(&key_input->keys[KEY_F8],  is_down, was_down); break;
                    case VK_F9:         win32_process_key(&key_input->keys[KEY_F9],  is_down, was_down); break;
                    case VK_F10:        win32_process_key(&key_input->keys[KEY_F10], is_down, was_down); break;
                    case VK_F11:        win32_process_key(&key_input->keys[KEY_F11], is_down, was_down); break;
                    case VK_F12:        win32_process_key(&key_input->keys[KEY_F12], is_down, was_down); break;
                }

                // Handle Alt+F4 closing window
                // NOTE(lucas): 29th bit is context (here, whether Alt is down)
                b32 alt_key_down = msg.lParam & (1 << 29);
                if ((virtual_key_code == VK_F4) && alt_key_down)
                    PostQuitMessage(0);

                TranslateMessage(&msg);
            } break;

            case WM_CHAR:
            {
                key_input->current_char = msg.wParam;
            } break;

            /* Mouse Input */
            case WM_MOUSEMOVE:
            {
                mouse->x = GET_X_LPARAM(msg.lParam);
                mouse->y = GET_Y_LPARAM(msg.lParam);
            } break;
            case WM_MOUSEWHEEL: mouse->scroll = GET_WHEEL_DELTA_WPARAM(msg.wParam) / WHEEL_DELTA; break;

            case WM_LBUTTONUP:     win32_process_mouse_button(&mouse->buttons[MOUSE_LEFT], 0); break;
            case WM_LBUTTONDOWN:   win32_process_mouse_button(&mouse->buttons[MOUSE_LEFT], 1); break;
            case WM_LBUTTONDBLCLK: win32_process_mouse_button(&mouse->buttons[MOUSE_LEFT], 2); break;

            case WM_MBUTTONUP:     win32_process_mouse_button(&mouse->buttons[MOUSE_MIDDLE], 0); break;
            case WM_MBUTTONDOWN:   win32_process_mouse_button(&mouse->buttons[MOUSE_MIDDLE], 1); break;
            case WM_MBUTTONDBLCLK: win32_process_mouse_button(&mouse->buttons[MOUSE_MIDDLE], 2); break;

            case WM_RBUTTONUP:     win32_process_mouse_button(&mouse->buttons[MOUSE_RIGHT], 0); break;
            case WM_RBUTTONDOWN:   win32_process_mouse_button(&mouse->buttons[MOUSE_RIGHT], 1); break;
            case WM_RBUTTONDBLCLK: win32_process_mouse_button(&mouse->buttons[MOUSE_RIGHT], 2); break;

            case WM_XBUTTONUP:     win32_process_mouse_button(&mouse->buttons[win32_get_mouse_xbutton(msg)], 0); break;
            case WM_XBUTTONDOWN:   win32_process_mouse_button(&mouse->buttons[win32_get_mouse_xbutton(msg)], 1); break;
            case WM_XBUTTONDBLCLK: win32_process_mouse_button(&mouse->buttons[win32_get_mouse_xbutton(msg)], 2); break;


            default:
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } break;
        }
    }
}

internal void win32_xinput_button_release(ButtonState* button)
{
    button->is_released = true;
    button->is_pressed  = false; 
}

internal void win32_process_xinput_buttons(XINPUT_KEYSTROKE keystroke, Gamepad* gamepad)
{
    // Disregard repeats
    if (keystroke.Flags & XINPUT_KEYSTROKE_REPEAT)
        return;

    if (keystroke.Flags & XINPUT_KEYSTROKE_KEYDOWN)
    {
        switch(keystroke.VirtualKey)
        {
            case VK_PAD_A: gamepad->a_button.is_pressed = true; break;
            case VK_PAD_B: gamepad->b_button.is_pressed = true; break;
            case VK_PAD_X: gamepad->x_button.is_pressed = true; break;
            case VK_PAD_Y: gamepad->y_button.is_pressed = true; break;

            case VK_PAD_LSHOULDER: gamepad->left_shoulder.is_pressed  = true; break;
            case VK_PAD_RSHOULDER: gamepad->right_shoulder.is_pressed = true; break;
            case VK_PAD_LTRIGGER:  gamepad->left_trigger.is_pressed   = true; break;
            case VK_PAD_RTRIGGER:  gamepad->right_trigger.is_pressed  = true; break;

            case VK_PAD_DPAD_UP:    gamepad->dpad_up.is_pressed    = true; break;
            case VK_PAD_DPAD_DOWN:  gamepad->dpad_down.is_pressed  = true; break;
            case VK_PAD_DPAD_LEFT:  gamepad->dpad_left.is_pressed  = true; break;
            case VK_PAD_DPAD_RIGHT: gamepad->dpad_right.is_pressed = true; break;

            case VK_PAD_START: gamepad->start_button.is_pressed = true; break;
            case VK_PAD_BACK:  gamepad->back_button.is_pressed  = true; break;

            case VK_PAD_LTHUMB_PRESS:     gamepad->left_stick_press.is_pressed     = true; break;
            case VK_PAD_LTHUMB_UP:        gamepad->left_stick_up.is_pressed        = true; break;
            case VK_PAD_LTHUMB_DOWN:      gamepad->left_stick_down.is_pressed      = true; break;
            case VK_PAD_LTHUMB_LEFT:      gamepad->left_stick_left.is_pressed      = true; break;
            case VK_PAD_LTHUMB_RIGHT:     gamepad->left_stick_right.is_pressed     = true; break;
            case VK_PAD_LTHUMB_UPLEFT:    gamepad->left_stick_upleft.is_pressed    = true; break;
            case VK_PAD_LTHUMB_UPRIGHT:   gamepad->left_stick_upright.is_pressed   = true; break;
            case VK_PAD_LTHUMB_DOWNLEFT:  gamepad->left_stick_downleft.is_pressed  = true; break;
            case VK_PAD_LTHUMB_DOWNRIGHT: gamepad->left_stick_downright.is_pressed = true; break;

            case VK_PAD_RTHUMB_PRESS:     gamepad->right_stick_press.is_pressed     = true; break;
            case VK_PAD_RTHUMB_UP:        gamepad->right_stick_up.is_pressed        = true; break;
            case VK_PAD_RTHUMB_DOWN:      gamepad->right_stick_down.is_pressed      = true; break;
            case VK_PAD_RTHUMB_LEFT:      gamepad->right_stick_left.is_pressed      = true; break;
            case VK_PAD_RTHUMB_RIGHT:     gamepad->right_stick_right.is_pressed     = true; break;
            case VK_PAD_RTHUMB_UPLEFT:    gamepad->right_stick_upleft.is_pressed    = true; break;
            case VK_PAD_RTHUMB_UPRIGHT:   gamepad->right_stick_upright.is_pressed   = true; break;
            case VK_PAD_RTHUMB_DOWNLEFT:  gamepad->right_stick_downleft.is_pressed  = true; break;
            case VK_PAD_RTHUMB_DOWNRIGHT: gamepad->right_stick_downright.is_pressed = true; break;

            default: break;
        }
    }
    else if (keystroke.Flags & XINPUT_KEYSTROKE_KEYUP)
    {
        switch(keystroke.VirtualKey)
        {
            case VK_PAD_A: win32_xinput_button_release(&gamepad->a_button); break;
            case VK_PAD_B: win32_xinput_button_release(&gamepad->b_button); break;
            case VK_PAD_X: win32_xinput_button_release(&gamepad->x_button); break;
            case VK_PAD_Y: win32_xinput_button_release(&gamepad->y_button); break;

            case VK_PAD_LSHOULDER: win32_xinput_button_release(&gamepad->left_shoulder); break;
            case VK_PAD_RSHOULDER: win32_xinput_button_release(&gamepad->right_shoulder); break;
            case VK_PAD_LTRIGGER:  win32_xinput_button_release(&gamepad->left_trigger); break;
            case VK_PAD_RTRIGGER:  win32_xinput_button_release(&gamepad->right_trigger); break;

            case VK_PAD_DPAD_UP:    win32_xinput_button_release(&gamepad->dpad_up); break;
            case VK_PAD_DPAD_DOWN:  win32_xinput_button_release(&gamepad->dpad_down); break;
            case VK_PAD_DPAD_LEFT:  win32_xinput_button_release(&gamepad->dpad_left); break;
            case VK_PAD_DPAD_RIGHT: win32_xinput_button_release(&gamepad->dpad_right); break;

            case VK_PAD_START: win32_xinput_button_release(&gamepad->start_button); break;
            case VK_PAD_BACK:  win32_xinput_button_release(&gamepad->back_button); break;

            case VK_PAD_LTHUMB_PRESS:     win32_xinput_button_release(&gamepad->left_stick_press); break;
            case VK_PAD_LTHUMB_UP:        win32_xinput_button_release(&gamepad->left_stick_up); break;
            case VK_PAD_LTHUMB_DOWN:      win32_xinput_button_release(&gamepad->left_stick_down); break;
            case VK_PAD_LTHUMB_LEFT:      win32_xinput_button_release(&gamepad->left_stick_left); break;
            case VK_PAD_LTHUMB_RIGHT:     win32_xinput_button_release(&gamepad->left_stick_right); break;
            case VK_PAD_LTHUMB_UPLEFT:    win32_xinput_button_release(&gamepad->left_stick_upleft); break;
            case VK_PAD_LTHUMB_UPRIGHT:   win32_xinput_button_release(&gamepad->left_stick_upright); break;
            case VK_PAD_LTHUMB_DOWNLEFT:  win32_xinput_button_release(&gamepad->left_stick_downleft); break;
            case VK_PAD_LTHUMB_DOWNRIGHT: win32_xinput_button_release(&gamepad->left_stick_downright); break;

            case VK_PAD_RTHUMB_PRESS:     win32_xinput_button_release(&gamepad->right_stick_press); break;
            case VK_PAD_RTHUMB_UP:        win32_xinput_button_release(&gamepad->right_stick_up); break;
            case VK_PAD_RTHUMB_DOWN:      win32_xinput_button_release(&gamepad->right_stick_down); break;
            case VK_PAD_RTHUMB_LEFT:      win32_xinput_button_release(&gamepad->right_stick_left); break;
            case VK_PAD_RTHUMB_RIGHT:     win32_xinput_button_release(&gamepad->right_stick_right); break;
            case VK_PAD_RTHUMB_UPLEFT:    win32_xinput_button_release(&gamepad->right_stick_upleft); break;
            case VK_PAD_RTHUMB_UPRIGHT:   win32_xinput_button_release(&gamepad->right_stick_upright); break;
            case VK_PAD_RTHUMB_DOWNLEFT:  win32_xinput_button_release(&gamepad->right_stick_downleft); break;
            case VK_PAD_RTHUMB_DOWNRIGHT: win32_xinput_button_release(&gamepad->right_stick_downright); break;
            
            default: break;
        }
    }
}

internal f32 win32_process_xinput_stick(SHORT xinput_stick_value, SHORT deadzone)
{
    f32 result = 0.0f;
    if (xinput_stick_value > deadzone)
    {
        // Clamp to max expected value
        if (xinput_stick_value > SHRT_MAX)
            xinput_stick_value = SHRT_MAX;

        // Adjust magnitude relative to deadzone value and normalize
        xinput_stick_value -= deadzone;
        result = (f32)xinput_stick_value / (f32)(SHRT_MAX - deadzone);
    }
    else if (xinput_stick_value < -deadzone)
    {
        if (xinput_stick_value < SHRT_MIN)
            xinput_stick_value = SHRT_MIN;
        
        xinput_stick_value += deadzone;
        result = (f32)xinput_stick_value / (f32)(SHRT_MAX - deadzone);
    }
    return result;
}

internal f32 win32_process_xinput_trigger(BYTE xinput_trigger_value)
{
    f32 result = 0.0f;
    if (xinput_trigger_value > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
    {
        if (xinput_trigger_value > UCHAR_MAX)
            xinput_trigger_value = UCHAR_MAX;

        // Normalize
        xinput_trigger_value -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
        result = (f32)xinput_trigger_value / (f32)(UCHAR_MAX - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
    }
    return result;
}

void win32_process_xinput_gamepad_input(Input* input)
{
    Gamepad* gamepad = {0};
    for (int i = 0; i < MAX_GAMEPADS; ++i)
    {
        gamepad = &input->gamepads[i];

        // Release state should not persist, so make sure it is false for each button
        for (int button_index = 0; button_index < ARRAY_COUNT(gamepad->buttons); button_index++)
            gamepad->buttons[button_index].is_released = false;

        XINPUT_STATE controller_state;
        ZeroMemory(&controller_state, sizeof(XINPUT_STATE));

        if (XInputGetState(i, &controller_state) != ERROR_SUCCESS)
        {
            // NOTE(lucas): The controller is not available
            gamepad->is_connected = false;
            continue;
        }
        gamepad->is_connected = true;

        // Buttons
        XINPUT_KEYSTROKE keystroke = {0};
        DWORD status = XInputGetKeystroke(i, 0, &keystroke);
        switch(status)
        {
            case ERROR_EMPTY: break; // Queue is empty

            case ERROR_DEVICE_NOT_CONNECTED:
            {
                // Controller at index not connected
                if (gamepad->is_connected)
                {
                    // Controller was connected and has become disconnected
                    gamepad->is_connected = false;
                    // TODO(lucas): Logging
                }
            } break; 
            case ERROR_SUCCESS:
            {
                win32_process_xinput_buttons(keystroke, gamepad);
            } break;    
            default: break;
        }

        // Sticks
        XINPUT_GAMEPAD xinput_gamepad = controller_state.Gamepad;
        gamepad->left_stick_x = win32_process_xinput_stick(xinput_gamepad.sThumbLX, 
                                                            XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        gamepad->left_stick_y = -win32_process_xinput_stick(xinput_gamepad.sThumbLY,
                                                            XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        gamepad->right_stick_x = win32_process_xinput_stick(xinput_gamepad.sThumbRX,
                                                            XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        gamepad->right_stick_y = -win32_process_xinput_stick(xinput_gamepad.sThumbRY,
                                                            XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);

        // Triggers
        gamepad->left_trigger_val = win32_process_xinput_trigger(xinput_gamepad.bLeftTrigger);
        gamepad->right_trigger_val = win32_process_xinput_trigger(xinput_gamepad.bRightTrigger);

        // Vibration
        XINPUT_VIBRATION vibration = {0};
        vibration.wLeftMotorSpeed = gamepad->left_vibration;
        vibration.wRightMotorSpeed = gamepad->right_vibration;
        XInputSetState(0, &vibration);
        gamepad->left_vibration = gamepad->right_vibration = 0;
    }
}
