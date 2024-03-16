#pragma once

#include "alchemy/util/types.h"

typedef struct Texture Texture;
typedef struct Renderer Renderer;
typedef struct RenderCommandSprite RenderCommandSprite;

typedef struct Sprite
{
    Texture* texture;
    v4 color;
    v2 position;
    v2 size;
    f32 rotation; // Rotation in degrees
} Sprite;

Sprite sprite_init(Texture* tex);
void output_sprite(Renderer* renderer, RenderCommandSprite* cmd);