#include "alchemy/window.h"
#include "alchemy/util/log.h"
#include "alchemy/util/types.h"

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

internal inline i64 win32_get_ticks(void)
{
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);
    return ticks.QuadPart;
}

f32 get_frame_seconds(Window* window)
{
    i64 start_ticks = window->_prev_frame_ticks;
    i64 end_ticks = win32_get_ticks();
    i64 microseconds_elapsed = (end_ticks - start_ticks);

	// We now have the elapsed number of ticks, along with the
	// number of ticks-per-second. We use these values
	// to convert to the number of elapsed microseconds.
	// To guard against loss-of-precision, we convert
	// to microseconds *before* dividing by ticks-per-second.
	microseconds_elapsed *= 1000000;
	microseconds_elapsed /= window->_ticks_per_second;
    
    f32 seconds_elapsed = (f32)microseconds_elapsed / 1000000.0f;
    if (seconds_elapsed < 0.0f)
        seconds_elapsed = 0.0f;
    
    window->_prev_frame_ticks = win32_get_ticks();
    return seconds_elapsed;
}

internal void window_update_size(Window* window)
{
    RECT client_rect = {0};
    GetClientRect(window->ptr, &client_rect);
    window->width = client_rect.right - client_rect.left;
    window->height = client_rect.bottom - client_rect.top;
}

internal LRESULT CALLBACK win32_main_window_callback(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;

    switch(msg)
    {
        case WM_SIZE:
        {
            Window* window = (Window*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
         
            // WM_SIZE is called on window creation, before the window data gets associated with the hwnd.
            if (window)
                window_update_size(window);
        } break;

        /*
        Sent when the window/application should close (e.g., user clicks X button).
        */
        case WM_CLOSE:
        {
            Window* window = (Window*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
            if (window)
                window->open = false;

            DestroyWindow(hwnd);
        } break;

        /*
        Sent when a window is being destroyed.
        */
        case WM_DESTROY:
        {
            // TODO(lucas): Handle this with a message to the user?
            Window* window = (Window*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
            if (window)
                window->open = false;

            PostQuitMessage(0);
        } break;

        case WM_GETMINMAXINFO:
        {
            LPMINMAXINFO mmi = (LPMINMAXINFO)lparam;
            Window* window = (Window*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
            if (window)
            {
                // NOTE(lucas): LPMINMAXINFO includes border size,
                // so need to find border size and add it to desired min/max size,
                // which the user supplies as client size
                RECT client;
                RECT wind;
                POINT diff;
                GetClientRect(hwnd, &client);
                GetWindowRect(hwnd, &wind);
                diff.x = (wind.right - wind.left) - client.right;
                diff.y = (wind.bottom - wind.top) - client.bottom;

                if (window->min_width > 0)  mmi->ptMinTrackSize.x = window->min_width  + diff.x;
                if (window->min_height > 0) mmi->ptMinTrackSize.y = window->min_height + diff.y;
                if (window->max_width > 0)  mmi->ptMaxTrackSize.x = window->max_width  + diff.x;
                if (window->max_height > 0) mmi->ptMaxTrackSize.y = window->max_height + diff.y;
            }
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        {
            ASSERT(0, "Keyboard input came in through a non-dispatch message!");
        } break;

        /*
        All message types that are not explicitly handled will end up here. DefWindowProc just provides default
        processing. Having a default case ensures that every Windows message gets processed.
        */
        default:
        {
            result = DefWindowProcA(hwnd, msg, wparam, lparam);
        } break;
    }
    
    return result;
}

void window_init(Window* window, const char* title, int width, int height)
{
    window->width = width;
    window->height = height;

    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    window->_ticks_per_second = frequency.QuadPart;

    // Open a window
    HINSTANCE instance = GetModuleHandleA(0);
    WNDCLASSEXA window_class = {0};
    window_class.cbSize = sizeof(WNDCLASSEXA);
    window_class.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    window_class.lpfnWndProc = &win32_main_window_callback;
    window_class.hInstance = instance;
    window_class.hIcon = LoadIconA(0, IDI_APPLICATION);
    window_class.hIconSm = LoadIconA(0, IDI_APPLICATION);
    window_class.lpszClassName = "MyWindowClass";
    // TODO(lucas): Allow the user to set the default cursor (system or custom)
    window_class.hCursor = LoadCursorA(NULL, IDC_ARROW);
    
    if(!RegisterClassExA(&window_class))
    {
        GetLastError();
        MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
    }

    RECT initial_window_rect = {0, 0, width, height};
    AdjustWindowRectEx(&initial_window_rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
    LONG initial_window_width = initial_window_rect.right - initial_window_rect.left;
    LONG initial_window_height = initial_window_rect.bottom - initial_window_rect.top;

    HWND hwnd = CreateWindowExA(WS_EX_OVERLAPPEDWINDOW,
                            window_class.lpszClassName,
                            title,
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                            CW_USEDEFAULT, CW_USEDEFAULT,
                            initial_window_width, 
                            initial_window_height,
                            0, 0, instance, 0);

    if(!hwnd)
    {
        GetLastError();
        MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
    }

    window->ptr = hwnd;
    window->open = true;

    // Associate window data with the window ptr
    SetWindowLongPtrA(window->ptr, GWLP_USERDATA, (LONG_PTR)window);

    window->_prev_frame_ticks = win32_get_ticks();
}

void window_render(Window* window)
{
    HDC device_context = GetDC(window->ptr);
    SwapBuffers(device_context);
    ReleaseDC(window->ptr, device_context);
}

void window_set_min_size(Window* window, int min_width, int min_height)
{
    window->min_width = min_width;
    window->min_height = min_height;
}

void window_set_max_size(Window* window, int max_width, int max_height)
{
    window->max_width = max_width;
    window->max_height = max_height;
}

void* window_icon_load_from_file(const char* filename)
{
    HICON icon = LoadImageA(NULL, filename, IMAGE_ICON, 0, 0, LR_LOADFROMFILE|LR_DEFAULTSIZE);
    return icon;
}

void window_icon_set_from_memory(Window* window, void* icon)
{
    SendMessage(window->ptr, WM_SETICON, ICON_SMALL, (LPARAM)icon);
    SendMessage(window->ptr, WM_SETICON, ICON_BIG, (LPARAM)icon);
}

void console_launch(void)
{
    AllocConsole();

    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    // Enable printing colored text
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (out == INVALID_HANDLE_VALUE) return;

    DWORD mode = GetConsoleMode(out, &mode);
    if (!mode) return;

    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(out, mode);

    // Set bright blue text to a more vibrant and readable shade
    CONSOLE_SCREEN_BUFFER_INFOEX cbi = {sizeof(CONSOLE_SCREEN_BUFFER_INFOEX)};
    if (GetConsoleScreenBufferInfoEx(out, &cbi))
    {
        cbi.ColorTable[9] = RGB(0, 120, 255);
        SetConsoleScreenBufferInfoEx(out, &cbi);
    }
}
