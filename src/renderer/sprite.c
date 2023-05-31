#include "sprite.h"

#include "shader.h"
#include "texture.h"
#include "types.h"

#include <glad/glad.h>

void init_sprite_renderer(SpriteRenderer* sprite_renderer, u32 shader)
{
    f32 vertices[] = 
    { 
        // pos      // tex
        1.0f, 0.0f, 1.0f, 0.0f, // top right
        1.0f, 1.0f, 1.0f, 1.0f, // bottom right
        0.0f, 1.0f, 0.0f, 1.0f, // bottom left
        0.0f, 0.0f, 0.0f, 0.0f  // top left
    };

    u32 indices[] = 
    {
    0, 1, 3,
    1, 2, 3  
    };

    u32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    u32 vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    u32 ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)(2*sizeof(f32)));
    
    glBindVertexArray(0);

    sprite_renderer->shader = shader;
    sprite_renderer->vao = vao;
    sprite_renderer->vbo = vbo;
    sprite_renderer->ibo = ibo;
}

void delete_sprite_renderer(SpriteRenderer* sprite_renderer)
{
    glDeleteVertexArrays(1, &sprite_renderer->vao);
    glDeleteBuffers(1, &sprite_renderer->vbo);
    glDeleteBuffers(1, &sprite_renderer->ibo);
    delete_shader(sprite_renderer->shader);
}

// TODO(lucas): Allow using only color and no texture
// Note that texture could not be set to 0 for no texture since that is OpenGL's error case
void draw_sprite(Sprite sprite)
{
    mat4s model = glms_mat4_identity();
    model = glms_translate(model, (vec3s){sprite.position.x, sprite.position.y, 0.0f});

    // NOTE(lucas): The origin of a quad is at the top left, but we want the origin to appear in the center of the quad
    // for rotation. So, before rotation, translate the quad right and down by half its size. After the rotation, undo
    // this translation.
    model = glms_translate(model, (vec3s){0.5f * sprite.size.x, 0.5f * sprite.size.y, 0.0f});
    model = glms_rotate(model, glm_rad(sprite.rotation), (vec3s){0.0f, 0.0f, 1.0f});
    model = glms_translate(model, (vec3s){-0.5f * sprite.size.x, -0.5f * sprite.size.y, 0.0f});

    // Scale sprite to appropriate size
    model = glms_scale(model, (vec3s){sprite.size.x, sprite.size.y, 1.0f});

    // Set model matrix and color shader values
    shader_set_mat4f(sprite.renderer->shader, "model", model, 0);
    shader_set_vec3f(sprite.renderer->shader, "color", sprite.color);

    bind_texture(sprite.texture, 0);

    glBindVertexArray(sprite.renderer->vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
