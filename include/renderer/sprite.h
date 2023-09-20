#pragma once

#include "renderer/texture.h"
#include "util/types.h"

typedef struct SpriteRenderer
{
    u32 shader;
    u32 vao;
    u32 vbo;
    u32 ibo;
} SpriteRenderer;

void sprite_renderer_init(SpriteRenderer* sprite_renderer, u32 shader);
void sprite_renderer_delete(SpriteRenderer* sprite_renderer);

typedef struct Sprite
{
    SpriteRenderer* renderer;
    Texture* texture;
    v4 color;
    v2 position;
    iv2 size;
    f32 rotation; // Rotation in degrees
} Sprite;

Sprite sprite_init(SpriteRenderer* renderer, Texture* tex);
void sprite_draw(Sprite sprite);
