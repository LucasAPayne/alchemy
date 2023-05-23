#pragma once

#include "types.h"

extern "C"
{
    typedef struct SpriteRenderer
    {
        u32 shader;
        u32 vao;
        u32 vbo;
        u32 ibo;
    } SpriteRenderer;

    void init_sprite_renderer(SpriteRenderer* sprite_renderer, u32 shader);
    void delete_sprite_renderer(SpriteRenderer* sprite_renderer);

    typedef struct Sprite
    {
        SpriteRenderer* renderer;
        u32 texture;
        vec3 color;
        vec2 position;
        vec2 size;
        f32 rotation; // Rotation in degrees
    } Sprite;

    void draw_sprite(Sprite sprite);
}
