#pragma once

#include "types.h"

u32 generate_texture();
u32 generate_texture_from_file(const char* filename);

void bind_texture(u32 texture, u32 slot);
void unbind_texture(void);
void delete_texture(u32 texture);
