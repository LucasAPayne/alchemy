#pragma once

#include "alchemy/util/types.h"

typedef struct Renderer Renderer;

u32 shader_init(Renderer* renderer, const char* vert_shader_path, const char* frag_shader_path);
void shader_bind(u32 id);
void shader_unbind();
void shader_delete(u32 id);

// Wrapper functions to set uniforms
void shader_set_v3(u32 shader, const char* name, v3 value);
void shader_set_v4(u32 shader, const char* name, v4 value);

void shader_set_m4(u32 shader, const char* name, m4 value, b32 transpose);

void shader_set_int(u32 shader, const char* name, int value);
