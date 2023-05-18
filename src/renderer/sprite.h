#pragma once

#include "types.h"

#include <glm/glm.hpp>

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
    glm::vec3 color;
    glm::vec2 position;
    glm::vec2 size;
    f32 rotation; // Rotation in degrees
} Sprite;

void draw_sprite(SpriteRenderer* sprite_renderer, u32 texture, glm::vec2 position, glm::vec2 size, f32 rotation = 0.0f,
                 glm::vec3 color=glm::vec3(1.0f));
void draw_sprite(Sprite sprite);
