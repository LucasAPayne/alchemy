#include "alchemy/renderer/sprite.h"
#include "alchemy/renderer/renderer.h"
#include "alchemy/util/math.h"

#include <glad/glad.h>

Sprite sprite_init(Texture* tex)
{
    Sprite sprite = {0};
    sprite.texture = tex;

    // Default sprite to have texture's size and no tint or rotation, and zero out position
    sprite.size = tex->size;
    sprite.rotation = 0.0f;
    sprite.position = (v2){0.0f, 0.0f};
    sprite.color = (v4){1.0f, 1.0f, 1.0f, 1.0f};

    return sprite;
}

void output_sprite(Renderer* renderer, RenderCommandSprite* cmd)
{
    Sprite sprite = cmd->sprite;
    m4 model = m4_identity();
    model = m4_translate(model, (v3){sprite.position.x, sprite.position.y, 0.0f});

    // NOTE(lucas): The origin of a quad is at the top left, but we want the origin to appear in the center of the quad
    // for rotation. So, before rotation, translate the quad right and down by half its size. After the rotation, undo
    // this translation.
    model = m4_translate(model, (v3){0.5f*sprite.size.x, 0.5f*sprite.size.y, 0.0f});
    model = m4_rotate(model, glm_rad(sprite.rotation), (v3){0.0f, 0.0f, 1.0f});
    model = m4_translate(model, (v3){-0.5f*sprite.size.x, -0.5f*sprite.size.y, 0.0f});

    // Scale sprite to appropriate size
    model = m4_scale(model, (v3){(f32)sprite.size.x, (f32)sprite.size.y, 1.0f});

    // Set model matrix and color shader values
    shader_set_m4(renderer->sprite_renderer.shader, "model", model, 0);
    shader_set_v4(renderer->sprite_renderer.shader, "color", sprite.color);

    texture_bind(sprite.texture, 0);

    glBindVertexArray(renderer->sprite_renderer.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
