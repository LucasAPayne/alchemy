#include "renderer/renderer.h"
#include "util/alchemy_math.h"

#include <glad/glad.h>

internal u32 vao_init()
{
    u32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    return vao;
}

internal u32 vbo_init(f32* vertices, usize size)
{
    u32 vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    return vbo;
}

internal u32 vbo_init_empty()
{
    u32 vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    return vbo;
}

internal u32 ibo_init(u32* indices, usize size)
{
    u32 ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);

    return ibo;
}

internal void vertex_layout_set(u32 index, int size, u32 stride, const void* ptr)
{
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, ptr);
}

internal RenderObject sprite_renderer_init(u32 shader)
{
    RenderObject sprite_renderer = {0};
    sprite_renderer.shader = shader;

    f32 vertices[] = 
    { 
        // pos      // tex
        1.0f, 0.0f, 1.0f, 1.0f, // top right
        1.0f, 1.0f, 1.0f, 0.0f, // bottom right
        0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        0.0f, 0.0f, 0.0f, 1.0f  // top left
    };

    u32 indices[] = 
    {
        0, 1, 3,
        1, 2, 3  
    };

    sprite_renderer.vao = vao_init();
    sprite_renderer.vbo = vbo_init(vertices, sizeof(vertices));
    sprite_renderer.ibo = ibo_init(indices, sizeof(indices));

    vertex_layout_set(0, 2, 4*sizeof(f32), 0);
    vertex_layout_set(1, 2, 4*sizeof(f32), (void*)(2*sizeof(f32)));
    
    glBindVertexArray(0);

    return sprite_renderer;
}

internal RenderObject rect_renderer_init(u32 shader)
{
    RenderObject rect_renderer = {0};
    rect_renderer.shader = shader;

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

    rect_renderer.vao = vao_init();
    rect_renderer.vbo = vbo_init(vertices, sizeof(vertices));
    rect_renderer.ibo = ibo_init(indices, sizeof(indices));

    vertex_layout_set(0, 2, 2*sizeof(f32), 0);
    
    glBindVertexArray(0);
    
    return rect_renderer;
}

internal RenderObject font_renderer_init(u32 shader)
{
    RenderObject font_renderer = {0};
    font_renderer.shader = shader;

    // NOTE(lucas): While vertex buffer data changes a lot, the order in which indices are drawn
    // does not. So, indices and index buffer can be defined here.
    u32 indices[] = 
    {
      0, 1, 3,
      1, 2, 3  
    };

    font_renderer.vao = vao_init();
    font_renderer.vbo = vbo_init_empty();
    font_renderer.ibo = ibo_init(indices, sizeof(indices));

    vertex_layout_set(0, 2, 4*sizeof(f32), 0);
    vertex_layout_set(1, 2, 4*sizeof(f32), (void*)(2*sizeof(f32)));

    glBindVertexArray(0);

    return font_renderer;
}

internal RenderObject line_renderer_init(u32 shader)
{
    RenderObject line_renderer = {0};
    line_renderer.shader = shader;
    line_renderer.vao = vao_init();
    line_renderer.vbo = vbo_init_empty();

    // f32 vertices[] =
    // {
    //     0.0f, 0.0f,
    //     0.0f, 1.0f
    // };
    // line_renderer.vbo = vbo_init(vertices, sizeof(vertices));
    
    vertex_layout_set(0, 2, 2*sizeof(f32), 0);

    glBindVertexArray(0);

    return line_renderer;
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
    u32 poly_shader   = shader_init("shaders/poly.vert", "shaders/poly.frag");
    u32 sprite_shader = shader_init("shaders/sprite.vert", "shaders/sprite.frag");
    u32 font_shader   = shader_init("shaders/font.vert", "shaders/font.frag");

    renderer.line_renderer   = line_renderer_init(poly_shader);
    renderer.rect_renderer   = rect_renderer_init(poly_shader);
    renderer.sprite_renderer = sprite_renderer_init(sprite_shader);
    renderer.font_renderer   = font_renderer_init(font_shader);

    return renderer;
}

void renderer_delete(Renderer* renderer)
{
    render_object_delete(&renderer->rect_renderer);
    render_object_delete(&renderer->sprite_renderer);
    render_object_delete(&renderer->font_renderer);
}

void renderer_viewport(Renderer* renderer, int x, int y, int width, int height)
{
    glViewport(x, y, width, height);
    m4 projection = m4_ortho(0.0f, (f32)width, 0.0f, (f32)height, -1.0f, 1.0f);
    shader_set_m4(renderer->line_renderer.shader,   "projection", projection, false);
    shader_set_m4(renderer->rect_renderer.shader,   "projection", projection, false);
    shader_set_m4(renderer->sprite_renderer.shader, "projection", projection, false);
    shader_set_m4(renderer->font_renderer.shader,   "projection", projection, false);
}

void renderer_clear(v4 color)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(color.r, color.g, color.b, color.a);
}

void draw_line(Renderer* renderer, v2 start, v2 end, v4 color)
{
    glBindVertexArray(renderer->line_renderer.vao);
    f32 vertices[] =
    {
        start.x, start.y,
        end.x, end.y
    };
    // TODO(lucas): Update with glBufferSubData?
    // Update VBO memory
    glBindBuffer(GL_ARRAY_BUFFER, renderer->line_renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    m4 model = m4_identity();
    // model = m4_translate(model, (v3){(end.x - start.x), (end.y - start.y), 0.0f});
    shader_set_m4(renderer->line_renderer.shader, "model", model, 0);
    shader_set_v4(renderer->line_renderer.shader, "color", color);

    glBindVertexArray(renderer->line_renderer.vao);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
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
    shader_set_m4(renderer->rect_renderer.shader, "model", model, 0);
    shader_set_v4(renderer->rect_renderer.shader, "color", color);

    glBindVertexArray(renderer->rect_renderer.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
