#include "alchemy/renderer/renderer.h"
#include "alchemy/window.h"
#include "alchemy/util/log.h"

#define GLAD_GL_IMPLEMENTATION
#define GLAD_WGL_IMPLEMENTATION
#include <glad/wgl.h>
#include <glad/gl.h>

#include <windows.h>

void exit_fail(HWND window, HDC window_dc)
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
