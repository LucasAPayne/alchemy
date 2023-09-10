#pragma once

#include "util/types.h"

u32 shader_init(const char* vertex_shader_path, const char* fragment_shader_path);
void bind_shader(u32 id);
void unbind_shader();
void delete_shader(u32 id);

// Wrapper functions to set uniforms
void shader_set_v3(u32 shader, const char* name, v3 value);
void shader_set_v4(u32 shader, const char* name, v4 value);

void shader_set_m4(u32 shader, const char* name, m4 value, b32 transpose);

void shader_set_int(u32 shader, const char* name, int value);
