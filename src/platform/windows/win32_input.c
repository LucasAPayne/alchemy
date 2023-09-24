#include "input.h"
#include "window.h"
#include "util/types.h"

#include <windows.h>
#include <Windowsx.h>
#include <Xinput.h>

global WINDOWPLACEMENT global_window_position_prev = {sizeof(global_window_position_prev)};

internal void win32_toggle_fullscreen(HWND window)
{
    // TODO(lucas): Update to use Get/SetWindowLongPtr functions instead of Get/SetWindowLong?

    // NOTE(lucas): Based on Raymond Chen's blog on fullscreen toggling:
    // https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
    DWORD style = GetWindowLongA(window, GWL_STYLE);
    if (style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO monitor_info = {sizeof(monitor_info)};
        if (GetWindowPlacement(window, &global_window_position_prev) && 
            GetMonitorInfoA(MonitorFromWindow(window, MONITOR_DEFAULTTOPRIMARY), &monitor_info))
        {
            /* NOTE(lucas): Make sure there are no styles that could potentially cause aritfacts, borders, etc.
             * in fullscreen mode.
             * In my case, one of the extended border styles was adding a 2 pixel white border
             * around the whole screen.
             */
            style &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
            DWORD ex_style = GetWindowLong(window, GWL_EXSTYLE);
            ex_style &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
            SetWindowLongA(window, GWL_EXSTYLE, ex_style);

            SetWindowLongA(window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(window, HWND_TOP,
                         monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
                         monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                         monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLongA(window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(window, &global_window_position_prev);
        SetWindowPos(window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

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
    key->pressed = is_down;
    key->released = was_down && !is_down;
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

internal void win32_process_mouse_button(MouseButtonState* button, int clicks)
{
    // NOTE(lucas): Clicks is the number of times key was pressed:
    // 0: released, 1: pressed, 2: double click
    switch(clicks)
    {
        case 0: // released
        {
            button->released = true;
            button->pressed = false;
        } break;

        case 1: // pressed
        {
            button->pressed = true;
        } break;

        case 2: // double clicked
        {
            button->double_clicked = true;
        } break;
    }
}

// TODO(lucas): Support for non-US keyboard layouts
void win32_keyboard_mouse_process_input(Window* window, Input* input)
{
    Keyboard* keyboard = &input->keyboard;
    Mouse* mouse = &input->mouse;

    // Reset any character that was entered last frame
    keyboard->current_char = 0;

    // Release state should not persist, so make sure it is false for each button
    for (int key = 0; key < ARRAY_COUNT(keyboard->keys); key++)
        keyboard->keys[key].released = false;

    // Release and double click state should not persist, so set it to false
    for (int button = 0; button < ARRAY_COUNT(mouse->buttons); button++)
    {
        mouse->buttons[button].released = false;
        mouse->buttons[button].double_clicked = false;
    }
    mouse->scroll = 0;

    MSG msg;
    while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
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
                    case 'A': win32_process_key(&keyboard->keys[KEY_A], is_down, was_down); break;
                    case 'B': win32_process_key(&keyboard->keys[KEY_B], is_down, was_down); break;
                    case 'C': win32_process_key(&keyboard->keys[KEY_C], is_down, was_down); break;
                    case 'D': win32_process_key(&keyboard->keys[KEY_D], is_down, was_down); break;
                    case 'E': win32_process_key(&keyboard->keys[KEY_E], is_down, was_down); break;
                    case 'F': win32_process_key(&keyboard->keys[KEY_F], is_down, was_down); break;
                    case 'G': win32_process_key(&keyboard->keys[KEY_G], is_down, was_down); break;
                    case 'H': win32_process_key(&keyboard->keys[KEY_H], is_down, was_down); break;
                    case 'I': win32_process_key(&keyboard->keys[KEY_I], is_down, was_down); break;
                    case 'J': win32_process_key(&keyboard->keys[KEY_J], is_down, was_down); break;
                    case 'K': win32_process_key(&keyboard->keys[KEY_K], is_down, was_down); break;
                    case 'L': win32_process_key(&keyboard->keys[KEY_L], is_down, was_down); break;
                    case 'M': win32_process_key(&keyboard->keys[KEY_M], is_down, was_down); break;
                    case 'N': win32_process_key(&keyboard->keys[KEY_N], is_down, was_down); break;
                    case 'O': win32_process_key(&keyboard->keys[KEY_O], is_down, was_down); break;
                    case 'P': win32_process_key(&keyboard->keys[KEY_P], is_down, was_down); break;
                    case 'Q': win32_process_key(&keyboard->keys[KEY_Q], is_down, was_down); break;
                    case 'R': win32_process_key(&keyboard->keys[KEY_R], is_down, was_down); break;
                    case 'S': win32_process_key(&keyboard->keys[KEY_S], is_down, was_down); break;
                    case 'T': win32_process_key(&keyboard->keys[KEY_T], is_down, was_down); break;
                    case 'U': win32_process_key(&keyboard->keys[KEY_U], is_down, was_down); break;
                    case 'V': win32_process_key(&keyboard->keys[KEY_V], is_down, was_down); break;
                    case 'W': win32_process_key(&keyboard->keys[KEY_W], is_down, was_down); break;
                    case 'X': win32_process_key(&keyboard->keys[KEY_X], is_down, was_down); break;
                    case 'Y': win32_process_key(&keyboard->keys[KEY_Y], is_down, was_down); break;
                    case 'Z': win32_process_key(&keyboard->keys[KEY_Z], is_down, was_down); break;

                    case '0': win32_process_key(&keyboard->keys[KEY_NUM0], is_down, was_down); break;
                    case '1': win32_process_key(&keyboard->keys[KEY_NUM1], is_down, was_down); break;
                    case '2': win32_process_key(&keyboard->keys[KEY_NUM2], is_down, was_down); break;
                    case '3': win32_process_key(&keyboard->keys[KEY_NUM3], is_down, was_down); break;
                    case '4': win32_process_key(&keyboard->keys[KEY_NUM4], is_down, was_down); break;
                    case '5': win32_process_key(&keyboard->keys[KEY_NUM5], is_down, was_down); break;
                    case '6': win32_process_key(&keyboard->keys[KEY_NUM6], is_down, was_down); break;
                    case '7': win32_process_key(&keyboard->keys[KEY_NUM7], is_down, was_down); break;
                    case '8': win32_process_key(&keyboard->keys[KEY_NUM8], is_down, was_down); break;
                    case '9': win32_process_key(&keyboard->keys[KEY_NUM9], is_down, was_down); break;

                    case VK_OEM_4:      win32_process_key(&keyboard->keys[KEY_LBRACKET],  is_down, was_down); break;
                    case VK_OEM_6:      win32_process_key(&keyboard->keys[KEY_RBRACKET],  is_down, was_down); break;
                    case VK_OEM_1:      win32_process_key(&keyboard->keys[KEY_SEMICOLON], is_down, was_down); break;
                    case VK_OEM_7:      win32_process_key(&keyboard->keys[KEY_QUOTE],     is_down, was_down); break;
                    case VK_OEM_COMMA:  win32_process_key(&keyboard->keys[KEY_COMMA],     is_down, was_down); break;
                    case VK_OEM_PERIOD: win32_process_key(&keyboard->keys[KEY_PERIOD],    is_down, was_down); break;
                    case VK_OEM_2:      win32_process_key(&keyboard->keys[KEY_SLASH],     is_down, was_down); break;
                    case VK_OEM_5:      win32_process_key(&keyboard->keys[KEY_BACKSLASH], is_down, was_down); break;
                    case VK_OEM_3:      win32_process_key(&keyboard->keys[KEY_TILDE],     is_down, was_down); break;
                    case VK_OEM_PLUS:   win32_process_key(&keyboard->keys[KEY_EQUAL],     is_down, was_down); break;
                    case VK_OEM_MINUS:  win32_process_key(&keyboard->keys[KEY_HYPHEN],    is_down, was_down); break;

                    case VK_SPACE:      win32_process_key(&keyboard->keys[KEY_SPACE],     is_down, was_down); break;
                    case VK_RETURN:     win32_process_key(&keyboard->keys[KEY_ENTER],     is_down, was_down); break;
                    case VK_ESCAPE:     win32_process_key(&keyboard->keys[KEY_ESCAPE],    is_down, was_down); break;
                    case VK_BACK:       win32_process_key(&keyboard->keys[KEY_BACKSPACE], is_down, was_down); break;
                    case VK_TAB:        win32_process_key(&keyboard->keys[KEY_TAB],       is_down, was_down); break;
                    case VK_LSHIFT:     win32_process_key(&keyboard->keys[KEY_LSHIFT],    is_down, was_down); break;
                    case VK_RSHIFT:     win32_process_key(&keyboard->keys[KEY_RSHIFT],    is_down, was_down); break;
                    case VK_LCONTROL:   win32_process_key(&keyboard->keys[KEY_LCONTROL],  is_down, was_down); break;
                    case VK_RCONTROL:   win32_process_key(&keyboard->keys[KEY_RCONTROL],  is_down, was_down); break;
                    case VK_LMENU:      win32_process_key(&keyboard->keys[KEY_LALT],      is_down, was_down); break;
                    case VK_RMENU:      win32_process_key(&keyboard->keys[KEY_RALT],      is_down, was_down); break;
                    case VK_LWIN:       win32_process_key(&keyboard->keys[KEY_LSYSTEM],   is_down, was_down); break;
                    case VK_RWIN:       win32_process_key(&keyboard->keys[KEY_RSYSTEM],   is_down, was_down); break;
                    case VK_APPS:       win32_process_key(&keyboard->keys[KEY_MENU],      is_down, was_down); break;
                    case VK_PRIOR:      win32_process_key(&keyboard->keys[KEY_PAGEUP],    is_down, was_down); break;
                    case VK_NEXT:       win32_process_key(&keyboard->keys[KEY_PAGEDOWN],  is_down, was_down); break;
                    case VK_END:        win32_process_key(&keyboard->keys[KEY_END],       is_down, was_down); break;
                    case VK_HOME:       win32_process_key(&keyboard->keys[KEY_HOME],      is_down, was_down); break;
                    case VK_INSERT:     win32_process_key(&keyboard->keys[KEY_INSERT],    is_down, was_down); break;
                    case VK_DELETE:     win32_process_key(&keyboard->keys[KEY_DEL],       is_down, was_down); break;
                    case VK_UP:         win32_process_key(&keyboard->keys[KEY_UP],        is_down, was_down); break;
                    case VK_DOWN:       win32_process_key(&keyboard->keys[KEY_DOWN],      is_down, was_down); break;
                    case VK_LEFT:       win32_process_key(&keyboard->keys[KEY_LEFT],      is_down, was_down); break;
                    case VK_RIGHT:      win32_process_key(&keyboard->keys[KEY_RIGHT],     is_down, was_down); break;

                    case VK_NUMPAD0:    win32_process_key(&keyboard->keys[KEY_NUMPAD0], is_down, was_down); break;
                    case VK_NUMPAD1:    win32_process_key(&keyboard->keys[KEY_NUMPAD1], is_down, was_down); break;
                    case VK_NUMPAD2:    win32_process_key(&keyboard->keys[KEY_NUMPAD2], is_down, was_down); break;
                    case VK_NUMPAD3:    win32_process_key(&keyboard->keys[KEY_NUMPAD3], is_down, was_down); break;
                    case VK_NUMPAD4:    win32_process_key(&keyboard->keys[KEY_NUMPAD4], is_down, was_down); break;
                    case VK_NUMPAD5:    win32_process_key(&keyboard->keys[KEY_NUMPAD5], is_down, was_down); break;
                    case VK_NUMPAD6:    win32_process_key(&keyboard->keys[KEY_NUMPAD6], is_down, was_down); break;
                    case VK_NUMPAD7:    win32_process_key(&keyboard->keys[KEY_NUMPAD7], is_down, was_down); break;
                    case VK_NUMPAD8:    win32_process_key(&keyboard->keys[KEY_NUMPAD8], is_down, was_down); break;
                    case VK_NUMPAD9:    win32_process_key(&keyboard->keys[KEY_NUMPAD9], is_down, was_down); break;

                    case VK_F1:         win32_process_key(&keyboard->keys[KEY_F1],  is_down, was_down); break;
                    case VK_F2:         win32_process_key(&keyboard->keys[KEY_F2],  is_down, was_down); break;
                    case VK_F3:         win32_process_key(&keyboard->keys[KEY_F3],  is_down, was_down); break;
                    case VK_F4:         win32_process_key(&keyboard->keys[KEY_F4],  is_down, was_down); break;
                    case VK_F5:         win32_process_key(&keyboard->keys[KEY_F5],  is_down, was_down); break;
                    case VK_F6:         win32_process_key(&keyboard->keys[KEY_F6],  is_down, was_down); break;
                    case VK_F7:         win32_process_key(&keyboard->keys[KEY_F7],  is_down, was_down); break;
                    case VK_F8:         win32_process_key(&keyboard->keys[KEY_F8],  is_down, was_down); break;
                    case VK_F9:         win32_process_key(&keyboard->keys[KEY_F9],  is_down, was_down); break;
                    case VK_F10:        win32_process_key(&keyboard->keys[KEY_F10], is_down, was_down); break;
                    case VK_F11:        win32_process_key(&keyboard->keys[KEY_F11], is_down, was_down); break;
                    case VK_F12:        win32_process_key(&keyboard->keys[KEY_F12], is_down, was_down); break;
                }

                if (is_down)
                {
                    // Handle Alt+F4 closing window
                    // NOTE(lucas): 29th bit is context (here, whether Alt is down)
                    b32 alt_key_down = msg.lParam & (1 << 29);
                    if ((virtual_key_code == VK_F4) && alt_key_down)
                        PostQuitMessage(0);

                    // NOTE(lucas): Fullscreen key set to F11 for now,
                    // but probably want ot make this configurable in the future
                    // Also want to allow key combos like ALT+ENTER
                    if (virtual_key_code == VK_F11)
                    {
                        if (msg.hwnd)
                            win32_toggle_fullscreen(msg.hwnd);
                    }
                }

                TranslateMessage(&msg);
            } break;

            case WM_QUIT:
            {
                window->open = false;
                DestroyWindow(window->ptr);
            } break;

            case WM_CHAR:
            {
                keyboard->current_char = msg.wParam;
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
                DispatchMessageA(&msg);
            } break;
        }
    }
}

void cursor_show(b32 show)
{
    ShowCursor(show);
}

void cursor_set_from_system(CursorType type)
{
    // NOTE(lucas): Horizontal, left, and right are all the same on Windows.
    // Linux has single arrow cursors for each direction.
    // The same goes for vertical, up, and down and the diagonals.
    HCURSOR cursor = {0};
    switch(type)
    {
        case CURSOR_ARROW:                      cursor = LoadCursorA(NULL, IDC_ARROW);       break;
        case CURSOR_ARROW_WAIT:                 cursor = LoadCursorA(NULL, IDC_APPSTARTING); break;
        case CURSOR_WAIT:                       cursor = LoadCursorA(NULL, IDC_WAIT);        break;
        case CURSOR_TEXT:                       cursor = LoadCursorA(NULL, IDC_IBEAM);       break;
        case CURSOR_HAND:                       cursor = LoadCursorA(NULL, IDC_HAND);        break;
        case CURSOR_SIZE_HORIZONTAL:            cursor = LoadCursorA(NULL, IDC_SIZEWE);      break;
        case CURSOR_SIZE_VERTICAL:              cursor = LoadCursorA(NULL, IDC_SIZENS);      break;
        case CURSOR_SIZE_TOP_LEFT_BOTTOM_RIGHT: cursor = LoadCursorA(NULL, IDC_SIZENWSE);    break;
        case CURSOR_SIZE_TOP_RIGHT_BOTTOM_LEFT: cursor = LoadCursorA(NULL, IDC_SIZENESW);    break;
        case CURSOR_SIZE_LEFT:                  cursor = LoadCursorA(NULL, IDC_SIZEWE);      break;
        case CURSOR_SIZE_RIGHT:                 cursor = LoadCursorA(NULL, IDC_SIZEWE);      break;
        case CURSOR_SIZE_TOP:                   cursor = LoadCursorA(NULL, IDC_SIZENS);      break;
        case CURSOR_SIZE_BOTTOM:                cursor = LoadCursorA(NULL, IDC_SIZENS);      break;
        case CURSOR_SIZE_TOP_LEFT:              cursor = LoadCursorA(NULL, IDC_SIZENWSE);    break;
        case CURSOR_SIZE_TOP_RIGHT:             cursor = LoadCursorA(NULL, IDC_SIZENESW);    break;
        case CURSOR_SIZE_BOTTOM_LEFT:           cursor = LoadCursorA(NULL, IDC_SIZENESW);    break;
        case CURSOR_SIZE_BOTTOM_RIGHT:          cursor = LoadCursorA(NULL, IDC_SIZENWSE);    break;
        case CURSOR_SIZE_ALL:                   cursor = LoadCursorA(NULL, IDC_SIZEALL);     break;
        case CURSOR_CROSS:                      cursor = LoadCursorA(NULL, IDC_CROSS);       break;
        case CURSOR_HELP:                       cursor = LoadCursorA(NULL, IDC_HELP);        break;
        case CURSOR_NOT_ALLOWED:                cursor = LoadCursorA(NULL, IDC_NO);          break;

        // TODO(lucas): Logging, invalid type value
        default: break;
    }
    SetCursor(cursor);
}

void* cursor_load_from_file(const char* filename)
{
    HCURSOR cursor = LoadImageA(NULL, filename, IMAGE_CURSOR, 0, 0, LR_LOADFROMFILE|LR_DEFAULTSIZE);
    return cursor;
}

void cursor_set_from_memory(void* cursor)
{
    SetCursor(cursor);
}

internal void win32_xinput_button_release(ButtonState* button)
{
    button->released = true;
    button->pressed  = false; 
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
            case VK_PAD_A: gamepad->a_button.pressed = true; break;
            case VK_PAD_B: gamepad->b_button.pressed = true; break;
            case VK_PAD_X: gamepad->x_button.pressed = true; break;
            case VK_PAD_Y: gamepad->y_button.pressed = true; break;

            case VK_PAD_LSHOULDER: gamepad->left_shoulder.pressed  = true; break;
            case VK_PAD_RSHOULDER: gamepad->right_shoulder.pressed = true; break;
            case VK_PAD_LTRIGGER:  gamepad->left_trigger.pressed   = true; break;
            case VK_PAD_RTRIGGER:  gamepad->right_trigger.pressed  = true; break;

            case VK_PAD_DPAD_UP:    gamepad->dpad_up.pressed    = true; break;
            case VK_PAD_DPAD_DOWN:  gamepad->dpad_down.pressed  = true; break;
            case VK_PAD_DPAD_LEFT:  gamepad->dpad_left.pressed  = true; break;
            case VK_PAD_DPAD_RIGHT: gamepad->dpad_right.pressed = true; break;

            case VK_PAD_START: gamepad->start_button.pressed = true; break;
            case VK_PAD_BACK:  gamepad->back_button.pressed  = true; break;

            case VK_PAD_LTHUMB_PRESS:     gamepad->left_stick_press.pressed     = true; break;
            case VK_PAD_LTHUMB_UP:        gamepad->left_stick_up.pressed        = true; break;
            case VK_PAD_LTHUMB_DOWN:      gamepad->left_stick_down.pressed      = true; break;
            case VK_PAD_LTHUMB_LEFT:      gamepad->left_stick_left.pressed      = true; break;
            case VK_PAD_LTHUMB_RIGHT:     gamepad->left_stick_right.pressed     = true; break;
            case VK_PAD_LTHUMB_UPLEFT:    gamepad->left_stick_upleft.pressed    = true; break;
            case VK_PAD_LTHUMB_UPRIGHT:   gamepad->left_stick_upright.pressed   = true; break;
            case VK_PAD_LTHUMB_DOWNLEFT:  gamepad->left_stick_downleft.pressed  = true; break;
            case VK_PAD_LTHUMB_DOWNRIGHT: gamepad->left_stick_downright.pressed = true; break;

            case VK_PAD_RTHUMB_PRESS:     gamepad->right_stick_press.pressed     = true; break;
            case VK_PAD_RTHUMB_UP:        gamepad->right_stick_up.pressed        = true; break;
            case VK_PAD_RTHUMB_DOWN:      gamepad->right_stick_down.pressed      = true; break;
            case VK_PAD_RTHUMB_LEFT:      gamepad->right_stick_left.pressed      = true; break;
            case VK_PAD_RTHUMB_RIGHT:     gamepad->right_stick_right.pressed     = true; break;
            case VK_PAD_RTHUMB_UPLEFT:    gamepad->right_stick_upleft.pressed    = true; break;
            case VK_PAD_RTHUMB_UPRIGHT:   gamepad->right_stick_upright.pressed   = true; break;
            case VK_PAD_RTHUMB_DOWNLEFT:  gamepad->right_stick_downleft.pressed  = true; break;
            case VK_PAD_RTHUMB_DOWNRIGHT: gamepad->right_stick_downright.pressed = true; break;

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

            case VK_PAD_LSHOULDER: win32_xinput_button_release(&gamepad->left_shoulder);  break;
            case VK_PAD_RSHOULDER: win32_xinput_button_release(&gamepad->right_shoulder); break;
            case VK_PAD_LTRIGGER:  win32_xinput_button_release(&gamepad->left_trigger);   break;
            case VK_PAD_RTRIGGER:  win32_xinput_button_release(&gamepad->right_trigger);  break;

            case VK_PAD_DPAD_UP:    win32_xinput_button_release(&gamepad->dpad_up);    break;
            case VK_PAD_DPAD_DOWN:  win32_xinput_button_release(&gamepad->dpad_down);  break;
            case VK_PAD_DPAD_LEFT:  win32_xinput_button_release(&gamepad->dpad_left);  break;
            case VK_PAD_DPAD_RIGHT: win32_xinput_button_release(&gamepad->dpad_right); break;

            case VK_PAD_START: win32_xinput_button_release(&gamepad->start_button); break;
            case VK_PAD_BACK:  win32_xinput_button_release(&gamepad->back_button);  break;

            case VK_PAD_LTHUMB_PRESS:     win32_xinput_button_release(&gamepad->left_stick_press);     break;
            case VK_PAD_LTHUMB_UP:        win32_xinput_button_release(&gamepad->left_stick_up);        break;
            case VK_PAD_LTHUMB_DOWN:      win32_xinput_button_release(&gamepad->left_stick_down);      break;
            case VK_PAD_LTHUMB_LEFT:      win32_xinput_button_release(&gamepad->left_stick_left);      break;
            case VK_PAD_LTHUMB_RIGHT:     win32_xinput_button_release(&gamepad->left_stick_right);     break;
            case VK_PAD_LTHUMB_UPLEFT:    win32_xinput_button_release(&gamepad->left_stick_upleft);    break;
            case VK_PAD_LTHUMB_UPRIGHT:   win32_xinput_button_release(&gamepad->left_stick_upright);   break;
            case VK_PAD_LTHUMB_DOWNLEFT:  win32_xinput_button_release(&gamepad->left_stick_downleft);  break;
            case VK_PAD_LTHUMB_DOWNRIGHT: win32_xinput_button_release(&gamepad->left_stick_downright); break;

            case VK_PAD_RTHUMB_PRESS:     win32_xinput_button_release(&gamepad->right_stick_press);     break;
            case VK_PAD_RTHUMB_UP:        win32_xinput_button_release(&gamepad->right_stick_up);        break;
            case VK_PAD_RTHUMB_DOWN:      win32_xinput_button_release(&gamepad->right_stick_down);      break;
            case VK_PAD_RTHUMB_LEFT:      win32_xinput_button_release(&gamepad->right_stick_left);      break;
            case VK_PAD_RTHUMB_RIGHT:     win32_xinput_button_release(&gamepad->right_stick_right);     break;
            case VK_PAD_RTHUMB_UPLEFT:    win32_xinput_button_release(&gamepad->right_stick_upleft);    break;
            case VK_PAD_RTHUMB_UPRIGHT:   win32_xinput_button_release(&gamepad->right_stick_upright);   break;
            case VK_PAD_RTHUMB_DOWNLEFT:  win32_xinput_button_release(&gamepad->right_stick_downleft);  break;
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

void win32_xinput_gamepad_process_input(Input* input)
{
    Gamepad* gamepad = {0};
    for (int i = 0; i < MAX_GAMEPADS; ++i)
    {
        gamepad = &input->gamepads[i];

        // Release state should not persist, so make sure it is false for each button
        for (int button_index = 0; button_index < ARRAY_COUNT(gamepad->buttons); button_index++)
            gamepad->buttons[button_index].released = false;

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
        gamepad->left_stick_y = win32_process_xinput_stick(xinput_gamepad.sThumbLY,
                                                            XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        gamepad->right_stick_x = win32_process_xinput_stick(xinput_gamepad.sThumbRX,
                                                            XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        gamepad->right_stick_y = win32_process_xinput_stick(xinput_gamepad.sThumbRY,
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

void input_process(Window* window, Input* input)
{
    win32_keyboard_mouse_process_input(window, input);
    win32_xinput_gamepad_process_input(input);
}

b32 clipboard_write_string(char* text)
{
    // Before clipboard can be written to, it first needs to be opened and emptied
    HWND window = GetActiveWindow();
    if (!OpenClipboard(window))
        return false;
    EmptyClipboard();

    // Allocate global memory for the text
    int string_length = lstrlenA(text);
    HGLOBAL string_handle = GlobalAlloc(GMEM_MOVEABLE, string_length+1);
    if (string_handle == NULL)
    {
        CloseClipboard();
        return false;
    }
    LPSTR string_copy = GlobalLock(string_handle);
    memcpy(string_copy, text, string_length+1);
    string_copy[string_length+1] = '\0';
    GlobalUnlock(string_handle);

    // Write to clipboard and close
    SetClipboardData(CF_TEXT, string_handle);
    CloseClipboard();
    return true;
}

char* clipboard_read_string(void)
{
    char* result = NULL;

    // Before clipboard can be written to, it first needs to be opened and emptied
    // Also, check that the desired format is available
    HWND window = GetActiveWindow();
    if (!IsClipboardFormatAvailable(CF_TEXT))
        return NULL;
    if (!OpenClipboard(window))
        return NULL;

    // Get clipboard data
    HGLOBAL clipboard_handle = GetClipboardData(CF_TEXT);
    LPCSTR clipboard_string = NULL;
    if (clipboard_handle != NULL)
    {
        // Get string from clipboard data
        clipboard_string = GlobalLock(clipboard_handle);
        if (clipboard_string != NULL)
        {
            // If string is not NULL, allocate a new one to return and copy
            usize string_length = lstrlenA(clipboard_string)+1;
            result = calloc(string_length, sizeof(TCHAR));
            memcpy(result, clipboard_string, string_length);
            result[string_length+1] = '\0';

            GlobalUnlock(clipboard_handle);
        }
    }

    CloseClipboard();
    return result;
}
