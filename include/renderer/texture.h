#pragma once

#include "util/types.h"

u32 generate_texture();
u32 generate_texture_from_file(const char* filename);

void texture_bind(u32 texture, u32 slot);
void texture_unbind(void);
void texture_delete(u32 texture);
