#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE

#include "example.h"
#include "input.h"
#include "util/types.h"
#include "platform/windows/win32_input.h"
#include "platform/windows/win32_sound.h"

#include <windows.h>

#include <glad/glad.h>

#include <stdio.h>

global_variable b32 global_running;

typedef struct Win32WindowDimensions
{
    int width;
    int height;
} Win32WindowDimensions;

internal Win32WindowDimensions win32_get_window_dimensions(HWND window)
{
    Win32WindowDimensions dimensions;
    RECT client_rect = {0};
    GetClientRect(window, &client_rect);
    dimensions.width = client_rect.right - client_rect.left;
    dimensions.height = client_rect.bottom - client_rect.top;
    return dimensions;
}

inline i64 win32_get_ticks(void)
{
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);
    return ticks.QuadPart;
}

inline f32 win32_get_seconds_elapsed(i64 start_ticks, i64 end_ticks, i64 ticks_per_second)
{
    i64 microseconds_elapsed = (end_ticks - start_ticks);

	// We now have the elapsed number of ticks, along with the
	// number of ticks-per-second. We use these values
	// to convert to the number of elapsed microseconds.
	// To guard against loss-of-precision, we convert
	// to microseconds *before* dividing by ticks-per-second.
	microseconds_elapsed *= 1000000;
	microseconds_elapsed /= ticks_per_second;
    
    f32 seconds_elapsed = (f32)microseconds_elapsed / 1000000.0f;
    if (seconds_elapsed < 0.0f)
        seconds_elapsed = 0.0f;
    
    return seconds_elapsed;
}

void GLAPIENTRY opengl_error_callback(GLenum source,
                                      GLenum type,
                                      GLuint id,
                                      GLenum severity,
                                      GLsizei length,
                                      const GLchar* message,
                                      const void* user_param)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    OutputDebugStringA("---------------\n");

    char info_log[512];
    snprintf(info_log, sizeof(info_log), "Debug message (%d): %s\n", id, message);
    OutputDebugStringA(info_log);

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             OutputDebugStringA("Source: API\n"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   OutputDebugStringA("Source: Window System\n"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: OutputDebugStringA("Source: Shader Compiler\n"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     OutputDebugStringA("Source: Third Party\n"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     OutputDebugStringA("Source: Application\n"); break;
        case GL_DEBUG_SOURCE_OTHER:           OutputDebugStringA("Source: Other\n"); break;
        default:                              OutputDebugStringA("Source: Unknown\n"); break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               OutputDebugStringA("Type: Error\n"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: OutputDebugStringA("Type: Deprecated Behavior\n"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  OutputDebugStringA("Type: Undefined Behavior\n"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         OutputDebugStringA("Type: Portability\n"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         OutputDebugStringA("Type: Performance\n"); break;
        case GL_DEBUG_TYPE_MARKER:              OutputDebugStringA("Type: Marker\n"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          OutputDebugStringA("Type: Push Group\n"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           OutputDebugStringA("Type: Pop Group\n"); break;
        case GL_DEBUG_TYPE_OTHER:               OutputDebugStringA("Type: Other\n"); break;
        default:                                OutputDebugStringA("Type: Unknown\n"); break;
    }
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         OutputDebugStringA("Severity: high\n\n"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       OutputDebugStringA("Severity: medium\n\n"); break;
        case GL_DEBUG_SEVERITY_LOW:          OutputDebugStringA("Severity: low\n\n"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: OutputDebugStringA("Severity: notification\n\n"); break;
        default:                             OutputDebugStringA("Severity: unknown\n\n"); break;
    }
}

internal void win32_init_opengl(HWND window)
{
    HDC window_dc = GetDC(window);

    // State desired pixel format properties
    PIXELFORMATDESCRIPTOR desired_pixel_format = {0};
    desired_pixel_format.nSize = sizeof(desired_pixel_format);
    desired_pixel_format.nVersion = 1;
    desired_pixel_format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    desired_pixel_format.cColorBits = 32;
    desired_pixel_format.cDepthBits = 24;
    desired_pixel_format.cStencilBits = 8;
    desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
    desired_pixel_format.iLayerType = PFD_MAIN_PLANE;

    // Find the closest available pixel format on the graphics card and use that
    int suggested_pixel_format_index = ChoosePixelFormat(window_dc, &desired_pixel_format);
    PIXELFORMATDESCRIPTOR suggested_pixel_format = {0};
    DescribePixelFormat(window_dc, suggested_pixel_format_index,
                        sizeof(suggested_pixel_format), &suggested_pixel_format);
    SetPixelFormat(window_dc, suggested_pixel_format_index, &suggested_pixel_format);

    // Get an OpenGL rendering context and set it as the current context
    HGLRC opengl_rc = wglCreateContext(window_dc);
    if (!wglMakeCurrent(window_dc, opengl_rc))
    {
        // TODO(lucas): Logging
    }

    if (!gladLoadGL())
    {
        // TODO(lucas): Logging
        MessageBoxA(0, "Glad initilization filed", "Glad Error", MB_OK);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Print version
    // TODO(lucas): Logging
    char* version = (char*)glGetString(GL_VERSION);
    char buf[128];
    snprintf(buf, sizeof(buf), "OpenGL version: %s\n", version);
    OutputDebugStringA(buf);

    // Set up debug context
#ifdef ALCHEMY_DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(opengl_error_callback, (void*)0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
        // TODO(lucas): Logging
        OutputDebugStringA("OpenGL debug mode enabled.\n");
#endif

    ReleaseDC(window, window_dc);
}

internal LRESULT CALLBACK win32_main_window_callback(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;

    switch(msg)
    {
        case WM_QUIT:
        {
            global_running = false;
            DestroyWindow(window);
        } break;

        /*
        Sent when the window/application should close (e.g., user clicks X button).
        */
        case WM_CLOSE:
        {
            global_running = false;
            DestroyWindow(window);
        } break;

        /*
        Sent when a window is being destroyed.
        */
        case WM_DESTROY:
        {
            // TODO(lucas): Handle this with a message to the user?
            global_running = false;
            PostQuitMessage(0);
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        {
            ASSERT(!"Keyboard input came in through a non-dispatch message!");
        } break;

        /*
        All message types that are not explicitly handled will end up here. DefWindowProc just provides default
        processing. Having a default case ensures that every Windows message gets processed.
        */
        default:
        {
            result = DefWindowProcW(window, msg, wparam, lparam);
        } break;
    }
    
    return result;
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmd_line, int show_cmd)
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    i64 ticks_per_second = frequency.QuadPart;

    // Open a window
    HWND window;
    {
        WNDCLASSEXW window_class = {0};
        window_class.cbSize = sizeof(WNDCLASSEXW);
        window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
        window_class.lpfnWndProc = &win32_main_window_callback;
        window_class.hInstance = instance;
        window_class.hIcon = LoadIconW(0, IDI_APPLICATION);
        window_class.hCursor = LoadCursorW(0, IDC_ARROW);
        window_class.lpszClassName = L"MyWindowClass";
        window_class.hIconSm = LoadIconW(0, IDI_APPLICATION);

        if(!RegisterClassExW(&window_class))
        {
            MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }

        RECT initial_window_rect = { 0, 0, 1280, 720 };
        AdjustWindowRectEx(&initial_window_rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
        LONG initial_window_width = initial_window_rect.right - initial_window_rect.left;
        LONG initial_window_height = initial_window_rect.bottom - initial_window_rect.top;

        window = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
                                window_class.lpszClassName,
                                L"Alchemy",
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                initial_window_width, 
                                initial_window_height,
                                0, 0, instance, 0);

        if(!window)
        {
            MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }
    }

    // Initialize COM
    {
        if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
        {
            // Could not initialize COM
            MessageBoxA(0, "CoInitializeEx failed", "COM error", MB_OK);
        }
    }

    win32_init_opengl(window);

    Win32XAudio2State xaudio2_state;
    win32_init_xaudio2(&xaudio2_state);

    ExampleState state;
    init_example_state(&state);

    i64 start_ticks = win32_get_ticks();

    global_running = true;
    while(global_running)
    {
        // Double buffer input to detect buttons held
        Input old_input = state.input;
        win32_process_keyboard_mouse_input(window, &state.input.keyboard, &state.input.mouse);
        win32_process_xinput_gamepad_input(&state.input);

        HDC device_context = GetDC(window);
        Win32WindowDimensions dimensions = win32_get_window_dimensions(window);

        f32 delta_time = win32_get_seconds_elapsed(start_ticks, win32_get_ticks(), ticks_per_second);
        start_ticks = win32_get_ticks();
        example_update_and_render(&state, delta_time, dimensions.width, dimensions.height);

        // Sound test
        win32_process_sound_output(&state.sound_output, &xaudio2_state);

        SwapBuffers(device_context);
        ReleaseDC(window, device_context);

        // The input of this frame becomes the old input for next frame
        old_input = state.input;
    }

    delete_example_state(&state);
    return 0;
}
