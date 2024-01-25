#include "alchemy/renderer/renderer.h"
#include "alchemy/state.h" // MAX_FILEPATH_LEN
#include "alchemy/util/math.h"
#include "alchemy/util/memory.h"
#include "alchemy/util/str.h"

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

internal RenderObject ui_renderer_init(u32 shader)
{
    RenderObject ui_renderer = {0};
    ui_renderer.shader = shader;
    return ui_renderer;
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
        // pos      // color
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // top right
        1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, // bottom right
        0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, // bottom left
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f  // top left
    };

    u32 indices[] =
    {
        0, 1, 3,
        1, 2, 3
    };

    rect_renderer.vao = vao_init();
    rect_renderer.vbo = vbo_init(vertices, sizeof(vertices));
    rect_renderer.ibo = ibo_init(indices, sizeof(indices));

    vertex_layout_set(0, 2, 6*sizeof(f32), 0);
    vertex_layout_set(1, 4, 6*sizeof(f32), (void*)(2*sizeof(f32)));
    
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

internal void renderer_gen_texture(Texture tex)
{
    if (!tex.data)
        return;

    glBindTexture(GL_TEXTURE_2D, tex.id);

    // TODO(lucas): Make options configurable
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = 0;
    switch(tex.channels)
    {
        case 1: format = GL_RED;  break;
        case 2: format = GL_RG;   break;
        case 3: format = GL_RGB;  break;
        case 4: format = GL_RGBA; break;
        default: break;
    }

    // TODO(lucas): Internal format is supposed to be like GL_RGBA8
    glTexImage2D(GL_TEXTURE_2D, 0, format, (int)tex.size.x, (int)tex.size.y, 0, format, GL_UNSIGNED_BYTE, tex.data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void output_quad(Renderer* renderer, v2 position, v2 origin, v2 size, v4 color, f32 rotation)
{
    m4 model = m4_identity();
    model = m4_translate(model, (v3){position.x, position.y, 0.0f});
    v2 delta = v2_sub(origin, position);

    if (rotation)
    {
        model = m4_translate(model, (v3){delta.x, delta.y, 0.0f});
        model = m4_rotate(model, glm_rad(rotation), (v3){0.0f, 0.0f, 1.0f});
        model = m4_translate(model, (v3){-delta.x, -delta.y, 0.0f});
    }

    model = m4_scale(model, (v3){(f32)size.x, (f32)size.y, 1.0f});

    shader_set_m4(renderer->rect_renderer.shader, "model", model, 0);
    shader_set_v4(renderer->rect_renderer.shader, "color", color);

    glBindVertexArray(renderer->rect_renderer.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void output_quad_outline(Renderer* renderer, v2 position, v2 origin, v2 size, v4 color, f32 rotation, f32 thickness)
{
    output_quad(renderer, position, origin, (v2){size.x, thickness}, color, rotation);
    output_quad(renderer, (v2){position.x + size.x - thickness, position.y}, origin, (v2){thickness, size.y}, color, rotation);
    output_quad(renderer, (v2){position.x, position.y + size.y - thickness}, origin, (v2){size.x, thickness}, color, rotation);
    output_quad(renderer, position, origin, (v2){thickness, size.y}, color, rotation);
}

void output_quad_gradient(Renderer* renderer, v2 position, v2 origin, v2 size, v4 color_left, v4 color_bottom,
                        v4 color_right, v4 color_top, f32 rotation)
{
    m4 model = m4_identity();
    model = m4_translate(model, (v3){position.x, position.y, 0.0f});
    v2 delta = v2_sub(origin, position);

    if (rotation)
    {
        model = m4_translate(model, (v3){delta.x, delta.y, 0.0f});
        model = m4_rotate(model, glm_rad(rotation), (v3){0.0f, 0.0f, 1.0f});
        model = m4_translate(model, (v3){-delta.x, -delta.y, 0.0f});
    }

    model = m4_scale(model, (v3){(f32)size.x, (f32)size.y, 1.0f});

    shader_set_m4(renderer->rect_renderer.shader, "model", model, 0);
    shader_set_v4(renderer->rect_renderer.shader, "color", color_white());

    f32 default_vertices[] =
    {
        // pos      // color
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // top right
        1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, // bottom right
        0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, // bottom left
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f  // top left
    };

    f32 gradient_vertices[] =
    {
        // pos      // color
        1.0f, 1.0f, color_right.r,  color_right.g,  color_right.b,  color_right.a,  // top right
        1.0f, 0.0f, color_bottom.r, color_bottom.g, color_bottom.b, color_bottom.a, // bottom right
        0.0f, 0.0f, color_left.r,   color_left.g,   color_left.b,   color_left.a,   // bottom left
        0.0f, 1.0f, color_top.r,    color_top.g,    color_top.b,    color_top.a     // top left
    };

    // TODO(lucas): Is there a better way to handle making sure vertex colors do not persist?
    glBindVertexArray(renderer->rect_renderer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->rect_renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gradient_vertices), gradient_vertices, GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(default_vertices), default_vertices, GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void output_line(Renderer* renderer, v2 start, v2 end, v4 color, f32 thickness)
{
    v2 length = v2_abs(v2_sub(start, end));
    v2 origin = v2_scale(v2_add(start, end), 0.5f);

    // NOTE(lucas): Horizontal and vertical lines need special treatment
    // since they will cause trig functions to be undefined
    v2 size = v2_zero();
    f32 rotation = 0.0f;

    // NOTE(lucas): atan is undefined for vertical lines,
    // so only call it if the line has slope
    if (length.x)
        rotation = atan_f32(length.y, length.x);

    if (length.x && length.y) // Diagonal line
        size = (v2){v2_mag(length), thickness};
    else if (length.x && !length.y) // Horizontal line
        size = (v2){length.x, thickness};
    else if (length.y && !length.x) // Vertical line
        size = (v2){thickness, length.y};

    output_quad(renderer, start, origin, size, color, glm_deg(rotation));
}

void output_circle(Renderer* renderer, v2 position, f32 radius, v4 color)
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
    f32* vertices = push_array(&renderer->scratch_arena, n_verts, f32);
    u32* indices = push_array(&renderer->scratch_arena, n_indices, u32);

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
}

internal RenderCommandBuffer render_command_buffer_alloc(MemoryArena* arena, usize max_size)
{
    RenderCommandBuffer result = {0};
    result.base = (u8*)push_size(arena, max_size);
    result.size = 0;
    result.max_size = max_size;
    return result;
}

internal void render_command_buffer_clear(RenderCommandBuffer* command_buffer)
{
    command_buffer->size = 0;
}

#define render_command_push(buffer, type) (type*)render_command_push_(buffer, sizeof(type), RENDER_COMMAND_##type)
internal RenderCommand* render_command_push_(RenderCommandBuffer* command_buffer, usize size, RenderCommandType type)
{
    RenderCommand* result = 0;
    if (command_buffer->size + size < command_buffer->max_size)
    {
        result = (RenderCommand*)(command_buffer->base + command_buffer->size);
        result->type = type;
        command_buffer->size += size;
    }
    else
    {
        INVALID_CODE_PATH();
    }
    return result;
}

internal void render_command_buffer_output(Renderer* renderer)
{
    RenderCommandBuffer* command_buffer = &renderer->command_buffer;
    for (usize base_address = 0; base_address < command_buffer->size;)
    {
        RenderCommand* header = (RenderCommand*)(command_buffer->base + base_address);
        switch(header->type)
        {
            case RENDER_COMMAND_RenderCommandLine:
            {
                RenderCommandLine* cmd = (RenderCommandLine*)header;
                output_line(renderer, cmd->start, cmd->end, cmd->color, cmd->thickness);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandQuad:
            {
                RenderCommandQuad* cmd = (RenderCommandQuad*)header;
                output_quad(renderer, cmd->position, cmd->origin, cmd->size, cmd->color, cmd->rotation);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandQuadOutline:
            {
                RenderCommandQuadOutline* cmd = (RenderCommandQuadOutline*)header;
                output_quad_outline(renderer, cmd->position, cmd->origin, cmd->size, cmd->color, cmd->rotation,
                                    cmd->thickness);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandQuadGradient:
            {
                RenderCommandQuadGradient* cmd = (RenderCommandQuadGradient*)header;
                output_quad_gradient(renderer, cmd->position, cmd->origin, cmd->size, cmd->color_left, cmd->color_bottom,
                                    cmd->color_right, cmd->color_top, cmd->rotation);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandCircle:
            {
                RenderCommandCircle* cmd = (RenderCommandCircle*)header;
                output_circle(renderer, cmd->position, cmd->radius, cmd->color);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandSprite:
            {
                RenderCommandSprite* cmd = (RenderCommandSprite*)header;
                output_sprite(renderer, cmd->sprite);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandText:
            {
                RenderCommandText* cmd = (RenderCommandText*)header;
                output_text(renderer, cmd->text);
                base_address += sizeof(*cmd);
            } break;

            INVALID_DEFAULT_CASE();
        }
    }    
}

internal void path_from_install_dir(char* path, char* dest)
{
    str_cat(ALCHEMY_INSTALL_PATH, str_len(ALCHEMY_INSTALL_PATH), path, str_len(path), dest, MAX_FILEPATH_LEN);
}

Renderer renderer_init(Window window, int viewport_width, int viewport_height, usize command_buffer_size)
{
    Renderer renderer = {0};

    opengl_init(window);

    glEnable(GL_MULTISAMPLE);

    renderer.command_buffer_arena = memory_arena_alloc(command_buffer_size);
    renderer.command_buffer = render_command_buffer_alloc(&renderer.command_buffer_arena, command_buffer_size);

    // TODO(lucas): What's the best way to initialize this? Maybe in config?
    // Don't want it to be necessary to define a memory arena to pass in to here.
    renderer.scratch_arena = memory_arena_alloc(MEGABYTES(4));

    renderer.viewport = rect_min_dim((v2){0.0f, 0.0f}, (v2){(f32)viewport_width, (f32)viewport_height});
    renderer.clear_color = (v4){0.0f, 0.0f, 0.0f, 1.0f};

    renderer.config.circle_line_segments = 128;
    renderer.config.msaa_level = 16;

    // Clamp MSAA samples to max samples supported by GPU
    GLint max_samples;
    glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
    if (renderer.config.msaa_level > max_samples)
        renderer.config.msaa_level = max_samples;

    // TODO(lucas): Better path joining that automatically inserts slashes
    char framebuffer_vert_shader_full_path[MAX_FILEPATH_LEN];
    char framebuffer_frag_shader_full_path[MAX_FILEPATH_LEN];
    char poly_vert_shader_full_path[MAX_FILEPATH_LEN];
    char poly_frag_shader_full_path[MAX_FILEPATH_LEN];
    char sprite_vert_shader_full_path[MAX_FILEPATH_LEN];
    char sprite_frag_shader_full_path[MAX_FILEPATH_LEN];
    char font_vert_shader_full_path[MAX_FILEPATH_LEN];
    char font_frag_shader_full_path[MAX_FILEPATH_LEN];
    char ui_vert_shader_full_path[MAX_FILEPATH_LEN];
    char ui_frag_shader_full_path[MAX_FILEPATH_LEN];

    path_from_install_dir("/res/shaders/framebuffer.vert", framebuffer_vert_shader_full_path);
    path_from_install_dir("/res/shaders/framebuffer.frag", framebuffer_frag_shader_full_path);
    path_from_install_dir("/res/shaders/poly.vert", poly_vert_shader_full_path);
    path_from_install_dir("/res/shaders/poly.frag", poly_frag_shader_full_path);
    path_from_install_dir("/res/shaders/sprite.vert", sprite_vert_shader_full_path);
    path_from_install_dir("/res/shaders/sprite.frag", sprite_frag_shader_full_path);
    path_from_install_dir("/res/shaders/font.vert", font_vert_shader_full_path);
    path_from_install_dir("/res/shaders/font.frag", font_frag_shader_full_path);
    path_from_install_dir("/res/shaders/ui.vert", ui_vert_shader_full_path);
    path_from_install_dir("/res/shaders/ui.frag", ui_frag_shader_full_path);

    u32 framebuffer_shader = shader_init(framebuffer_vert_shader_full_path, framebuffer_frag_shader_full_path);
    u32 poly_shader        = shader_init(poly_vert_shader_full_path, poly_frag_shader_full_path);
    u32 sprite_shader      = shader_init(sprite_vert_shader_full_path, sprite_frag_shader_full_path);
    u32 font_shader        = shader_init(font_vert_shader_full_path, font_frag_shader_full_path);
    u32 ui_shader          = shader_init(ui_vert_shader_full_path, ui_frag_shader_full_path);

    renderer.circle_renderer      = circle_renderer_init(poly_shader, renderer.config.circle_line_segments);
    renderer.rect_renderer        = rect_renderer_init(poly_shader);
    renderer.sprite_renderer      = sprite_renderer_init(sprite_shader);
    renderer.font_renderer        = font_renderer_init(font_shader);
    renderer.framebuffer_renderer = framebuffer_renderer_init(framebuffer_shader);
    renderer.ui_renderer          = ui_renderer_init(ui_shader);
    
    renderer.ui_render_state = ui_render_state_init(ui_shader);

    renderer.framebuffer = framebuffer_init(framebuffer_shader, viewport_width, viewport_height,
                                            renderer.config.msaa_level, false);
    renderer.intermediate_framebuffer = framebuffer_init(framebuffer_shader, viewport_width, viewport_height, 0, true);

    for (u32 i = 0; i < ARRAY_COUNT(renderer.tex_ids); ++i)
    {
        RenderID* tex_id = renderer.tex_ids + i;
        glGenTextures(1, &tex_id->id);
    }

    return renderer;
}

void renderer_delete(Renderer* renderer)
{
    render_object_delete(&renderer->circle_renderer);
    render_object_delete(&renderer->rect_renderer);
    render_object_delete(&renderer->sprite_renderer);
    render_object_delete(&renderer->font_renderer);
    render_object_delete(&renderer->framebuffer_renderer);

    framebuffer_delete(&renderer->framebuffer);
    framebuffer_delete(&renderer->intermediate_framebuffer);
    ui_render_state_shutdown(&renderer->ui_render_state);
}

void renderer_new_frame(Renderer* renderer, Window window)
{
    if (renderer->config.wireframe_mode)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_MULTISAMPLE);

    // NOTE(lucas): If the user does not call renderer_viewport to set the viewport themselves,
    // fit the viewport to the window.
    if (rect_is_zero(renderer->viewport))
    {
        rect viewport = rect_min_dim(v2_zero(), (v2){(f32)window.width, (f32)window.height});
        renderer_viewport(renderer, viewport);
    }

    fbo_bind(renderer->framebuffer.id);

    rect viewport = renderer->viewport;
    int msaa = renderer->config.msaa_level;
    texture_fill_empty_data(&renderer->framebuffer.texture, (int)viewport.width, (int)viewport.height, msaa);
    texture_fill_empty_data(&renderer->intermediate_framebuffer.texture, (int)viewport.width, (int)viewport.height, 0);

    rbo_bind(renderer->framebuffer.rbo);
    rbo_update((int)viewport.width, (int)viewport.height, msaa);
    rbo_unbind();

    // NOTE(lucas): If the viewport does not start at (0, 0), offset the projection matrix by the viewport origin
    m4 projection = m4_ortho(viewport.x, viewport.x + viewport.width, viewport.y, viewport.y + viewport.height,
                             -1.0f, 1.0f);

    // NOTE(lucas): Most shapes use the same shader,
    // so no need to set the uniform for each shape
    shader_set_m4(renderer->rect_renderer.shader,   "projection", projection, false);
    shader_set_m4(renderer->sprite_renderer.shader, "projection", projection, false);
    shader_set_m4(renderer->font_renderer.shader,   "projection", projection, false);

    ui_new_frame(renderer, window.width, window.height);

    // Clear viewport
    renderer_clear(color_black());
}

void renderer_render(Renderer* renderer)
{
    for (u32 i = 0; i < ARRAY_COUNT(renderer->tex_ids); ++i)
    {
        RenderID* tex_id = renderer->tex_ids + i;
        if (tex_id->used)
        {
            Texture tex = renderer->textures_to_generate[i];
            renderer_gen_texture(tex);
            glGenTextures(1, &tex_id->id);
        }
    }

    rect viewport = renderer->viewport;

    render_command_buffer_output(renderer);

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

    // NOTE(lucas): Invalidate the viewport so that the new frame call will set it correctly to
    // window dimensions if the user does not resize the viewport themselves 
    renderer->viewport = rect_zero();
    memory_arena_clear(&renderer->scratch_arena);
    memory_arena_clear(&renderer->command_buffer_arena);
    render_command_buffer_clear(&renderer->command_buffer);

    for (u32 i = 0; i < ARRAY_COUNT(renderer->tex_ids); ++i)
        renderer->textures_to_generate[i] = (Texture){0};

    // TODO(lucas): Use renderer AA settings
    ui_render(renderer, NK_ANTI_ALIASING_ON);
    
}

void renderer_viewport(Renderer* renderer, rect viewport)
{
    renderer->viewport = viewport;
    glViewport((int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height);
}

void renderer_clear(v4 color)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(color.r, color.g, color.b, color.a);
}

void draw_line(Renderer* renderer, v2 start, v2 end, v4 color, f32 thickness)
{
    RenderCommandLine* cmd = render_command_push(&renderer->command_buffer, RenderCommandLine);
    if (!cmd)
        return;
    cmd->thickness = thickness;
    cmd->start = start;
    cmd->end = end;
    cmd->color = color;
}

void draw_quad(Renderer* renderer, v2 position, v2 size, v4 color, f32 rotation)
{
    RenderCommandQuad* cmd = render_command_push(&renderer->command_buffer, RenderCommandQuad);
    if (!cmd)
        return;
    v2 origin = v2_add(position, v2_scale(size, 0.5f));
    cmd->position = position;
    cmd->origin = origin;
    cmd->size = size;
    cmd->color = color;
    cmd->rotation = rotation;
}

void draw_quad_outline(Renderer* renderer, v2 position, v2 size, v4 color, f32 rotation, f32 thickness)
{
    RenderCommandQuadOutline* cmd = render_command_push(&renderer->command_buffer, RenderCommandQuadOutline);
    if (!cmd)
        return;
    v2 origin = v2_add(position, v2_scale(size, 0.5f));
    cmd->position = position;
    cmd->origin = origin;
    cmd->size = size;
    cmd->color = color;
    cmd->rotation = rotation;
    cmd->thickness = thickness;
}

void draw_quad_gradient(Renderer* renderer, v2 position, v2 size, v4 color_left, v4 color_bottom, v4 color_right, v4 color_top, f32 rotation)
{
    RenderCommandQuadGradient* cmd = render_command_push(&renderer->command_buffer, RenderCommandQuadGradient);
    if (!cmd)
        return;
    v2 origin = v2_add(position, v2_scale(size, 0.5f));
    cmd->position = position;
    cmd->origin = origin;
    cmd->size = size;
    cmd->color_left = color_left;
    cmd->color_bottom = color_bottom;
    cmd->color_right = color_right;
    cmd->color_top = color_top;
    cmd->rotation = rotation;
}

void draw_circle(Renderer* renderer, v2 position, f32 radius, v4 color)
{
    RenderCommandCircle* cmd = render_command_push(&renderer->command_buffer, RenderCommandCircle);
    if (!cmd)
        return;
    cmd->position = position;
    cmd->radius = radius;
    cmd->color = color;
}

void draw_sprite(Renderer* renderer, Sprite sprite)
{
    RenderCommandSprite* cmd = render_command_push(&renderer->command_buffer, RenderCommandSprite);
    if (!cmd)
        return;
    cmd->sprite = sprite;
}

void draw_text(Renderer* renderer, Text text)
{
    RenderCommandText* cmd = render_command_push(&renderer->command_buffer, RenderCommandText);
    if (!cmd)
        return;
    char* text_copy = str_copy(text.string, &renderer->scratch_arena);
    cmd->text = text;
    cmd->text.string = text_copy;
}

u32 renderer_next_tex_id(Renderer* renderer)
{
    u32 id = 0;
    
    if (renderer->tex_index <= ARRAY_COUNT(renderer->tex_ids))
    {
        id = renderer->tex_ids[renderer->tex_index].id;
        renderer->tex_ids[renderer->tex_index].used = true;
    }

    return id;
}

void renderer_push_texture(Renderer* renderer, Texture tex)
{
    if (renderer->tex_index <= ARRAY_COUNT(renderer->tex_ids))
    {
        renderer->textures_to_generate[renderer->tex_index] = tex;
        ++renderer->tex_index;
    }
}
