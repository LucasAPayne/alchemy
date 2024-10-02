#pragma once

#include "alchemy/util/types.h"

typedef struct Renderer Renderer;

u32 shader_init(Renderer* renderer, const char* vert_shader_path, const char* frag_shader_path);
void shader_bind(u32 id);
void shader_unbind();
void shader_delete(u32 id);

// Wrapper functions to set uniforms
void shader_set_i32(u32 shader, const char* name, i32 value);
void shader_set_iv2(u32 shader, const char* name, iv2 value);
void shader_set_iv3(u32 shader, const char* name, iv3 value);
void shader_set_iv4(u32 shader, const char* name, iv4 value);

void shader_set_f32(u32 shader, const char* name, f32 value);
void shader_set_v2(u32 shader, const char* name, v2 value);
void shader_set_v3(u32 shader, const char* name, v3 value);
void shader_set_v4(u32 shader, const char* name, v4 value);

void shader_set_m2(u32 shader, const char* name, m2 value, b32 transpose);
void shader_set_m3(u32 shader, const char* name, m3 value, b32 transpose);
void shader_set_m4(u32 shader, const char* name, m4 value, b32 transpose);
