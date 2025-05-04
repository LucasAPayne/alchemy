#pragma once

#include "alchemy/util/types.h"

typedef struct Renderer Renderer;
typedef struct MemoryArena MemoryArena;

typedef struct Texture
{
    u32 id;
    i32 channels;
    v2 size;
    ubyte* data;
} Texture;

Texture texture_generate(int samples);
void texture_fill_empty_data(Texture* texture, int width, int height, int samples);
Texture texture_load_from_file(const char* filename, Renderer* renderer, MemoryArena* arena);
Texture texture_load_from_memory(Renderer* renderer, int width, int height, int samples, ubyte* data);

void texture_bind_id(u32 id, int samples);
void texture_bind(Texture* tex, int samples);
void texture_unbind(int samples);
void texture_delete(Texture* tex);
