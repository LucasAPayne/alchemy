#include "alchemy/window.h"
#include "alchemy/util/log.h"
#include "alchemy/util/types.h"

#include "win32_base.c"

#include <windows.h>

#define GLAD_GL_IMPLEMENTATION
#define GLAD_WGL_IMPLEMENTATION
#include <glad/wgl.h>
#include <glad/gl.h>

internal void exit_fail(HWND window, HDC window_dc)
{
    ReleaseDC(window, window_dc);
    DestroyWindow(window);
    ExitProcess(1);
}

void GLAPIENTRY opengl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                      const GLchar* message, const void* user_param)
{
    persist b32 shader_recomp_warning_printed = false;
    if (shader_recomp_warning_printed) return;

    // Filter out purely informational messages, which might print every frame.
    // 131185 says since GL_STATIC_DRAW is used, the buffer is placed in video memory.
    // 131169 says that the driver has allocated memory for the render buffer.
    // 131204 says a texture cannot be used for texture mapping.
    if (id == 131185 || id == 131169 || id == 131204) return;

    // Only log performance-related messages once. Otherwise, they might print every frame.
    if (id == 131218)
        shader_recomp_warning_printed = true;

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         log_error("OpenGL (%d): %s", id, message); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       log_warn("OpenGL (%d): %s", id, message);  break;
        case GL_DEBUG_SEVERITY_LOW:          log_trace("OpenGL (%d): %s", id, message); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: log_info("OpenGL (%d): %s", id, message);  break;
        default:                             log_info("OpenGL (%d): %s", id, message);  break;
    }

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             log_debug("OpenGL error source: API");             break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   log_debug("OpenGL error source: Window System");   break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: log_debug("OpenGL error source: Shader Compiler"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     log_debug("OpenGL error source: Third Party");     break;
        case GL_DEBUG_SOURCE_APPLICATION:     log_debug("OpenGL error source: Application");     break;
        case GL_DEBUG_SOURCE_OTHER:           log_debug("OpenGL error source: Other");           break;
        default:                              log_debug("OpenGL error source: Unknown");         break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               log_debug("OpenGL error type: Error");               break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: log_debug("OpenGL error type: Deprecated Behavior"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  log_debug("OpenGL error type: Undefined Behavior");  break;
        case GL_DEBUG_TYPE_PORTABILITY:         log_debug("OpenGL error type: Portability");         break;
        case GL_DEBUG_TYPE_PERFORMANCE:         log_debug("OpenGL error type: Performance");         break;
        case GL_DEBUG_TYPE_MARKER:              log_debug("OpenGL error type: Marker");              break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          log_debug("OpenGL error type: Push Group");          break;
        case GL_DEBUG_TYPE_POP_GROUP:           log_debug("OpenGL error type: Pop Group");           break;
        case GL_DEBUG_TYPE_OTHER:               log_debug("OpenGL error type: Other");               break;
        default:                                log_debug("OpenGL error type: Unknown");             break;
    }
}

void opengl_init(Window* window)
{
    HDC window_dc = GetDC(window->ptr);
    if (!window_dc)
    {
        log_fatal("Failed to get window device context");
        DestroyWindow(window->ptr);
        ExitProcess(1);
    }

    // State desired pixel format properties
    PIXELFORMATDESCRIPTOR pfd = {0};
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int format = ChoosePixelFormat(window_dc, &pfd);
    b32 format_success = SetPixelFormat(window_dc, format, &pfd);
    if (!format || !format_success)
    {
        log_fatal("Failed to set pixel format");
        exit_fail(window->ptr, window_dc);
    }

    // Create a temporary OpenGL context. The real one is made through WGL
    HGLRC temp_context = wglCreateContext(window_dc);
    if (!temp_context)
    {
        log_fatal("Failed to create OpenGL context");
        exit_fail(window->ptr, window_dc);
    }
    wglMakeCurrent(window_dc, temp_context);

    // Load WGL extensions
    gladLoaderLoadWGL(window_dc);

    int attribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 6,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };

    // Create final OpenGL context and delete temporary one
    HGLRC opengl_context = wglCreateContextAttribsARB(window_dc, 0, attribs);
    if (!opengl_context)
    {
        log_fatal("Failed to create OpenGL context.");
        exit_fail(window->ptr, window_dc);
    }
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(temp_context);

    if (!wglMakeCurrent(window_dc, opengl_context))
    {
        log_fatal("wglMakeCurrent failed. Unable to set OpenGL rendering context.");
        exit_fail(window->ptr, window_dc);
    }

    if (!gladLoaderLoadGL())
    {
        log_fatal("GLAD initialization failed.");
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(opengl_context);
        exit_fail(window->ptr, window_dc);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    char* version = (char*)glGetString(GL_VERSION);
    log_info("OpenGL version: %s", version);

    // Set up debug context
#ifdef ALCHEMY_DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(opengl_error_callback, (void*)0);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
    log_debug("OpenGL debug mode enabled.");
#endif

    ReleaseDC(window->ptr, window_dc);
}

global HICON global_window_icon;

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

Window* window_create(const char* title, int width, int height)
{
    Window* window = VirtualAllocEx(GetCurrentProcess(), NULL, sizeof(Window), MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

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
    window_class.lpszClassName = "MyWindowClass";
    window_class.hCursor = LoadCursorA(NULL, IDC_ARROW);

    if (global_window_icon == NULL)
        global_window_icon = LoadIconA(0, IDI_APPLICATION);

    window_class.hIcon = global_window_icon;
    window_class.hIconSm = global_window_icon;

    RECT initial_window_rect = {0, 0, width, height};
    AdjustWindowRectEx(&initial_window_rect, WS_VISIBLE|WS_CAPTION, 0, 0);
    // AdjustWindowRectEx(&initial_window_rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);

    if(!RegisterClassExA(&window_class))
        win32_error_callback();

    LONG initial_window_width = initial_window_rect.right - initial_window_rect.left;
    LONG initial_window_height = initial_window_rect.bottom - initial_window_rect.top;

    HWND hwnd = CreateWindowExA(0, window_class.lpszClassName, title,
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                                initial_window_width, initial_window_height, 0, 0, instance, 0);

    if(!hwnd)
        win32_error_callback();

    window->ptr = hwnd;
    window->open = true;

    // Associate window data with the window ptr
    SetWindowLongPtrA(window->ptr, GWLP_USERDATA, (LONG_PTR)window);

    opengl_init(window);

    window->_prev_frame_ticks = win32_get_ticks();

    return window;
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

void window_icon_set_from_resource(int id)
{
    global_window_icon = (HICON)LoadImageA(GetModuleHandleA(0), MAKEINTRESOURCEA(id), IMAGE_ICON,
                                           0, 0, LR_DEFAULTSIZE|LR_SHARED);
}

void console_launch(void)
{
#ifdef ALCHEMY_CONSOLE
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

    SetConsoleOutputCP(CP_UTF8);

    // Set bright blue text to a more vibrant and readable shade
    CONSOLE_SCREEN_BUFFER_INFOEX cbi = {sizeof(CONSOLE_SCREEN_BUFFER_INFOEX)};
    if (GetConsoleScreenBufferInfoEx(out, &cbi))
    {
        cbi.ColorTable[9] = RGB(0, 120, 255);
        SetConsoleScreenBufferInfoEx(out, &cbi);
    }

#endif
}
