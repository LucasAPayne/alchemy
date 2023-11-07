#pragma once

#include "util/types.h"

typedef struct Texture Texture;
typedef struct Renderer Renderer;

typedef struct Sprite
{
    Texture* texture;
    v4 color;
    v2 position;
    v2 size;
    f32 rotation; // Rotation in degrees
} Sprite;

Sprite sprite_init(Texture* tex);
void draw_sprite(Renderer* renderer, Sprite sprite);