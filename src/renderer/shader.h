#pragma once

#include "util/types.h"

u32 shader_init(const char* vertex_shader_path, const char* fragment_shader_path);
void bind_shader(u32 id);
void unbind_shader();
void delete_shader(u32 id);

// Wrapper functions to set uniforms
void shader_set_vec3f(u32 shader, const char* name, vec3s value);
void shader_set_vec4f(u32 shader, const char* name, vec4s value);

void shader_set_mat4f(u32 shader, const char* name, mat4s value, b32 transpose);

void shader_set_int(u32 shader, const char* name, int value);
