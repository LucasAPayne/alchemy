#include "alchemy/renderer/shader.h"
#include "alchemy/util/types.h"

#include <glad/glad.h>

#include <stdio.h>  // File I/O
#include <stdlib.h> // malloc

// NOTE(lucas): This is a temporary file loading function that will probably be changed or replaced
/*
* Reads the file 'path' into a string 'buf'. If 'add_null' is true, terminates string with '\0'.
* If successful, returns file size.
* If unsuccessful, returns -1.
* WARNING: Remember to free 'buf' after calling this function.
*/
internal char* file_to_string(const char* path)
{
    char* buf = NULL;
    i64 length = 0;
    FILE* f;
    errno_t error_code = fopen_s(&f, path, "rb");

    if (f)
    {
        fseek(f, 0, SEEK_END);
        length = ftell(f);
        fseek(f, 0, SEEK_SET);
        buf = (char*)malloc(length+1);

        if (buf)
            fread(buf, 1, length, f);

        fclose(f);
        buf[length] = '\0';
    }
    return buf;
}

// Check whether there are errors in shader compilation/linking and print the log if so.
internal void shader_error_check(GLuint shader)
{
    GLint success = 0;
    GLchar info_log[512];
    
    if (glIsShader(shader))
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    else if (glIsProgram(shader))
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
    else
    {
        // TODO(lucas): Logging
        // Shader error: Object is not a shader or shader program!
        ASSERT(0);
    }

    if (!success)
    {
        if (glIsShader(shader))
        {
            glGetShaderInfoLog(shader, sizeof(info_log), NULL, info_log);
            // TODO(lucas): Logging
            // Shader error: Shader compilation failed
            ASSERT(0);
        }
        else if (glIsProgram(shader))
        {
            GLsizei log_length = 0;
            glGetProgramInfoLog(shader, sizeof(info_log), &log_length, info_log);
            // TODO(lucas): Logging
            // Shader Error: Shader linking failed
            ASSERT(0);
        }
        // TODO(lucas): Output info log
    }
}

u32 shader_init(const char* vertex_shader_path, const char* fragment_shader_path)
{
    // Read shaders from files
    char* vertex_shader_source = file_to_string(vertex_shader_path);
    char* fragment_shader_source = file_to_string(fragment_shader_path);

    // Create and compile shaders
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    shader_error_check(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    shader_error_check(fragment_shader);

    // Link both shaders into a shader program
    GLuint shader_program = glCreateProgram();
    // TODO(lucas): Logging
    if (!shader_program)
    {
        // Shader Error: Shader program creation failed!
        ASSERT(0);
    }
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    shader_error_check(shader_program);

    free(vertex_shader_source);
    free(fragment_shader_source);

    // Delete the shader sources as they are no longer needed
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}

void shader_bind(u32 id)
{
    glUseProgram(id);
}

void shader_unbind()
{
    glUseProgram(0);
}

void shader_delete(u32 id)
{
    glDeleteProgram(id);
}

void shader_set_v3(u32 shader, const char* name, v3 value)
{
    shader_bind(shader);
    glUniform3f(glGetUniformLocation(shader, name), value.x, value.y, value.z);
}

void shader_set_v4(u32 shader, const char* name, v4 value)
{
    shader_bind(shader);
    glUniform4f(glGetUniformLocation(shader, name), value.x, value.y, value.z, value.w);
}

void shader_set_m4(u32 shader, const char* name, m4 value, b32 transpose)
{
    shader_bind(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, (GLboolean)transpose, value.raw[0]);
}


void shader_set_int(u32 shader, const char* name, int value)
{
    shader_bind(shader);
    glUniform1i(glGetUniformLocation(shader, name), value);
}
