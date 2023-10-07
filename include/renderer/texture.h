#pragma once

#include "util/types.h"

typedef struct Texture 
{
    u32 id;
    i32 channels;
    iv2 size;
} Texture;

Texture texture_generate(int samples);
void texture_fill_empty_data(Texture* texture, int width, int height, int samples);
Texture texture_load_from_file(const char* filename);

void texture_bind_id(u32 id, int samples);
void texture_bind(Texture* tex, int samples);
void texture_unbind(int samples);
void texture_delete(Texture* tex);
