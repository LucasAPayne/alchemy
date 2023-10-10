#include "renderer/renderer.h"
#include "util/alchemy_math.h"

#include <glad/glad.h>

internal void vao_bind(u32 vao)
{
    glBindVertexArray(vao);
}

internal void vao_unbind(void)
{
    glBindVertexArray(0);
}

internal u32 vao_init(void)
{
    u32 vao;
    glGenVertexArrays(1, &vao);
    vao_bind(vao);

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

internal u32 vbo_init_empty(void)
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

internal u32 ibo_init_empty(void)
{
    u32 ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    return ibo;
}

internal void fbo_bind(u32 fbo)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

internal void fbo_unbind(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

internal u32 fbo_init(void)
{
    u32 fbo;
    glGenFramebuffers(1, &fbo);
    fbo_bind(fbo);

    return fbo;
}

internal void fbo_delete(u32* fbo)
{
    glDeleteFramebuffers(1, fbo);
}

internal void rbo_bind(u32 rbo)
{
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
}

internal void rbo_unbind()
{
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

internal u32 rbo_init()
{
    u32 rbo;
    glGenRenderbuffers(1, &rbo);
    rbo_bind(rbo);

    return rbo;
}

internal void rbo_delete(u32 *rbo)
{
    glDeleteRenderbuffers(1, rbo);
}

internal void rbo_update(int window_width, int window_height, int samples)
{
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, window_width, window_height);
}

internal void vertex_layout_set(u32 index, int size, u32 stride, const void* ptr)
{
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, ptr);
}

internal RenderObject framebuffer_renderer_init(u32 shader)
{
    RenderObject framebuffer_renderer = {0};
    framebuffer_renderer.shader = shader;

    // These are in normalized device coordinates and will fill the screen
    f32 vertices[] =
    {    // pos       // tex
         1.0f,  1.0f, 1.0f, 1.0f, // top right
         1.0f, -1.0f, 1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f, 1.0f  // top left
    };

    u32 indices[] =
    {
        0, 1, 3,
        1, 2, 3
    };

    framebuffer_renderer.vao = vao_init();
    framebuffer_renderer.vbo = vbo_init(vertices, sizeof(vertices));
    framebuffer_renderer.ibo = ibo_init(indices, sizeof(indices));

    vertex_layout_set(0, 2, 4*sizeof(f32), 0);
    vertex_layout_set(1, 2, 4*sizeof(f32), (void*)(2*sizeof(f32)));

    vao_bind(0);    

    return framebuffer_renderer;
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
    
    vao_bind(0);

    return sprite_renderer;
}

internal RenderObject rect_renderer_init(u32 shader)
{
    RenderObject rect_renderer = {0};
    rect_renderer.shader = shader;

    f32 vertices[] =
    {
        // pos
        1.0f, 1.0f, // top right
        1.0f, 0.0f, // bottom right
        0.0f, 0.0f, // bottom left
        0.0f, 1.0f  // top left
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

    f32 vertices[] =
    {
        0.0f, 0.0f,
        1.0f, 1.0f
    };

    u32 indices[] =
    {
        0, 1
    };

    line_renderer.vbo = vbo_init(vertices, sizeof(vertices));
    line_renderer.ibo = ibo_init(indices, sizeof(indices));

    vertex_layout_set(0, 2, 2*sizeof(f32), 0);
    glBindVertexArray(0);

    return line_renderer;
}

internal RenderObject circle_renderer_init(u32 shader, u32 segs)
{
    RenderObject circle_renderer = {0};
    circle_renderer.shader = shader;
    circle_renderer.vao = vao_init();

    circle_renderer.vbo = vbo_init_empty();
    circle_renderer.ibo = ibo_init_empty();

    vertex_layout_set(0, 2, 2*sizeof(f32), 0);
    glBindVertexArray(0);

    return circle_renderer;
}

internal void render_object_delete(RenderObject* render_object)
{
    glDeleteVertexArrays(1, &render_object->vao);
    glDeleteBuffers(1, &render_object->vbo);
    glDeleteBuffers(1, &render_object->ibo);
    shader_delete(render_object->shader);
}

internal void framebuffer_attach_texture(Framebuffer* framebuffer, Texture texture, int samples)
{
    if (samples > 0)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture.id, 0);
    else
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id, 0);
}

internal void framebuffer_attach_renderbuffer(Framebuffer* framebuffer)
{
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebuffer->rbo);
}

// Generate render-to-texture framebuffer
internal Framebuffer framebuffer_init(u32 shader, int window_width, int window_height, int samples, b32 only_color)
{
    Framebuffer framebuffer = {0};
    framebuffer.id = fbo_init();

    framebuffer.texture = texture_generate(samples);
    texture_fill_empty_data(&framebuffer.texture, window_width, window_height, samples);
    framebuffer_attach_texture(&framebuffer, framebuffer.texture, samples);

    // NOTE(lucas): Rendbuffer is used for depth/stencil attachments.
    // The intermediate framebuffer only needs a color attachment
    if (!only_color)
    {
        framebuffer.rbo = rbo_init();
        rbo_update(window_width, window_height, samples);
        framebuffer_attach_renderbuffer(&framebuffer);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        ASSERT(0);

    fbo_unbind();    

    shader_bind(shader);

    return framebuffer;
}

internal void framebuffer_delete(Framebuffer* framebuffer)
{
    texture_delete(&framebuffer->texture);
    rbo_delete(&framebuffer->rbo);
    fbo_delete(&framebuffer->id);
}

void renderer_viewport(rect viewport)
{
    glViewport((int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height);
}

Renderer renderer_init(int viewport_width, int viewport_height)
{
    Renderer renderer = {0};

    glEnable(GL_MULTISAMPLE);

    renderer.viewport = rect_min_dim((v2){0.0f, 0.0f}, (v2){(f32)viewport_width, (f32)viewport_height});
    renderer.clear_color = (v4){0.0f, 0.0f, 0.0f, 1.0f};

    renderer.config.circle_line_segments = 128;
    renderer.config.msaa_level = 16;

    // Clamp MSAA samples to max samples supported by GPU
    GLint max_samples;
    glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
    if (renderer.config.msaa_level > max_samples)
        renderer.config.msaa_level = max_samples;

    // TODO(lucas): These relative paths might cause problems in the future
    u32 framebuffer_shader = shader_init("shaders/framebuffer.vert", "shaders/framebuffer.frag");
    u32 poly_shader        = shader_init("shaders/poly.vert", "shaders/poly.frag");
    u32 sprite_shader      = shader_init("shaders/sprite.vert", "shaders/sprite.frag");
    u32 font_shader        = shader_init("shaders/font.vert", "shaders/font.frag");

    renderer.line_renderer        = line_renderer_init(poly_shader);
    renderer.circle_renderer      = circle_renderer_init(poly_shader, renderer.config.circle_line_segments);
    renderer.rect_renderer        = rect_renderer_init(poly_shader);
    renderer.sprite_renderer      = sprite_renderer_init(sprite_shader);
    renderer.font_renderer        = font_renderer_init(font_shader);
    renderer.framebuffer_renderer = framebuffer_renderer_init(framebuffer_shader);
    
    renderer.framebuffer = framebuffer_init(framebuffer_shader, viewport_width, viewport_height,
                                            renderer.config.msaa_level, false);
    renderer.intermediate_framebuffer = framebuffer_init(framebuffer_shader, viewport_width, viewport_height, 0, true);

    return renderer;
}

void renderer_delete(Renderer* renderer)
{
    render_object_delete(&renderer->line_renderer);
    render_object_delete(&renderer->circle_renderer);
    render_object_delete(&renderer->rect_renderer);
    render_object_delete(&renderer->sprite_renderer);
    render_object_delete(&renderer->font_renderer);
    render_object_delete(&renderer->framebuffer_renderer);

    framebuffer_delete(&renderer->framebuffer);
    framebuffer_delete(&renderer->intermediate_framebuffer);
}

void renderer_new_frame(Renderer* renderer, Window window)
{
    if (renderer->config.wireframe_mode)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_MULTISAMPLE);
    renderer->viewport.width = (f32)window.width;
    renderer->viewport.height = (f32)window.height;

    fbo_bind(renderer->framebuffer.id);

    rect viewport = renderer->viewport;
    renderer_viewport(viewport);

    int msaa = renderer->config.msaa_level;
    texture_fill_empty_data(&renderer->framebuffer.texture, (int)viewport.width, (int)viewport.height, msaa);
    texture_fill_empty_data(&renderer->intermediate_framebuffer.texture, (int)viewport.width, (int)viewport.height, 0);

    rbo_bind(renderer->framebuffer.rbo);
    rbo_update((int)viewport.width, (int)viewport.height, msaa);
    rbo_unbind();

    m4 projection = m4_ortho(0.0f, viewport.width, 0.0f, viewport.height, -1.0f, 1.0f);

    // NOTE(lucas): Most shapes use the same shader,
    // so no need to set the uniform for each shape
    shader_set_m4(renderer->rect_renderer.shader,   "projection", projection, false);
    shader_set_m4(renderer->sprite_renderer.shader, "projection", projection, false);
    shader_set_m4(renderer->font_renderer.shader,   "projection", projection, false);

    // Clear viewport
    renderer_clear(renderer->clear_color);
}

void renderer_render(Renderer* renderer)
{
    rect viewport = renderer->viewport;

    // NOTE(lucas): If MSAA is used, blit the multisampled framebuffer onto the
    // intermediate framebuffer
    if (renderer->config.msaa_level > 0)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->framebuffer.id);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer->intermediate_framebuffer.id);
        glBlitFramebuffer((int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height,
                        (int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height,
                        GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    fbo_unbind();
    renderer_viewport(viewport);
    renderer_clear(renderer->clear_color);

    shader_bind(renderer->framebuffer_renderer.shader);
    vao_bind(renderer->framebuffer_renderer.vao);

    if (renderer->config.msaa_level > 0)
        texture_bind(&renderer->intermediate_framebuffer.texture, 0);
    else
        texture_bind(&renderer->framebuffer.texture, renderer->config.msaa_level);

    // NOTE(lucas): Turn wireframe mode off before rendering the screen texture.
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    vao_unbind();
}

void renderer_clear(v4 color)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(color.r, color.g, color.b, color.a);
}

void draw_line(Renderer* renderer, v2 start, v2 end, v4 color)
{
    v2 length = v2_abs(v2_sub(start, end));
    m4 model = m4_identity();
    model = m4_translate(model, (v3){start.x, start.y, 0.0f});
    model = m4_scale(model, (v3){length.x, length.y, 1.0f});

    shader_set_m4(renderer->line_renderer.shader, "model", model, false);
    shader_set_v4(renderer->line_renderer.shader, "color", color);

    glBindVertexArray(renderer->line_renderer.vao);
    glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void draw_circle(Renderer* renderer, v2 position, f32 radius, v4 color)
{
    m4 model = m4_identity();
    model = m4_translate(model, (v3){position.x, position.y, 0.0f});
    model = m4_scale(model, (v3){radius, radius, 1.0f});

    // Set model matrix and color shader values
    shader_set_m4(renderer->rect_renderer.shader, "model", model, false);
    shader_set_v4(renderer->rect_renderer.shader, "color", color);


    // The number of triangles is 2 less than the number of line segments or points
    u32 segs = renderer->config.circle_line_segments;
    u32 tris = segs - 2;
    u32 n_verts = 2*segs;
    u32 n_indices = 3*tris;
    f32* vertices = malloc(n_verts*sizeof(f32));
    u32* indices = malloc(n_indices*sizeof(f32));

    // Construct points from angles of tris
    f32 angle = 360.0f / segs;
    for (u32 i = 0; i < 2*segs; i += 2)
    {
        f32 a = glm_rad(angle*i);
        vertices[i] = cos_f32(a);
        vertices[i+1] = sin_f32(a);
    }

    // Construct tris using indices, where the first vertex is shared by all tris
    u32 index = 1;
    for (u32 i = 0; i < 3*tris; i+= 3)
    {
        indices[i] = 0;
        indices[i+1] = index++;
        indices[i+2] = index;
    }

    glBindVertexArray(renderer->circle_renderer.vao);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->circle_renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, n_verts*sizeof(f32), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->circle_renderer.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_indices*sizeof(u32), indices, GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, n_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    free(vertices);
    free(indices);
}

void draw_rect(Renderer* renderer, v2 position, v2 size, v4 color, f32 rotation)
{
    m4 model = m4_identity();
    model = m4_translate(model, (v3){position.x, position.y, 0.0f});

    // NOTE(lucas): The origin of a quad is at the top left,
    // but we want the origin to appear in the center of the quad
    // for rotation. So, before rotation, translate the quad right and down by half its size.
    // After the rotation, undo this translation.
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
