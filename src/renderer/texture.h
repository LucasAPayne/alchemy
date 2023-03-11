#pragma once

#include "types.h"

u32 generate_texture(const char* filename);
u32 generate_font_texture(u32 width, u32 height, ubyte* data);

void bind_texture(u32 texture, u32 slot);
void unbind_texture(void);
void delete_texture(u32 texture);
