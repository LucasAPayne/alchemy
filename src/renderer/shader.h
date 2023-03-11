#pragma once

#include "types.h"

#include <glm/glm.hpp>

u32 shader_init(const char* vertex_shader_path, const char* fragment_shader_path);
void bind_shader(u32 id);
void unbind_shader();
void delete_shader(u32 id);

// Wrapper functions to set uniforms
void shader_set_vec3f(u32 shader, const char* name, f32 value1, f32 value2, f32 value3);
void shader_set_vec3f(u32 shader, const char* name, glm::vec3 value);

void shader_set_vec4f(u32 shader, const char* name, f32 value1, f32 value2, f32 value3, f32 value4);
void shader_set_vec4f(u32 shader, const char* name, glm::vec4 value);

void shader_set_mat4f(u32 shader, const char* name, glm::mat4 value, bool transpose = false);

void shader_set_int(u32 shader, const char* name, int value);
