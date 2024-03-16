#include "alchemy/renderer/shader.h"
<<<<<<< HEAD
=======
#include "alchemy/renderer/renderer.h"
#include "alchemy/util/memory.h"
#include "alchemy/util/str.h"
>>>>>>> ebd83c9268a6a9fec3725ad1abd65f4521e57b33
#include "alchemy/util/types.h"

#include <glad/glad.h>

#include <stdio.h> // File I/O

internal char* file_to_string(const char* path, MemoryArena* arena)
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
        buf = push_array(arena, length+1, char);

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

u32 shader_init(Renderer* renderer, const char* vert_shader_path, const char* frag_shader_path)
{
    // Read shaders from files
    char* vert_shader_source = file_to_string(vert_shader_path, &renderer->scratch_arena);
    char* frag_shader_source = file_to_string(frag_shader_path, &renderer->scratch_arena);

    int vert_shader_len = str_len(vert_shader_source);
    int frag_shader_len = str_len(frag_shader_source);

    // Create and compile shaders
    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vert_shader_source, NULL);
    glCompileShader(vert_shader);
    shader_error_check(vert_shader);

    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &frag_shader_source, NULL);
    glCompileShader(frag_shader);
    shader_error_check(frag_shader);

    // Link both shaders into a shader program
    GLuint shader = glCreateProgram();
    // TODO(lucas): Logging
    if (!shader)
    {
        // Shader Error: Shader program creation failed!
        ASSERT(0);
    }
    glAttachShader(shader, vert_shader);
    glAttachShader(shader, frag_shader);
    glLinkProgram(shader);
    shader_error_check(shader);

    memory_arena_pop(&renderer->scratch_arena, vert_shader_len+1);
    memory_arena_pop(&renderer->scratch_arena, frag_shader_len+1);

    // Delete the shader sources as they are no longer needed
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return shader;
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

void shader_set_i32(u32 shader, const char* name, i32 value)
{
    shader_bind(shader);
    glUniform1i(glGetUniformLocation(shader, name), value);
}

void shader_set_iv2(u32 shader, const char* name, iv2 value)
{
    shader_bind(shader);
    glUniform2i(glGetUniformLocation(shader, name), value.x, value.y);
}

void shader_set_iv3(u32 shader, const char* name, iv3 value)
{
    shader_bind(shader);
    glUniform3i(glGetUniformLocation(shader, name), value.x, value.y, value.z);
}

void shader_set_iv4(u32 shader, const char* name, iv4 value)
{
    shader_bind(shader);
    glUniform4i(glGetUniformLocation(shader, name), value.x, value.y, value.z, value.w);
}

void shader_set_f32(u32 shader, const char* name, f32 value)
{
    shader_bind(shader);
    glUniform1f(glGetUniformLocation(shader, name), value);
}

void shader_set_v2(u32 shader, const char* name, v2 value)
{
    shader_bind(shader);
    glUniform2f(glGetUniformLocation(shader, name), value.x, value.y);
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

void shader_set_m2(u32 shader, const char* name, m2 value, b32 transpose)
{
    shader_bind(shader);
    glUniformMatrix2fv(glGetUniformLocation(shader, name), 1, (GLboolean)transpose, value.raw[0]);
}

void shader_set_m3(u32 shader, const char* name, m3 value, b32 transpose)
{
    shader_bind(shader);
    glUniformMatrix3fv(glGetUniformLocation(shader, name), 1, (GLboolean)transpose, value.raw[0]);
}

void shader_set_m4(u32 shader, const char* name, m4 value, b32 transpose)
{
    shader_bind(shader);
    glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, (GLboolean)transpose, value.raw[0]);
}
