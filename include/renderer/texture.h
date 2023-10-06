#pragma once

#include "util/types.h"

typedef struct Texture 
{
    u32 id;
    i32 channels;
    iv2 size;
} Texture;

Texture texture_generate();
void texture_fill_empty_data(int width, int height);
Texture texture_load_from_file(const char* filename);

void texture_bind_id(u32 id);
void texture_bind(Texture* tex);
void texture_unbind(void);
void texture_delete(Texture* tex);
