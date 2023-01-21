#include "sprite.h"

#include "shader.h"
#include "texture.h"
#include "types.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void init_sprite_renderer(sprite_renderer* renderer, u32 shader)
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

    renderer->shader = shader;
    renderer->vao = vao;
    renderer->vbo = vbo;
    renderer->ibo = ibo;
}

void delete_sprite_renderer(sprite_renderer* renderer)
{
    glDeleteVertexArrays(1, &renderer->vao);
    glDeleteBuffers(1, &renderer->vbo);
    glDeleteBuffers(1, &renderer->ibo);
    delete_shader(renderer->shader);
}

// TODO(lucas): Allow using only color and no texture
// Note that texture could not be set to 0 for no texture since that is OpenGL's error case
void draw_sprite(sprite_renderer* renderer, u32 texture, glm::vec2 position, glm::vec2 size, f32 rotation,
                 glm::vec3 color)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));

    // NOTE(lucas): The origin of a quad is at the top left, but we want the origin to appear in the center of the quad
    // for rotation. So, before rotation, translate the quad right and down by half its size. After the rotation, undo
    // this translation.
    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

    model = glm::scale(model, glm::vec3(size, 1.0f));

    shader_set_mat4f(renderer->shader, "model", model);
    shader_set_vec3f(renderer->shader, "color", color);

    bind_texture(texture, 0);

    glBindVertexArray(renderer->vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void draw_sprite(sprite s)
{
    draw_sprite(s.renderer, s.texture, s.position, s.size, s.rotation, s.color);
}
