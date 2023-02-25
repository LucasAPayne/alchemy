#include "win32_keyboard.h"
#include "keyboard.h"
#include "types.h"

#include <windows.h>

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

internal void win32_process_key(key_state* key, bool32 is_down, bool32 was_down)
{
    key->is_pressed = is_down;
    key->is_released = was_down && !is_down;
}

// TODO(lucas): Support for non-US keyboard layouts
void win32_process_keyboard_input(HWND window, keyboard_input* key_input)
{
    // Release state should not persist, so make sure it is false for each button
    for (int key = 0; key < ARRAY_COUNT(key_input->keys); key++)
        key_input->keys[key].is_released = false;

    MSG msg;
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        switch(msg.message)
        {
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
                bool32 was_down = (msg.lParam & (1 << 30)) != 0; // 30th bit is previous state (1 for down, 0 for up)
                bool32 is_down = (msg.lParam & (1 << 31)) == 0; // 31st bit is transition, always 1 for keyup, 0 for keydown

                // Disregard key repeats
                if (was_down == is_down)
                    break;

                // NOTE(lucas): Most keycodes map directly to their ANSI equivalent (letters are capital)
                // For keys that have no ANSI equivalent, Windows provides defines for virtual keycodes
                switch(virtual_key_code)
                {
                    case 'A': win32_process_key(&key_input->keys[key::A], is_down, was_down); break;
                    case 'B': win32_process_key(&key_input->keys[key::B], is_down, was_down); break;
                    case 'C': win32_process_key(&key_input->keys[key::C], is_down, was_down); break;
                    case 'D': win32_process_key(&key_input->keys[key::D], is_down, was_down); break;
                    case 'E': win32_process_key(&key_input->keys[key::E], is_down, was_down); break;
                    case 'F': win32_process_key(&key_input->keys[key::F], is_down, was_down); break;
                    case 'G': win32_process_key(&key_input->keys[key::G], is_down, was_down); break;
                    case 'H': win32_process_key(&key_input->keys[key::H], is_down, was_down); break;
                    case 'I': win32_process_key(&key_input->keys[key::I], is_down, was_down); break;
                    case 'J': win32_process_key(&key_input->keys[key::J], is_down, was_down); break;
                    case 'K': win32_process_key(&key_input->keys[key::K], is_down, was_down); break;
                    case 'L': win32_process_key(&key_input->keys[key::L], is_down, was_down); break;
                    case 'M': win32_process_key(&key_input->keys[key::M], is_down, was_down); break;
                    case 'N': win32_process_key(&key_input->keys[key::N], is_down, was_down); break;
                    case 'O': win32_process_key(&key_input->keys[key::O], is_down, was_down); break;
                    case 'P': win32_process_key(&key_input->keys[key::P], is_down, was_down); break;
                    case 'Q': win32_process_key(&key_input->keys[key::Q], is_down, was_down); break;
                    case 'R': win32_process_key(&key_input->keys[key::R], is_down, was_down); break;
                    case 'S': win32_process_key(&key_input->keys[key::S], is_down, was_down); break;
                    case 'T': win32_process_key(&key_input->keys[key::T], is_down, was_down); break;
                    case 'U': win32_process_key(&key_input->keys[key::U], is_down, was_down); break;
                    case 'V': win32_process_key(&key_input->keys[key::V], is_down, was_down); break;
                    case 'W': win32_process_key(&key_input->keys[key::W], is_down, was_down); break;
                    case 'X': win32_process_key(&key_input->keys[key::X], is_down, was_down); break;
                    case 'Y': win32_process_key(&key_input->keys[key::Y], is_down, was_down); break;
                    case 'Z': win32_process_key(&key_input->keys[key::Z], is_down, was_down); break;

                    case '0': win32_process_key(&key_input->keys[key::NUM0], is_down, was_down); break;
                    case '1': win32_process_key(&key_input->keys[key::NUM1], is_down, was_down); break;
                    case '2': win32_process_key(&key_input->keys[key::NUM2], is_down, was_down); break;
                    case '3': win32_process_key(&key_input->keys[key::NUM3], is_down, was_down); break;
                    case '4': win32_process_key(&key_input->keys[key::NUM4], is_down, was_down); break;
                    case '5': win32_process_key(&key_input->keys[key::NUM5], is_down, was_down); break;
                    case '6': win32_process_key(&key_input->keys[key::NUM6], is_down, was_down); break;
                    case '7': win32_process_key(&key_input->keys[key::NUM7], is_down, was_down); break;
                    case '8': win32_process_key(&key_input->keys[key::NUM8], is_down, was_down); break;
                    case '9': win32_process_key(&key_input->keys[key::NUM9], is_down, was_down); break;

                    case VK_OEM_4:      win32_process_key(&key_input->keys[key::LBRACKET],  is_down, was_down); break;
                    case VK_OEM_6:      win32_process_key(&key_input->keys[key::RBRACKET],  is_down, was_down); break;
                    case VK_OEM_1:      win32_process_key(&key_input->keys[key::SEMICOLON], is_down, was_down); break;
                    case VK_OEM_7:      win32_process_key(&key_input->keys[key::QUOTE],     is_down, was_down); break;
                    case VK_OEM_COMMA:  win32_process_key(&key_input->keys[key::COMMA],     is_down, was_down); break;
                    case VK_OEM_PERIOD: win32_process_key(&key_input->keys[key::PERIOD],    is_down, was_down); break;
                    case VK_OEM_2:      win32_process_key(&key_input->keys[key::SLASH],     is_down, was_down); break;
                    case VK_OEM_5:      win32_process_key(&key_input->keys[key::BACKSLASH], is_down, was_down); break;
                    case VK_OEM_3:      win32_process_key(&key_input->keys[key::TILDE],     is_down, was_down); break;
                    case VK_OEM_PLUS:   win32_process_key(&key_input->keys[key::EQUAL],     is_down, was_down); break;
                    case VK_OEM_MINUS:  win32_process_key(&key_input->keys[key::HYPHEN],    is_down, was_down); break;

                    case VK_SPACE:      win32_process_key(&key_input->keys[key::SPACE],     is_down, was_down); break;
                    case VK_RETURN:     win32_process_key(&key_input->keys[key::ENTER],     is_down, was_down); break;
                    case VK_ESCAPE:     win32_process_key(&key_input->keys[key::ESCAPE],    is_down, was_down); break;
                    case VK_BACK:       win32_process_key(&key_input->keys[key::BACKSPACE], is_down, was_down); break;
                    case VK_TAB:        win32_process_key(&key_input->keys[key::TAB],       is_down, was_down); break;
                    case VK_LSHIFT:     win32_process_key(&key_input->keys[key::LSHIFT],    is_down, was_down); break;
                    case VK_RSHIFT:     win32_process_key(&key_input->keys[key::RSHIFT],    is_down, was_down); break;
                    case VK_LCONTROL:   win32_process_key(&key_input->keys[key::LCONTROL],  is_down, was_down); break;
                    case VK_RCONTROL:   win32_process_key(&key_input->keys[key::RCONTROL],  is_down, was_down); break;
                    case VK_LMENU:      win32_process_key(&key_input->keys[key::LALT],      is_down, was_down); break;
                    case VK_RMENU:      win32_process_key(&key_input->keys[key::RALT],      is_down, was_down); break;
                    case VK_LWIN:       win32_process_key(&key_input->keys[key::LSYSTEM],   is_down, was_down); break;
                    case VK_RWIN:       win32_process_key(&key_input->keys[key::RSYSTEM],   is_down, was_down); break;
                    case VK_APPS:       win32_process_key(&key_input->keys[key::MENU],      is_down, was_down); break;
                    case VK_PRIOR:      win32_process_key(&key_input->keys[key::PAGEUP],    is_down, was_down); break;
                    case VK_NEXT:       win32_process_key(&key_input->keys[key::PAGEDOWN],  is_down, was_down); break;
                    case VK_END:        win32_process_key(&key_input->keys[key::END],       is_down, was_down); break;
                    case VK_HOME:       win32_process_key(&key_input->keys[key::HOME],      is_down, was_down); break;
                    case VK_INSERT:     win32_process_key(&key_input->keys[key::INSERT],    is_down, was_down); break;
                    case VK_DELETE:     win32_process_key(&key_input->keys[key::DEL],       is_down, was_down); break;
                    case VK_UP:         win32_process_key(&key_input->keys[key::UP],        is_down, was_down); break;
                    case VK_DOWN:       win32_process_key(&key_input->keys[key::DOWN],      is_down, was_down); break;
                    case VK_LEFT:       win32_process_key(&key_input->keys[key::LEFT],      is_down, was_down); break;
                    case VK_RIGHT:      win32_process_key(&key_input->keys[key::RIGHT],     is_down, was_down); break;

                    case VK_NUMPAD0:    win32_process_key(&key_input->keys[key::NUMPAD0], is_down, was_down); break;
                    case VK_NUMPAD1:    win32_process_key(&key_input->keys[key::NUMPAD1], is_down, was_down); break;
                    case VK_NUMPAD2:    win32_process_key(&key_input->keys[key::NUMPAD2], is_down, was_down); break;
                    case VK_NUMPAD3:    win32_process_key(&key_input->keys[key::NUMPAD3], is_down, was_down); break;
                    case VK_NUMPAD4:    win32_process_key(&key_input->keys[key::NUMPAD4], is_down, was_down); break;
                    case VK_NUMPAD5:    win32_process_key(&key_input->keys[key::NUMPAD5], is_down, was_down); break;
                    case VK_NUMPAD6:    win32_process_key(&key_input->keys[key::NUMPAD6], is_down, was_down); break;
                    case VK_NUMPAD7:    win32_process_key(&key_input->keys[key::NUMPAD7], is_down, was_down); break;
                    case VK_NUMPAD8:    win32_process_key(&key_input->keys[key::NUMPAD8], is_down, was_down); break;
                    case VK_NUMPAD9:    win32_process_key(&key_input->keys[key::NUMPAD9], is_down, was_down); break;

                    case VK_F1:         win32_process_key(&key_input->keys[key::F1], is_down,  was_down); break;
                    case VK_F2:         win32_process_key(&key_input->keys[key::F2], is_down,  was_down); break;
                    case VK_F3:         win32_process_key(&key_input->keys[key::F3], is_down,  was_down); break;
                    case VK_F4:         win32_process_key(&key_input->keys[key::F4], is_down,  was_down); break;
                    case VK_F5:         win32_process_key(&key_input->keys[key::F5], is_down,  was_down); break;
                    case VK_F6:         win32_process_key(&key_input->keys[key::F6], is_down,  was_down); break;
                    case VK_F7:         win32_process_key(&key_input->keys[key::F7], is_down,  was_down); break;
                    case VK_F8:         win32_process_key(&key_input->keys[key::F8], is_down,  was_down); break;
                    case VK_F9:         win32_process_key(&key_input->keys[key::F9], is_down,  was_down); break;
                    case VK_F10:        win32_process_key(&key_input->keys[key::F10], is_down, was_down); break;
                    case VK_F11:        win32_process_key(&key_input->keys[key::F11], is_down, was_down); break;
                    case VK_F12:        win32_process_key(&key_input->keys[key::F12], is_down, was_down); break;
                }

                // Handle Alt+F4 closing window
                // NOTE(lucas): 29th bit is context (here, whether Alt is down)
                bool32 alt_key_down = msg.lParam & (1 << 29);
                if ((virtual_key_code == VK_F4) && alt_key_down)
                    PostQuitMessage(0);
            } break;

            default:
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } break;
        }
    }
}
