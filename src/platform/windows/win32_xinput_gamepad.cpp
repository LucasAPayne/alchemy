#include "win32_xinput_gamepad.h"
#include "gamepad.h"

#include <windows.h>
#include <Xinput.h>

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

void win32_process_xinput_gamepad_input(Gamepad* gamepad)
{
        // Release state should not persist, so make sure it is false for each button
        for (int button_index = 0; button_index < ARRAY_COUNT(gamepad->buttons); button_index++)
            gamepad->buttons[button_index].is_released = false;

        XINPUT_STATE controller_state;

        if (XInputGetState(0, &controller_state) != ERROR_SUCCESS)
        {
            // NOTE(lucas): The controller is not available
            gamepad->is_connected = false;
            return;
        }
        gamepad->is_connected = true;

        // Buttons
        XINPUT_KEYSTROKE keystroke = {0};
        DWORD status = XInputGetKeystroke(0, 0, &keystroke);
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
