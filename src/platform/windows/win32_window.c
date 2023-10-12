#include "window.h"
#include "util/types.h"

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// TODO(lucas): Move OpenGL handling to separate file
#include <glad/glad.h>

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
                            "Alchemy",
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

    win32_init_opengl(window->ptr);

    window->_prev_frame_ticks = win32_get_ticks();
}

void window_render(Window* window)
{
    HDC device_context = GetDC(window->ptr);
    SwapBuffers(device_context);
    ReleaseDC(window->ptr, device_context);
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
