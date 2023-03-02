#pragma once

#include "types.h"

#include <glm/glm.hpp>

struct SpriteRenderer
{
    u32 shader;
    u32 vao;
    u32 vbo;
    u32 ibo;
};

void init_sprite_renderer(SpriteRenderer* sprite_renderer, u32 shader);
void delete_sprite_renderer(SpriteRenderer* sprite_renderer);

struct Sprite
{
    SpriteRenderer* renderer;
    u32 texture;
    glm::vec3 color;
    glm::vec2 position;
    glm::vec2 size;
    f32 rotation; // Rotation in degrees
};

void draw_sprite(SpriteRenderer* sprite_renderer, u32 texture, glm::vec2 position, glm::vec2 size, f32 rotation = 0.0f,
                 glm::vec3 color=glm::vec3(1.0f));
void draw_sprite(Sprite sprite);
