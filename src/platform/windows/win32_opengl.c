#include "alchemy/renderer/renderer.h"
#include "alchemy/window.h"

#include <glad/glad.h>

#include <windows.h>

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
        case GL_DEBUG_SOURCE_API:             OutputDebugStringA("Source: API\n");             break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   OutputDebugStringA("Source: Window System\n");   break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: OutputDebugStringA("Source: Shader Compiler\n"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     OutputDebugStringA("Source: Third Party\n");     break;
        case GL_DEBUG_SOURCE_APPLICATION:     OutputDebugStringA("Source: Application\n");     break;
        case GL_DEBUG_SOURCE_OTHER:           OutputDebugStringA("Source: Other\n");           break;
        default:                              OutputDebugStringA("Source: Unknown\n");         break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               OutputDebugStringA("Type: Error\n");               break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: OutputDebugStringA("Type: Deprecated Behavior\n"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  OutputDebugStringA("Type: Undefined Behavior\n");  break; 
        case GL_DEBUG_TYPE_PORTABILITY:         OutputDebugStringA("Type: Portability\n");         break;
        case GL_DEBUG_TYPE_PERFORMANCE:         OutputDebugStringA("Type: Performance\n");         break;
        case GL_DEBUG_TYPE_MARKER:              OutputDebugStringA("Type: Marker\n");              break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          OutputDebugStringA("Type: Push Group\n");          break;
        case GL_DEBUG_TYPE_POP_GROUP:           OutputDebugStringA("Type: Pop Group\n");           break;
        case GL_DEBUG_TYPE_OTHER:               OutputDebugStringA("Type: Other\n");               break;
        default:                                OutputDebugStringA("Type: Unknown\n");             break;
    }
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         OutputDebugStringA("Severity: high\n\n");         break;
        case GL_DEBUG_SEVERITY_MEDIUM:       OutputDebugStringA("Severity: medium\n\n");       break;
        case GL_DEBUG_SEVERITY_LOW:          OutputDebugStringA("Severity: low\n\n");          break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: OutputDebugStringA("Severity: notification\n\n"); break;
        default:                             OutputDebugStringA("Severity: unknown\n\n");      break;
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

    ReleaseDC(window.ptr, window_dc);
}
