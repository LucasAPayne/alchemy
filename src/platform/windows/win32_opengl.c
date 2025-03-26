#include "alchemy/renderer/renderer.h"
#include "alchemy/window.h"
#include "alchemy/util/log.h"

#include <glad/glad.h>

#include <windows.h>

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
    if(id == 131218)
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

void opengl_init(Window window)
{
    HDC window_dc = GetDC(window.ptr);

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
        log_error("wglMakeCurrent failed. Unable to set OpenGL rendering context.");

    if (!gladLoadGL())
        log_error("Glad initilization failed");

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

    ReleaseDC(window.ptr, window_dc);
}
