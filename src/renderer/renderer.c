#include "renderer/renderer.h"
#include "util/alchemy_math.h"

#include <glad/glad.h>

internal RenderObject sprite_renderer_init(u32 shader)
{
    RenderObject sprite_renderer = {0};

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

    sprite_renderer.shader = shader;
    sprite_renderer.vao = vao;
    sprite_renderer.vbo = vbo;
    sprite_renderer.ibo = ibo;

    return sprite_renderer;
}

internal RenderObject poly_renderer_init(u32 shader)
{
    RenderObject poly_renderer = {0};

    f32 vertices[] =
    {
        // pos
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), 0);
    
    glBindVertexArray(0);

    poly_renderer.shader = shader;
    poly_renderer.vao = vao;
    poly_renderer.vbo = vbo;
    poly_renderer.ibo = ibo;
    
    return poly_renderer;
}

internal RenderObject font_renderer_init(u32 shader)
{
    RenderObject font_renderer = {0};
    u32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    u32 vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // NOTE(lucas): While vertex buffer data changes a lot, the order in which indices are drawn
    // does not. So, indices and index buffer can be defined here.
    u32 indices[] = 
    {
      0, 1, 3,
      1, 2, 3  
    };

    u32 ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    // TODO(lucas): Usage is GL_STATIC_DRAW for now, while vbo uses GL_DYNAMIC_DRAW.
    // Is this acceptable since the ibo will not change, while the vbo will frequently?
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)(2*sizeof(f32)));

    glBindVertexArray(0);

    font_renderer.shader = shader;
    font_renderer.vao = vao;
    font_renderer.vbo = vbo;
    font_renderer.ibo = ibo;

    return font_renderer;
}

internal void render_object_delete(RenderObject* render_object)
{
    glDeleteVertexArrays(1, &render_object->vao);
    glDeleteBuffers(1, &render_object->vbo);
    glDeleteBuffers(1, &render_object->ibo);
    shader_delete(render_object->shader);
}

Renderer renderer_init()
{
    Renderer renderer = {0};

    // TODO(lucas): These relative paths might cause problems in the future
    u32 poly_shader = shader_init("shaders/poly.vert", "shaders/poly.frag");
    u32 sprite_shader = shader_init("shaders/sprite.vert", "shaders/sprite.frag");
    u32 font_shader = shader_init("shaders/font.vert", "shaders/font.frag");

    renderer.poly_renderer = poly_renderer_init(poly_shader);
    renderer.sprite_renderer = sprite_renderer_init(sprite_shader);
    renderer.font_renderer = font_renderer_init(font_shader);

    return renderer;
}

void renderer_delete(Renderer* renderer)
{
    render_object_delete(&renderer->poly_renderer);
    render_object_delete(&renderer->sprite_renderer);
    render_object_delete(&renderer->font_renderer);
}

void renderer_viewport(Renderer* renderer, int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
    m4 projection = m4_ortho(0.0f, (f32)width, (f32)height, 0.0f, -1.0f, 1.0f);
    shader_set_m4(renderer->poly_renderer.shader, "projection", projection, 0);
    shader_set_m4(renderer->sprite_renderer.shader, "projection", projection, 0);
    shader_set_m4(renderer->font_renderer.shader, "projection", projection, 0);
}

void renderer_clear(v4 color)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(color.r, color.g, color.b, color.a);
}

void draw_rect(Renderer* renderer, v2 position, v2 size, v4 color, f32 rotation)
{
    m4 model = m4_identity();
    model = m4_translate(model, (v3){position.x, position.y, 0.0f});

    // NOTE(lucas): The origin of a quad is at the top left, but we want the origin to appear in the center of the quad
    // for rotation. So, before rotation, translate the quad right and down by half its size. After the rotation, undo
    // this translation.
    model = m4_translate(model, (v3){0.5f*size.x, 0.5f*size.y, 0.0f});
    model = m4_rotate(model, glm_rad(rotation), (v3){0.0f, 0.0f, 1.0f});
    model = m4_translate(model, (v3){-0.5f*size.x, -0.5f*size.y, 0.0f});

    // Scale sprite to appropriate size
    model = m4_scale(model, (v3){(f32)size.x, (f32)size.y, 1.0f});

    // Set model matrix and color shader values
    shader_set_m4(renderer->poly_renderer.shader, "model", model, 0);
    shader_set_v4(renderer->poly_renderer.shader, "color", color);

    glBindVertexArray(renderer->poly_renderer.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
