#include "win32_input.h"
#include "input.h"
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
    WORD scancode = LOBYTE(key_flags); // scancode
    BOOL extended = (key_flags & KF_EXTENDED) == KF_EXTENDED; // extended-key flag, 1 if scancode has 0xE0 prefix
    
    if (extended)
        scancode = MAKEWORD(scancode, 0xE0);

    switch (vk)
    {
        case VK_SHIFT:   // converts to VK_LSHIFT or VK_RSHIFT
        case VK_CONTROL: // converts to VK_LCONTROL or VK_RCONTROL
        case VK_MENU:    // converts to VK_LMENU or VK_RMENU
            vk = LOWORD(MapVirtualKeyW(scancode, MAPVK_VSC_TO_VK_EX));
            break;

        default: break;
    }

    return vk;
}

// TODO(lucas): Support for non-US keyboard layouts
void win32_process_input(HWND window, keyboard_input* key_input)
{
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
                    case 'A': key_input->keys[key::A].is_down = is_down; break;
                    case 'B': key_input->keys[key::B].is_down = is_down; break;
                    case 'C': key_input->keys[key::C].is_down = is_down; break;
                    case 'D': key_input->keys[key::D].is_down = is_down; break;
                    case 'E': key_input->keys[key::E].is_down = is_down; break;
                    case 'F': key_input->keys[key::F].is_down = is_down; break;
                    case 'G': key_input->keys[key::G].is_down = is_down; break;
                    case 'H': key_input->keys[key::H].is_down = is_down; break;
                    case 'I': key_input->keys[key::I].is_down = is_down; break;
                    case 'J': key_input->keys[key::J].is_down = is_down; break;
                    case 'K': key_input->keys[key::K].is_down = is_down; break;
                    case 'L': key_input->keys[key::L].is_down = is_down; break;
                    case 'M': key_input->keys[key::M].is_down = is_down; break;
                    case 'N': key_input->keys[key::N].is_down = is_down; break;
                    case 'O': key_input->keys[key::O].is_down = is_down; break;
                    case 'P': key_input->keys[key::P].is_down = is_down; break;
                    case 'Q': key_input->keys[key::Q].is_down = is_down; break;
                    case 'R': key_input->keys[key::R].is_down = is_down; break;
                    case 'S': key_input->keys[key::S].is_down = is_down; break;
                    case 'T': key_input->keys[key::T].is_down = is_down; break;
                    case 'U': key_input->keys[key::U].is_down = is_down; break;
                    case 'V': key_input->keys[key::V].is_down = is_down; break;
                    case 'W': key_input->keys[key::W].is_down = is_down; break;
                    case 'X': key_input->keys[key::X].is_down = is_down; break;
                    case 'Y': key_input->keys[key::Y].is_down = is_down; break;
                    case 'Z': key_input->keys[key::Z].is_down = is_down; break;

                    case '0': key_input->keys[key::NUM0].is_down = is_down; break;
                    case '1': key_input->keys[key::NUM1].is_down = is_down; break;
                    case '2': key_input->keys[key::NUM2].is_down = is_down; break;
                    case '3': key_input->keys[key::NUM3].is_down = is_down; break;
                    case '4': key_input->keys[key::NUM4].is_down = is_down; break;
                    case '5': key_input->keys[key::NUM5].is_down = is_down; break;
                    case '6': key_input->keys[key::NUM6].is_down = is_down; break;
                    case '7': key_input->keys[key::NUM7].is_down = is_down; break;
                    case '8': key_input->keys[key::NUM8].is_down = is_down; break;
                    case '9': key_input->keys[key::NUM9].is_down = is_down; break;

                    case VK_OEM_4:      key_input->keys[key::LBRACKET].is_down  = is_down; break;
                    case VK_OEM_6:      key_input->keys[key::RBRACKET].is_down  = is_down; break;
                    case VK_OEM_1:      key_input->keys[key::SEMICOLON].is_down = is_down; break;
                    case VK_OEM_7:      key_input->keys[key::QUOTE].is_down     = is_down; break;
                    case VK_OEM_COMMA:  key_input->keys[key::COMMA].is_down     = is_down; break;
                    case VK_OEM_PERIOD: key_input->keys[key::PERIOD].is_down    = is_down; break;
                    case VK_OEM_2:      key_input->keys[key::SLASH].is_down     = is_down; break;
                    case VK_OEM_5:      key_input->keys[key::BACKSLASH].is_down = is_down; break;
                    case VK_OEM_3:      key_input->keys[key::TILDE].is_down     = is_down; break;
                    case VK_OEM_PLUS:   key_input->keys[key::EQUAL].is_down     = is_down; break;
                    case VK_OEM_MINUS:  key_input->keys[key::HYPHEN].is_down    = is_down; break;

                    case VK_SPACE: key_input->keys[key::SPACE].is_down       = is_down; break;
                    case VK_RETURN: key_input->keys[key::ENTER].is_down      = is_down; break;
                    case VK_ESCAPE: key_input->keys[key::ESCAPE].is_down     = is_down; break;
                    case VK_BACK: key_input->keys[key::BACKSPACE].is_down    = is_down; break;
                    case VK_TAB: key_input->keys[key::TAB].is_down           = is_down; break;
                    case VK_LSHIFT: key_input->keys[key::LSHIFT].is_down     = is_down; break;
                    case VK_RSHIFT: key_input->keys[key::RSHIFT].is_down     = is_down; break;
                    case VK_LCONTROL: key_input->keys[key::LCONTROL].is_down = is_down; break;
                    case VK_RCONTROL: key_input->keys[key::RCONTROL].is_down = is_down; break;
                    case VK_LMENU: key_input->keys[key::LALT].is_down        = is_down; break;
                    case VK_RMENU: key_input->keys[key::RALT].is_down        = is_down; break;
                    case VK_LWIN: key_input->keys[key::LSYSTEM].is_down      = is_down; break;
                    case VK_RWIN: key_input->keys[key::RSYSTEM].is_down      = is_down; break;
                    case VK_APPS: key_input->keys[key::MENU].is_down         = is_down; break;
                    case VK_PRIOR: key_input->keys[key::PAGEUP].is_down      = is_down; break;
                    case VK_NEXT: key_input->keys[key::PAGEDOWN].is_down     = is_down; break;
                    case VK_END: key_input->keys[key::END].is_down           = is_down; break;
                    case VK_HOME: key_input->keys[key::HOME].is_down         = is_down; break;
                    case VK_INSERT: key_input->keys[key::INSERT].is_down     = is_down; break;
                    case VK_DELETE: key_input->keys[key::DEL].is_down        = is_down; break;
                    case VK_UP: key_input->keys[key::UP].is_down             = is_down; break;
                    case VK_DOWN: key_input->keys[key::DOWN].is_down         = is_down; break;
                    case VK_LEFT: key_input->keys[key::LEFT].is_down         = is_down; break;
                    case VK_RIGHT: key_input->keys[key::RIGHT].is_down       = is_down; break;

                    case VK_NUMPAD0: key_input->keys[key::NUMPAD0].is_down = is_down; break;
                    case VK_NUMPAD1: key_input->keys[key::NUMPAD1].is_down = is_down; break;
                    case VK_NUMPAD2: key_input->keys[key::NUMPAD2].is_down = is_down; break;
                    case VK_NUMPAD3: key_input->keys[key::NUMPAD3].is_down = is_down; break;
                    case VK_NUMPAD4: key_input->keys[key::NUMPAD4].is_down = is_down; break;
                    case VK_NUMPAD5: key_input->keys[key::NUMPAD5].is_down = is_down; break;
                    case VK_NUMPAD6: key_input->keys[key::NUMPAD6].is_down = is_down; break;
                    case VK_NUMPAD7: key_input->keys[key::NUMPAD7].is_down = is_down; break;
                    case VK_NUMPAD8: key_input->keys[key::NUMPAD8].is_down = is_down; break;
                    case VK_NUMPAD9: key_input->keys[key::NUMPAD9].is_down = is_down; break;

                    case VK_F1: key_input->keys[key::F1].is_down   = is_down; break;
                    case VK_F2: key_input->keys[key::F2].is_down   = is_down; break;
                    case VK_F3: key_input->keys[key::F3].is_down   = is_down; break;
                    case VK_F4: key_input->keys[key::F4].is_down   = is_down; break;
                    case VK_F5: key_input->keys[key::F5].is_down   = is_down; break;
                    case VK_F6: key_input->keys[key::F6].is_down   = is_down; break;
                    case VK_F7: key_input->keys[key::F7].is_down   = is_down; break;
                    case VK_F8: key_input->keys[key::F8].is_down   = is_down; break;
                    case VK_F9: key_input->keys[key::F9].is_down   = is_down; break;
                    case VK_F10: key_input->keys[key::F10].is_down = is_down; break;
                    case VK_F11: key_input->keys[key::F11].is_down = is_down; break;
                    case VK_F12: key_input->keys[key::F12].is_down = is_down; break;
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
