#include "alchemy/renderer/renderer.h"
#include "alchemy/state.h" // MAX_FILEPATH_LEN
#include "alchemy/util/math.h"
#include "alchemy/util/memory.h"
#include "alchemy/util/str.h"

#include <glad/glad.h>
#include <stb_image/stb_image.h>

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

internal u32 vbo_init(f32* vertices, size bytes)
{
    u32 vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, bytes, vertices, GL_STATIC_DRAW);

    return vbo;
}

internal u32 vbo_init_empty(void)
{
    u32 vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    return vbo;
}

internal u32 ibo_init(u32* indices, size bytes)
{
    u32 ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytes, indices, GL_STATIC_DRAW);

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

void vertex_layout_set(u32 index, int size, u32 stride, const void* ptr)
{
    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, ptr);
}

internal void stencil_buffer_enable_write(void)
{
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
}

internal void stencil_buffer_disable_write(void)
{
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
}

internal RenderObject framebuffer_renderer_init(u32 shader)
{
    RenderObject framebuffer_renderer = {0};
    framebuffer_renderer.shader = shader;

    // These are in normalized device coordinates and will fill the screen
    f32 vertices[] =
    {    // pos       // tex
        -1.0f, -1.0f, 0.0f, 0.0f, // bottom left
         1.0f, -1.0f, 1.0f, 0.0f, // bottom right
         1.0f,  1.0f, 1.0f, 1.0f, // top right
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
        0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        1.0f, 1.0f, 1.0f, 0.0f, // bottom right
        1.0f, 0.0f, 1.0f, 1.0f, // top right
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

internal RenderObject triangle_renderer_init(u32 shader)
{
    RenderObject triangle_renderer = {0};
    triangle_renderer.shader = shader;

    u32 indices[] =
    {
        0, 1, 2
    };

    triangle_renderer.vao = vao_init();
    triangle_renderer.vbo = vbo_init_empty();
    triangle_renderer.ibo = ibo_init(indices, sizeof(indices));

    vertex_layout_set(0, 2, 6*sizeof(f32), 0);
    vertex_layout_set(1, 4, 6*sizeof(f32), (void*)(2*sizeof(f32)));

    vao_bind(0);

    return triangle_renderer;
}

internal RenderObject quad_renderer_init(u32 shader)
{
    RenderObject quad_renderer = {0};
    quad_renderer.shader = shader;

    f32 vertices[] =
    {
        // pos      // color
        0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, // bottom left
        1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, // bottom right
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // top right
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f  // top left
    };

    u32 indices[] =
    {
        0, 1, 3,
        1, 2, 3
    };

    quad_renderer.vao = vao_init();
    quad_renderer.vbo = vbo_init(vertices, sizeof(vertices));
    quad_renderer.ibo = ibo_init(indices, sizeof(indices));

    vertex_layout_set(0, 2, 6*sizeof(f32), 0);
    vertex_layout_set(1, 4, 6*sizeof(f32), (void*)(2*sizeof(f32)));

    vao_bind(0);
    
    return quad_renderer;
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

    vao_bind(0);

    return font_renderer;
}

internal RenderObject circle_renderer_init(u32 shader, u32 segs)
{
    RenderObject circle_renderer = {0};
    circle_renderer.shader = shader;
    circle_renderer.vao = vao_init();

    circle_renderer.vbo = vbo_init_empty();
    circle_renderer.ibo = ibo_init_empty();

    vertex_layout_set(0, 2, 6*sizeof(f32), 0);
    vertex_layout_set(1, 4, 6*sizeof(f32), (void*)(2*sizeof(f32)));
    vao_bind(0);

    return circle_renderer;
}

internal void render_object_delete(RenderObject* render_object)
{
    glDeleteVertexArrays(1, &render_object->vao);
    glDeleteBuffers(1, &render_object->vbo);
    glDeleteBuffers(1, &render_object->ibo);

    // TODO(lucas): Some render objects share shaders, and deleting twice gives a GL_INVALID_VALUE error
    // shader_delete(render_object->shader);
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

    // NOTE(lucas): Renderbuffer is used for depth/stencil attachments.
    // The intermediate framebuffer only needs a color attachment
    if (!only_color)
    {
        framebuffer.rbo = rbo_init();
        rbo_update(window_width, window_height, samples);
        framebuffer_attach_renderbuffer(&framebuffer);
    }

    GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(framebuffer_status)
    {
        case GL_FRAMEBUFFER_COMPLETE: break;

        case GL_FRAMEBUFFER_UNDEFINED:
        {
            ASSERT(0, "OpenGL framebuffer error: Default framebuffer chosen, but does not exist");
        } break;

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        {
            ASSERT(0, "OpenGL framebuffer error: At least one framebuffer attachment is incomplete");
        } break;

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        {
            ASSERT(0, "OpenGL framebuffer error: Framebuffer does not have at least one image attached");
        } break;

        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        {
            ASSERT(0, "OpenGL framebuffer error: Draw buffer has a missing or invalid color attachment point");
        } break;

        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        {
            ASSERT(0, "OpenGL framebuffer error: Read buffer has a missing or invalid color attachment point");
        } break;

        case GL_FRAMEBUFFER_UNSUPPORTED:
        {
            ASSERT(0, "OpenGL framebuffer error: Internal format combination of attached images is unsupported");
        } break;

        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        {
            ASSERT(0, "OpenGL framebuffer error: The number of samples is not the same for all attached renderbuffers, "
                      "all attached textures, or the renderbuffer and texture samples do not match");
        } break;

        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
        {
            ASSERT(0, "OpenGL framebuffer error: The framebuffer is layered, but a populated attachment is not layered; "
                      "or all populated color attachments are not from textures of the same target");
        } break;

        default: ASSERT(0, "OpenGL framebuffer error: Unexpected value from glCheckFramebufferStatus()"); break;
    }
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    

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

internal void output_quad(Renderer* renderer, RenderCommandQuad* cmd);

internal void output_line(Renderer* renderer, RenderCommandLine* cmd)
{
    v2 delta = v2_sub(cmd->end, cmd->start);

    // NOTE(lucas): Horizontal and vertical lines need special treatment
    // since they will cause trig functions to be undefined
    v2 size = v2_zero();
    f32 initial_rotation = 0.0f;

    // NOTE(lucas): atan is undefined for vertical lines,
    // so only call it if the line has slope
    if (delta.x)
        initial_rotation = atan_f32(delta.y, delta.x);

    if (delta.x && delta.y) // Diagonal line
        size = (v2){v2_mag(delta), cmd->thickness};
    else if (delta.x && !delta.y) // Horizontal line
        size = (v2){delta.x, cmd->thickness};
    else if (delta.y && !delta.x) // Vertical line
        size = (v2){cmd->thickness, delta.y};

    // TODO(lucas): The initial rotation needs to be about the starting point,
    // white the additional rotation needs to be about the origin
    RenderCommandQuad quad_cmd = {RENDER_COMMAND_RenderCommandQuad, cmd->start, cmd->origin, size, cmd->color,
                                  glm_deg(initial_rotation) + cmd->rotation};
    output_quad(renderer, &quad_cmd);
}

internal void output_triangle(Renderer* renderer, RenderCommandTriangle* cmd)
{
    v2 min_point = v2_full(F32_MAX);
    v2 max_point = v2_full(-F32_MAX);

    if (cmd->a.x < min_point.x) min_point.x = cmd->a.x;
    if (cmd->b.x < min_point.x) min_point.x = cmd->b.x;
    if (cmd->c.x < min_point.x) min_point.x = cmd->c.x;

    if (cmd->a.y < min_point.y) min_point.y = cmd->a.y;
    if (cmd->b.y < min_point.y) min_point.y = cmd->b.y;
    if (cmd->c.y < min_point.y) min_point.y = cmd->c.y;

    if (cmd->a.x > max_point.x) max_point.x = cmd->a.x;
    if (cmd->b.x > max_point.x) max_point.x = cmd->b.x;
    if (cmd->c.x > max_point.x) max_point.x = cmd->c.x;

    if (cmd->a.y > max_point.y) max_point.y = cmd->a.y; 
    if (cmd->b.y > max_point.y) max_point.y = cmd->b.y; 
    if (cmd->c.y > max_point.y) max_point.y = cmd->c.y; 

    v2 scale = v2_sub(max_point, min_point);

    v2 a_norm = {(cmd->a.x - min_point.x) / scale.x, (cmd->a.y - min_point.y) / scale.y};
    v2 b_norm = {(cmd->b.x - min_point.x) / scale.x, (cmd->b.y - min_point.y) / scale.y};
    v2 c_norm = {(cmd->c.x - min_point.x) / scale.x, (cmd->c.y - min_point.y) / scale.y};

    // TODO(lucas): Current triangle being drawn should go off the screen to the left.
    // Should this go from 0 to 1?
    f32 vertices[] =
    {
        // pos              // color
        a_norm.x, a_norm.y, 1.0f, 1.0f, 1.0f, 1.0f, // bottom left
        b_norm.x, b_norm.y, 1.0f, 1.0f, 1.0f, 1.0f, // bottom right
        c_norm.x, c_norm.y, 1.0f, 1.0f, 1.0f, 1.0f  // top
    };

    vao_bind(renderer->triangle_renderer.vao);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->triangle_renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    m4 model = m4_identity();
    model = m4_translate(model, (v3){min_point.x, min_point.y, 0.0f});
    v2 delta = v2_abs(v2_sub(cmd->origin, min_point));

    if (cmd->rotation)
    {
        model = m4_translate(model, (v3){delta.x, delta.y, 0.0f});
        model = m4_rotate(model, glm_rad(-cmd->rotation), (v3){0.0f, 0.0f, 1.0f});
        model = m4_translate(model, (v3){-delta.x, -delta.y, 0.0f});
    }

    model = m4_scale(model, (v3){scale.x, scale.y, 1.0f});

    shader_set_m4(renderer->triangle_renderer.shader, "model", model, false);
    shader_set_v4(renderer->triangle_renderer.shader, "color", cmd->color);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    vao_bind(0);
}

internal void output_triangle_outline(Renderer* renderer, RenderCommandTriangleOutline* cmd)
{
    /* NOTE(lucas): To find the vertices of the shrunken triangle, first find incenter of the triangle (the center of
     * the inscribed circle). Then, find the inradius, the radius of the inscribed circle. Trivially, the two triangles
     * share angle bisectors and thus incenters. So, we want to find a coefficient that gives us new vertices based on
     * moving X units along the inradius, where X is the outline thickness, which can be done with the all-too-familiar
     * linear blend. If A is the old vertex, A' is the new vertx, Q is the incenter, and k is the coefficient, we have
     * A' = A(1-k) + Qk
     * More details found here: https://math.stackexchange.com/questions/17561/how-to-shrink-a-triangle
     */
    f32 ab = v2_mag(v2_sub(cmd->b, cmd->a));
    f32 bc = v2_mag(v2_sub(cmd->c, cmd->b));
    f32 ca = v2_mag(v2_sub(cmd->c, cmd->a));

    v2 incenter = {(ab*cmd->c.x + bc*cmd->a.x + ca*cmd->b.x) / (ab + bc + ca),
                   (ab*cmd->c.y + bc*cmd->a.y + ca*cmd->b.y) / (ab + bc + ca)};

    f32 semiperimeter = (ab + bc + ca) / 2.0f;
    f32 inradius = sqrt_f32(((semiperimeter-ab)*(semiperimeter-bc)*(semiperimeter-ca)) / semiperimeter);
    f32 k = cmd->thickness / inradius;
    v2 qk = v2_scale(incenter, k);

    v2 new_a = v2_add(v2_scale(cmd->a, 1.0f-k), qk);
    v2 new_b = v2_add(v2_scale(cmd->b, 1.0f-k), qk);
    v2 new_c = v2_add(v2_scale(cmd->c, 1.0f-k), qk);

    RenderCommandTriangle transparent_cmd = {RENDER_COMMAND_RenderCommandTriangle, new_a, new_b, new_c, cmd->origin,
                                             color_transparent(), cmd->rotation};
    RenderCommandTriangle outline_cmd = {RENDER_COMMAND_RenderCommandTriangle, cmd->a, cmd->b, cmd->c, cmd->origin,
                                         cmd->color, cmd->rotation};

    glEnable(GL_DEPTH_TEST);
    stencil_buffer_enable_write();
    output_triangle(renderer, &transparent_cmd);

    glDisable(GL_DEPTH_TEST);
    stencil_buffer_disable_write();
    renderer->triangle_renderer.shader = renderer->poly_border_shader;
    output_triangle(renderer, &outline_cmd);
    stencil_buffer_enable_write();
    renderer->triangle_renderer.shader = renderer->poly_shader;
}

// TODO(lucas): Think about pulling out common code and default vertices for shape variants
internal void output_triangle_gradient(Renderer* renderer, RenderCommandTriangleGradient* cmd)
{
    v2 min_point = v2_full(F32_MAX);
    v2 max_point = v2_full(-F32_MAX);

    if (cmd->a.x < min_point.x) min_point.x = cmd->a.x;
    if (cmd->b.x < min_point.x) min_point.x = cmd->b.x;
    if (cmd->c.x < min_point.x) min_point.x = cmd->c.x;

    if (cmd->a.y < min_point.y) min_point.y = cmd->a.y;
    if (cmd->b.y < min_point.y) min_point.y = cmd->b.y;
    if (cmd->c.y < min_point.y) min_point.y = cmd->c.y;

    if (cmd->a.x > max_point.x) max_point.x = cmd->a.x;
    if (cmd->b.x > max_point.x) max_point.x = cmd->b.x;
    if (cmd->c.x > max_point.x) max_point.x = cmd->c.x;

    if (cmd->a.y > max_point.y) max_point.y = cmd->a.y; 
    if (cmd->b.y > max_point.y) max_point.y = cmd->b.y; 
    if (cmd->c.y > max_point.y) max_point.y = cmd->c.y; 

    v2 scale = v2_sub(max_point, min_point);

    v2 a_norm = {(cmd->a.x - min_point.x) / scale.x, (cmd->a.y - min_point.y) / scale.y};
    v2 b_norm = {(cmd->b.x - min_point.x) / scale.x, (cmd->b.y - min_point.y) / scale.y};
    v2 c_norm = {(cmd->c.x - min_point.x) / scale.x, (cmd->c.y - min_point.y) / scale.y};

    m4 model = m4_identity();
    model = m4_translate(model, (v3){min_point.x, min_point.y, 0.0f});
    v2 delta = v2_abs(v2_sub(cmd->origin, min_point));

    if (cmd->rotation)
    {
        model = m4_translate(model, (v3){delta.x, delta.y, 0.0f});
        model = m4_rotate(model, glm_rad(-cmd->rotation), (v3){0.0f, 0.0f, 1.0f});
        model = m4_translate(model, (v3){-delta.x, -delta.y, 0.0f});
    }

    model = m4_scale(model, (v3){scale.x, scale.y, 1.0f});

    shader_set_m4(renderer->triangle_renderer.shader, "model", model, false);
    shader_set_v4(renderer->triangle_renderer.shader, "color", color_white());

    f32 default_vertices[] =
    {
        // pos              // color
        a_norm.x, a_norm.y, 1.0f, 1.0f, 1.0f, 1.0f, // bottom left
        b_norm.x, b_norm.y, 1.0f, 1.0f, 1.0f, 1.0f, // bottom right
        c_norm.x, c_norm.y, 1.0f, 1.0f, 1.0f, 1.0f  // top
    };

    f32 gradient_vertices[] =
    {
        // pos              // color
        a_norm.x, a_norm.y, cmd->color_a.r, cmd->color_a.g, cmd->color_a.b, cmd->color_a.a, // bottom left
        b_norm.x, b_norm.y, cmd->color_b.r, cmd->color_b.g, cmd->color_b.b, cmd->color_b.a, // bottom right
        c_norm.x, c_norm.y, cmd->color_c.r, cmd->color_c.g, cmd->color_c.b, cmd->color_c.a  // top
    };

    vao_bind(renderer->triangle_renderer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->triangle_renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gradient_vertices), gradient_vertices, GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(default_vertices), default_vertices, GL_STATIC_DRAW);
    vao_bind(0);    
}

internal void output_quad(Renderer* renderer, RenderCommandQuad* cmd)
{
    m4 model = m4_identity();
    model = m4_translate(model, (v3){cmd->position.x, cmd->position.y, 0.0f});
    v2 delta = v2_sub(cmd->origin, cmd->position);

    if (cmd->rotation)
    {
        model = m4_translate(model, (v3){delta.x, delta.y, 0.0f});
        model = m4_rotate(model, glm_rad(-cmd->rotation), (v3){0.0f, 0.0f, 1.0f});
        model = m4_translate(model, (v3){-delta.x, -delta.y, 0.0f});
    }

    model = m4_scale(model, (v3){(f32)cmd->size.x, (f32)cmd->size.y, 1.0f});

    shader_set_m4(renderer->quad_renderer.shader, "model", model, false);
    shader_set_v4(renderer->quad_renderer.shader, "color", cmd->color);

    vao_bind(renderer->quad_renderer.vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    vao_bind(0);
}

internal void output_quad_outline(Renderer* renderer, RenderCommandQuadOutline* cmd)
{
    v2 new_pos = v2_add(cmd->position, v2_full(cmd->thickness));
    v2 new_size = v2_sub(cmd->size, v2_full(2.0f*cmd->thickness));

    RenderCommandQuad transparent_cmd = {RENDER_COMMAND_RenderCommandQuad, new_pos, cmd->origin, new_size,
                                         color_transparent(), cmd->rotation};
    RenderCommandQuad outline_cmd = {RENDER_COMMAND_RenderCommandQuad, cmd->position, cmd->origin, cmd->size,
                                     cmd->color, cmd->rotation};

    glEnable(GL_DEPTH_TEST);
    stencil_buffer_enable_write();
    output_quad(renderer, &transparent_cmd);

    glDisable(GL_DEPTH_TEST);
    stencil_buffer_disable_write();
    renderer->quad_renderer.shader = renderer->poly_border_shader;
    output_quad(renderer, &outline_cmd);
    stencil_buffer_enable_write();
    renderer->quad_renderer.shader = renderer->poly_shader;
}

internal void output_quad_gradient(Renderer* renderer, RenderCommandQuadGradient* cmd)
{
    m4 model = m4_identity();
    model = m4_translate(model, (v3){cmd->position.x, cmd->position.y, 0.0f});
    v2 delta = v2_sub(cmd->origin, cmd->position);

    if (cmd->rotation)
    {
        model = m4_translate(model, (v3){delta.x, delta.y, 0.0f});
        model = m4_rotate(model, glm_rad(-cmd->rotation), (v3){0.0f, 0.0f, 1.0f});
        model = m4_translate(model, (v3){-delta.x, -delta.y, 0.0f});
    }

    model = m4_scale(model, (v3){(f32)cmd->size.x, (f32)cmd->size.y, 1.0f});

    shader_set_m4(renderer->quad_renderer.shader, "model", model, false);
    shader_set_v4(renderer->quad_renderer.shader, "color", color_white());

    f32 default_vertices[] =
    {
        // pos      // color
        0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, // bottom left
        1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, // bottom right
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // top right
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f  // top left
    };

    f32 gradient_vertices[] =
    {
        // pos      // color
        0.0f, 0.0f, cmd->color_bl.r, cmd->color_bl.g, cmd->color_bl.b, cmd->color_bl.a, // bottom left
        1.0f, 0.0f, cmd->color_br.r, cmd->color_br.g, cmd->color_br.b, cmd->color_br.a, // bottom right
        1.0f, 1.0f, cmd->color_tr.r, cmd->color_tr.g, cmd->color_tr.b, cmd->color_tr.a, // top right
        0.0f, 1.0f, cmd->color_tl.r, cmd->color_tl.g, cmd->color_tl.b, cmd->color_tl.a  // top left
    };

    // TODO(lucas): Is there a better way to handle making sure vertex colors do not persist?
    vao_bind(renderer->quad_renderer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->quad_renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gradient_vertices), gradient_vertices, GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(default_vertices), default_vertices, GL_STATIC_DRAW);
    vao_bind(0);
}

internal void output_circle(Renderer* renderer, RenderCommandCircle* cmd)
{
    m4 model = m4_identity();
    model = m4_translate(model, (v3){cmd->center.x, cmd->center.y, 0.0f});
    model = m4_scale(model, (v3){cmd->radius, cmd->radius, 1.0f});

    shader_set_m4(renderer->circle_renderer.shader, "model", model, false);
    shader_set_v4(renderer->circle_renderer.shader, "color", cmd->color);

    u32 segs = renderer->config.circle_line_segments;
    u32 tris = segs - 2;
    u32 n_verts = 6*segs;
    u32 n_indices = 3*tris;
    f32* vertices = push_array(&renderer->scratch_arena, n_verts, f32);
    u32* indices = push_array(&renderer->scratch_arena, n_indices, u32);

    // Construct points from angles of tris
    f32 angle_delta = 360.0f / segs;
    u32 index = 0;
    for (u32 i = 0; i < segs; ++i)
    {
        f32 a = glm_rad(angle_delta*i);
        vertices[index++] = cos_f32(a);
        vertices[index++] = sin_f32(a);

        vertices[index++] = cmd->color.r;
        vertices[index++] = cmd->color.g;
        vertices[index++] = cmd->color.b;
        vertices[index++] = cmd->color.a;
    }

    // Construct tris using indices, where the first vertex is shared by all tris
    index = 1;
    for (u32 i = 0; i < n_indices; i += 3)
    {
        indices[i] = 0;
        indices[i+1] = index++;
        indices[i+2] = index;
    }

    vao_bind(renderer->circle_renderer.vao);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->circle_renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, n_verts*sizeof(f32), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->circle_renderer.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_indices*sizeof(u32), indices, GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, n_indices, GL_UNSIGNED_INT, 0);
    vao_bind(0);
}

internal void output_circle_outline(Renderer* renderer, RenderCommandCircleOutline* cmd)
{
    RenderCommandCircle transparent_cmd = {RENDER_COMMAND_RenderCommandCircle, cmd->center,
                                           color_transparent(), cmd->radius - cmd->thickness};
    RenderCommandCircle outline_cmd = {RENDER_COMMAND_RenderCommandCircle, cmd->center, cmd->color, cmd->radius};

    glEnable(GL_DEPTH_TEST);
    stencil_buffer_enable_write();
    output_circle(renderer, &transparent_cmd);

    glDisable(GL_DEPTH_TEST);
    stencil_buffer_disable_write();
    renderer->circle_renderer.shader = renderer->poly_border_shader;
    output_circle(renderer, &outline_cmd);
    stencil_buffer_enable_write();
    renderer->circle_renderer.shader = renderer->poly_shader;
}

internal void output_circle_sector(Renderer* renderer, RenderCommandCircleSector* cmd)
{
    m4 model = m4_identity();
    model = m4_translate(model, (v3){cmd->center.x, cmd->center.y, 0.0f});
    model = m4_rotate(model, glm_rad(cmd->rotation), (v3){0.0f, 0.0f, 1.0f});
    model = m4_scale(model, (v3){cmd->radius, cmd->radius, 1.0f});

    shader_set_m4(renderer->circle_renderer.shader, "model", model, false);
    shader_set_v4(renderer->circle_renderer.shader, "color", cmd->color);

    u32 segs = renderer->config.circle_line_segments;
    u32 tris = segs;
    u32 n_verts = 6*(segs + 2);
    u32 n_indices = 3*tris;
    f32* vertices = push_array(&renderer->scratch_arena, n_verts, f32);
    u32* indices = push_array(&renderer->scratch_arena, n_indices, u32);

    // NOTE(lucas): For drawing circle sectors, it is easiest for vertices to share the center of the circle.
    vertices[0] = 0.0f;
    vertices[1] = 0.0f;
    vertices[2] = cmd->color.r;
    vertices[3] = cmd->color.g;
    vertices[4] = cmd->color.b;
    vertices[5] = cmd->color.a;

    // Construct points from angles of tris
    f32 angle_delta = abs_f32(cmd->end_angle - cmd->start_angle) / segs;
    u32 iterations = n_verts/2;
    u32 index = 6;
    for (u32 i = 0; i < iterations; ++i)
    {
        f32 a = glm_rad(cmd->start_angle + angle_delta*i);
        vertices[index++] = cos_f32(a);
        vertices[index++] = sin_f32(a);

        vertices[index++] = cmd->color.r;
        vertices[index++] = cmd->color.g;
        vertices[index++] = cmd->color.b;
        vertices[index++] = cmd->color.a;
    }

    // Construct tris using indices, where the first vertex is shared by all tris
    index = 1;
    for (u32 i = 0; i < n_indices; i += 3)
    {
        indices[i] = 0;
        indices[i+1] = index++;
        indices[i+2] = index;
    }

    vao_bind(renderer->circle_renderer.vao);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->circle_renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, n_verts*sizeof(f32), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->circle_renderer.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_indices*sizeof(u32), indices, GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, n_indices, GL_UNSIGNED_INT, 0);
    vao_bind(0);
}

internal void output_ring(Renderer* renderer, RenderCommandRing* cmd)
{
    // NOTE(lucas): Outer radius must be positive and larger than inner radius
    if (cmd->inner_radius > cmd->outer_radius)
    {
        f32 temp = cmd->inner_radius;
        cmd->inner_radius = cmd->outer_radius;
        cmd->outer_radius = temp;
    }

    if (cmd->outer_radius <= 0.0f)
        cmd->outer_radius = 0.1f;

    m4 model = m4_identity();
    model = m4_translate(model, (v3){cmd->center.x, cmd->center.y, 0.0f});
    model = m4_rotate(model, glm_rad(cmd->rotation), (v3){0.0f, 0.0f, 1.0f});
    model = m4_scale(model, (v3){cmd->outer_radius, cmd->outer_radius, 1.0f});

    shader_set_m4(renderer->circle_renderer.shader, "model", model, false);
    shader_set_v4(renderer->circle_renderer.shader, "color", cmd->color);

    u32 segs = renderer->config.circle_line_segments;
    u32 tris = segs;
    u32 n_verts = 6*(segs + 2);
    u32 n_indices = 3*tris;
    f32* vertices = push_array(&renderer->scratch_arena, n_verts, f32);
    u32* indices = push_array(&renderer->scratch_arena, n_indices, u32);

    // Construct points from angles of tris
    f32 angle_delta = abs_f32(cmd->end_angle - cmd->start_angle) / segs;
    u32 index = 0;
    u32 iterations = n_verts/2;
    f32 k = cmd->inner_radius / cmd->outer_radius;
    for (u32 i = 0; i < iterations; i += 2)
    {
        f32 a = glm_rad(cmd->start_angle + angle_delta*i);
        vertices[index++] = k*cos_f32(a);
        vertices[index++] = k*sin_f32(a);

        vertices[index++] = cmd->color.r;
        vertices[index++] = cmd->color.g;
        vertices[index++] = cmd->color.b;
        vertices[index++] = cmd->color.a;

        vertices[index++] = cos_f32(a);
        vertices[index++] = sin_f32(a);

        vertices[index++] = cmd->color.r;
        vertices[index++] = cmd->color.g;
        vertices[index++] = cmd->color.b;
        vertices[index++] = cmd->color.a;
    }

    index = 0;
    for (u32 i = 0; i < n_indices; i += 3, ++index)
    {
        indices[i] = index;
        indices[i+1] = index+1;
        indices[i+2] = index+2;
    }

    vao_bind(renderer->circle_renderer.vao);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->circle_renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, n_verts*sizeof(f32), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->circle_renderer.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_indices*sizeof(u32), indices, GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, n_indices, GL_UNSIGNED_INT, 0);
    vao_bind(0);
}

internal void output_ring_outline(Renderer* renderer, RenderCommandRingOutline* cmd)
{
    if (cmd->inner_radius > cmd->outer_radius)
    {
        f32 temp = cmd->inner_radius;
        cmd->inner_radius = cmd->outer_radius;
        cmd->outer_radius = temp;
    }

    if (cmd->outer_radius <= 0.0f)
        cmd->outer_radius = 0.1f;

    m4 model = m4_identity();
    model = m4_translate(model, (v3){cmd->center.x, cmd->center.y, 0.0f});
    model = m4_rotate(model, glm_rad(cmd->rotation), (v3){0.0f, 0.0f, 1.0f});
    model = m4_scale(model, (v3){cmd->outer_radius, cmd->outer_radius, 1.0f});

    shader_set_m4(renderer->circle_renderer.shader, "model", model, false);
    shader_set_v4(renderer->circle_renderer.shader, "color", cmd->color);

    u32 segs = renderer->config.circle_line_segments;
    // u32 tris = 2*segs + 14;
    // u32 n_verts = 4*segs + 28;
    u32 tris = 4*segs;
    u32 n_verts = 24*segs;
    u32 n_indices = 3*tris;
    f32* vertices = push_array(&renderer->scratch_arena, n_verts, f32);
    u32* indices = push_array(&renderer->scratch_arena, n_indices, u32);

    // Construct points from angles of tris
    f32 angle_delta = abs_f32(cmd->end_angle - cmd->start_angle) / segs;
    u32 index = 0;
    u32 iterations = n_verts/4 - 2;
    f32 k_in = cmd->inner_radius / cmd->outer_radius;
    f32 k_t = cmd->thickness / cmd->outer_radius;

    // NOTE(lucas): inner edge
    for (u32 i = 0; i <= segs; i += 2)
    {
        f32 a_deg = cmd->start_angle + angle_delta*i;
        f32 a = glm_rad(a_deg);
        vertices[index++] = k_in*cos_f32(a);
        vertices[index++] = -k_in*sin_f32(a);

        vertices[index++] = cmd->color.r;
        vertices[index++] = cmd->color.g;
        vertices[index++] = cmd->color.b;
        vertices[index++] = cmd->color.a;

        vertices[index++] = (k_in + k_t)*cos_f32(a);
        vertices[index++] = -(k_in + k_t)*sin_f32(a);

        vertices[index++] = cmd->color.r;
        vertices[index++] = cmd->color.g;
        vertices[index++] = cmd->color.b;
        vertices[index++] = cmd->color.a;
    }

    // NOTE(lucas): outer edge
    for (u32 i = 0; i <= segs; i += 2)
    {
        f32 a_deg = cmd->end_angle - angle_delta*i;
        f32 a = glm_rad(a_deg);
        vertices[index++] = cos_f32(a);
        vertices[index++] = -sin_f32(a);

        vertices[index++] = cmd->color.r;
        vertices[index++] = cmd->color.g;
        vertices[index++] = cmd->color.b;
        vertices[index++] = cmd->color.a;

        vertices[index++] = (1.0f - k_t)*cos_f32(a);
        vertices[index++] = -(1.0f - k_t)*sin_f32(a);

        vertices[index++] = cmd->color.r;
        vertices[index++] = cmd->color.g;
        vertices[index++] = cmd->color.b;
        vertices[index++] = cmd->color.a;
    }

    index = 0;
    for (u32 i = 0; i < n_indices; i += 3, ++index)
    {
        indices[i] = index;
        indices[i+1] = index+1;
        indices[i+2] = index+2;
    }

    vao_bind(renderer->circle_renderer.vao);

    glBindBuffer(GL_ARRAY_BUFFER, renderer->circle_renderer.vbo);
    glBufferData(GL_ARRAY_BUFFER, n_verts*sizeof(f32), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->circle_renderer.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n_indices*sizeof(u32), indices, GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, n_indices, GL_UNSIGNED_INT, 0);
    vao_bind(0);

    // NOTE(lucas): Draw cap lines
    // TODO(lucas): Figure out how to properly include cap lines directly in vertex data?
    v2 cap_start = {cmd->center.x + cmd->inner_radius, cmd->center.y};
    v2 cap_end = {cmd->center.x + cmd->outer_radius, cmd->center.y};

    RenderCommandLine start_cap = {RENDER_COMMAND_RenderCommandLine, cmd->color, cap_start, cap_end, cmd->center,
                                   cmd->thickness, cmd->start_angle + cmd->rotation};
    RenderCommandLine end_cap = {RENDER_COMMAND_RenderCommandLine, cmd->color, cap_start, cap_end, cmd->center,
                                 cmd->thickness, cmd->end_angle + cmd->rotation};

    output_line(renderer, &start_cap);
    output_line(renderer, &end_cap);
}

internal void output_scissor_test(Renderer* renderer, RenderCommandScissorTest* cmd)
{
    glEnable(GL_SCISSOR_TEST);
    glScissor((GLint)cmd->clip.x, (GLint)cmd->clip.y, (GLsizei)cmd->clip.width, (GLsizei)cmd->clip.height);
}

internal RenderCommandBuffer render_command_buffer_alloc(MemoryArena* arena, size max_bytes)
{
    RenderCommandBuffer result = {0};
    result.base = (u8*)push_size(arena, max_bytes);
    result.bytes = 0;
    result.max_bytes = max_bytes;
    return result;
}

internal void render_command_buffer_clear(RenderCommandBuffer* command_buffer)
{
    command_buffer->bytes = 0;
}

#define render_command_push(buffer, type) (type*)render_command_push_(buffer, sizeof(type), RENDER_COMMAND_##type)
internal RenderCommand* render_command_push_(RenderCommandBuffer* command_buffer, size bytes, RenderCommandType type)
{
    RenderCommand* result = 0;
    if (command_buffer->bytes + bytes < command_buffer->max_bytes)
    {
        result = (RenderCommand*)(command_buffer->base + command_buffer->bytes);
        result->type = type;
        command_buffer->bytes += bytes;
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
    for (size base_address = 0; base_address < command_buffer->bytes;)
    {
        // TODO(lucas): This can probably be collapsed into a macro
        RenderCommand* header = (RenderCommand*)(command_buffer->base + base_address);
        switch(header->type)
        {
            case RENDER_COMMAND_RenderCommandLine:
            {
                RenderCommandLine* cmd = (RenderCommandLine*)header;
                output_line(renderer, cmd);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandTriangle:
            {
                RenderCommandTriangle* cmd = (RenderCommandTriangle*)header;
                output_triangle(renderer, cmd);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandTriangleOutline:
            {
                RenderCommandTriangleOutline* cmd = (RenderCommandTriangleOutline*)header;
                output_triangle_outline(renderer, cmd);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandTriangleGradient:
            {
                RenderCommandTriangleGradient* cmd = (RenderCommandTriangleGradient*)header;
                output_triangle_gradient(renderer, cmd);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandQuad:
            {
                RenderCommandQuad* cmd = (RenderCommandQuad*)header;
                output_quad(renderer, cmd);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandQuadOutline:
            {
                RenderCommandQuadOutline* cmd = (RenderCommandQuadOutline*)header;
                output_quad_outline(renderer, cmd);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandQuadGradient:
            {
                RenderCommandQuadGradient* cmd = (RenderCommandQuadGradient*)header;
                output_quad_gradient(renderer, cmd);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandCircle:
            {
                RenderCommandCircle* cmd = (RenderCommandCircle*)header;
                output_circle(renderer, cmd);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandCircleOutline:
            {
                RenderCommandCircleOutline* cmd = (RenderCommandCircleOutline*)header;
                output_circle_outline(renderer, cmd);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandCircleSector:
            {
                RenderCommandCircleSector* cmd = (RenderCommandCircleSector*)header;
                output_circle_sector(renderer, cmd);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandRing:
            {
                RenderCommandRing* cmd = (RenderCommandRing*)header;
                output_ring(renderer, cmd);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandRingOutline:
            {
                RenderCommandRingOutline* cmd = (RenderCommandRingOutline*)header;
                output_ring_outline(renderer, cmd);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandSprite:
            {
                RenderCommandSprite* cmd = (RenderCommandSprite*)header;
                output_sprite(renderer, cmd);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandText:
            {
                RenderCommandText* cmd = (RenderCommandText*)header;
                output_text(renderer, cmd);
                base_address += sizeof(*cmd);
            } break;

            case RENDER_COMMAND_RenderCommandScissorTest:
            {
                RenderCommandScissorTest* cmd = (RenderCommandScissorTest*)header;
                output_scissor_test(renderer, cmd);
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

Renderer renderer_init(Window* window, int viewport_width, int viewport_height, size command_buffer_bytes)
{
    Renderer renderer = {0};
    renderer.window_width = window->width;
    renderer.window_height = window->height;

    stbi_set_flip_vertically_on_load(true);
    opengl_init(window);

    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    renderer.command_buffer_arena = memory_arena_alloc(command_buffer_bytes);
    renderer.command_buffer = render_command_buffer_alloc(&renderer.command_buffer_arena, command_buffer_bytes);

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
    char border_frag_shader_full_path[MAX_FILEPATH_LEN];

    path_from_install_dir("/res/shaders/framebuffer.vs", framebuffer_vert_shader_full_path);
    path_from_install_dir("/res/shaders/framebuffer.fs", framebuffer_frag_shader_full_path);
    path_from_install_dir("/res/shaders/poly.vs", poly_vert_shader_full_path);
    path_from_install_dir("/res/shaders/poly.fs", poly_frag_shader_full_path);
    path_from_install_dir("/res/shaders/sprite.vs", sprite_vert_shader_full_path);
    path_from_install_dir("/res/shaders/sprite.fs", sprite_frag_shader_full_path);
    path_from_install_dir("/res/shaders/font.vs", font_vert_shader_full_path);
    path_from_install_dir("/res/shaders/font.fs", font_frag_shader_full_path);
    path_from_install_dir("/res/shaders/ui.vs", ui_vert_shader_full_path);
    path_from_install_dir("/res/shaders/ui.fs", ui_frag_shader_full_path);
    path_from_install_dir("/res/shaders/border.fs", border_frag_shader_full_path);

    u32 framebuffer_shader = shader_init(&renderer, framebuffer_vert_shader_full_path, framebuffer_frag_shader_full_path);
    u32 poly_shader        = shader_init(&renderer, poly_vert_shader_full_path, poly_frag_shader_full_path);
    u32 sprite_shader      = shader_init(&renderer, sprite_vert_shader_full_path, sprite_frag_shader_full_path);
    u32 font_shader        = shader_init(&renderer, font_vert_shader_full_path, font_frag_shader_full_path);
    u32 ui_shader          = shader_init(&renderer, ui_vert_shader_full_path, ui_frag_shader_full_path);
    u32 poly_border_shader = shader_init(&renderer, poly_vert_shader_full_path, border_frag_shader_full_path);

    renderer.triangle_renderer    = triangle_renderer_init(poly_shader);
    renderer.quad_renderer        = quad_renderer_init(poly_shader);
    renderer.circle_renderer      = circle_renderer_init(poly_shader, renderer.config.circle_line_segments);
    renderer.sprite_renderer      = sprite_renderer_init(sprite_shader);
    renderer.font_renderer        = font_renderer_init(font_shader);
    renderer.framebuffer_renderer = framebuffer_renderer_init(framebuffer_shader);
    renderer.ui_renderer          = ui_renderer_init(ui_shader);

    renderer.poly_shader = poly_shader;
    renderer.poly_border_shader = poly_border_shader;
    
    renderer.framebuffer = framebuffer_init(framebuffer_shader, viewport_width, viewport_height,
                                            renderer.config.msaa_level, false);
    renderer.intermediate_framebuffer = framebuffer_init(framebuffer_shader, viewport_width, viewport_height, 0, true);

    for (u32 i = 0; i < countof(renderer.tex_ids); ++i)
    {
        RenderID* tex_id = renderer.tex_ids + i;
        glGenTextures(1, &tex_id->id);
    }

    return renderer;
}

void renderer_delete(Renderer* renderer)
{
    render_object_delete(&renderer->circle_renderer);
    render_object_delete(&renderer->quad_renderer);
    render_object_delete(&renderer->sprite_renderer);
    render_object_delete(&renderer->font_renderer);
    render_object_delete(&renderer->framebuffer_renderer);

    framebuffer_delete(&renderer->framebuffer);
    framebuffer_delete(&renderer->intermediate_framebuffer);
}

void renderer_new_frame(Renderer* renderer, Window* window)
{
    if (renderer->config.wireframe_mode)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_MULTISAMPLE);

    renderer->window_width = window->width;
    renderer->window_height = window->height;

    // NOTE(lucas): If the user does not call renderer_viewport to set the viewport themselves,
    // fit the viewport to the window.
    if (rect_is_zero(renderer->viewport))
    {
        rect viewport = rect_min_dim(v2_zero(), (v2){(f32)window->width, (f32)window->height});
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
    m4 projection = m4_ortho(renderer->viewport.x, renderer->viewport.x + renderer->viewport.width,
                             renderer->viewport.y + renderer->viewport.height, renderer->viewport.y,
                             -1.0f, 1.0f);

    // NOTE(lucas): Most shapes use the same shader, so no need to set the uniform for each shape
    shader_set_m4(renderer->triangle_renderer.shader, "projection", projection, false);
    shader_set_m4(renderer->poly_border_shader,       "projection", projection, false);
    shader_set_m4(renderer->sprite_renderer.shader,   "projection", projection, false);
    shader_set_m4(renderer->font_renderer.shader,     "projection", projection, false);
    shader_set_m4(renderer->ui_renderer.shader,       "projection", projection, false);

    ui_new_frame(renderer, window->width, window->height);

    renderer_clear(color_black());
}

void renderer_render(Renderer* renderer)
{
    for (u32 i = 0; i < countof(renderer->tex_ids); ++i)
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

    // TODO(lucas): Use renderer AA settings
    ui_render(renderer, NK_ANTI_ALIASING_ON);
    render_command_buffer_output(renderer);

    // NOTE(lucas): If MSAA is used, blit the multisampled framebuffer onto the
    // intermediate framebuffer
    if (renderer->config.msaa_level > 0)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, renderer->framebuffer.id);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderer->intermediate_framebuffer.id);
        glBlitFramebuffer((int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height,
                        (int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height,
                        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
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

    for (u32 i = 0; i < countof(renderer->tex_ids); ++i)
        renderer->textures_to_generate[i] = (Texture){0};
}

void renderer_viewport(Renderer* renderer, rect viewport)
{
    renderer->viewport = viewport;
    glViewport((int)viewport.x, (int)viewport.y, (int)viewport.width, (int)viewport.height);
}

void renderer_clear(v4 color)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(color.r, color.g, color.b, color.a);
}

void draw_line(Renderer* renderer, v2 start, v2 end, v4 color, f32 thickness, f32 rotation)
{
    RenderCommandLine* cmd = render_command_push(&renderer->command_buffer, RenderCommandLine);
    if (!cmd)
        return;
    v2 origin = v2_scale(v2_add(start, end), 0.5f);
    cmd->start = start;
    cmd->end = end;
    cmd->origin = origin;
    cmd->color = color;
    cmd->thickness = thickness;
    cmd->rotation = rotation;
}

void draw_triangle(Renderer* renderer, v2 a, v2 b, v2 c, v4 color, f32 rotation)
{
    RenderCommandTriangle* cmd = render_command_push(&renderer->command_buffer, RenderCommandTriangle);
    if (!cmd)
        return;
    v2 origin = v2_scale(v2_add(v2_add(a, b), c), 1.0f/3.0f);
    cmd->a = a;
    cmd->b = b;
    cmd->c = c;
    cmd->origin = origin;
    cmd->color = color;
    cmd->rotation = rotation;
}

void draw_triangle_outline(Renderer* renderer, v2 a, v2 b, v2 c, v4 color, f32 rotation, f32 thickness)
{
    RenderCommandTriangleOutline* cmd = render_command_push(&renderer->command_buffer, RenderCommandTriangleOutline);
    if (!cmd)
        return;
    v2 origin = v2_scale(v2_add(v2_add(a, b), c), 1.0f/3.0f);
    cmd->a = a;
    cmd->b = b;
    cmd->c = c;
    cmd->origin = origin;
    cmd->color = color;
    cmd->thickness = thickness;
    cmd->rotation = rotation;
}

void draw_triangle_gradient(Renderer* renderer, v2 a, v2 b, v2 c, v4 color_a, v4 color_b, v4 color_c, f32 rotation)
{
    RenderCommandTriangleGradient* cmd = render_command_push(&renderer->command_buffer, RenderCommandTriangleGradient);
    if (!cmd)
        return;
    v2 origin = v2_scale(v2_add(v2_add(a, b), c), 1.0f/3.0f);
    cmd->a = a;
    cmd->b = b;
    cmd->c = c;
    cmd->origin = origin;
    cmd->color_a = color_a;
    cmd->color_b = color_b;
    cmd->color_c = color_c;
    cmd->rotation = rotation;
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

void draw_quad_outline(Renderer* renderer, v2 position, v2 size, v4 color, f32 thickness, f32 rotation)
{
    RenderCommandQuadOutline* cmd = render_command_push(&renderer->command_buffer, RenderCommandQuadOutline);
    if (!cmd)
        return;
    v2 origin = v2_add(position, v2_scale(size, 0.5f));
    cmd->position = position;
    cmd->origin = origin;
    cmd->size = size;
    cmd->color = color;
    cmd->thickness = thickness;
    cmd->rotation = rotation;
}

void draw_quad_gradient(Renderer* renderer, v2 position, v2 size, v4 color_bl, v4 color_br, v4 color_tr, v4 color_tl,
                        f32 rotation)
{
    RenderCommandQuadGradient* cmd = render_command_push(&renderer->command_buffer, RenderCommandQuadGradient);
    if (!cmd)
        return;
    v2 origin = v2_add(position, v2_scale(size, 0.5f));
    cmd->position = position;
    cmd->origin = origin;
    cmd->size = size;
    cmd->color_bl = color_bl;
    cmd->color_br = color_br;
    cmd->color_tr = color_tr;
    cmd->color_tl = color_tl;
    cmd->rotation = rotation;
}

void draw_circle(Renderer* renderer, v2 center, f32 radius, v4 color)
{
    RenderCommandCircle* cmd = render_command_push(&renderer->command_buffer, RenderCommandCircle);
    if (!cmd)
        return;
    cmd->center = center;
    cmd->radius = radius;
    cmd->color = color;
}

void draw_circle_outline(Renderer* renderer, v2 center, f32 radius, v4 color, f32 thickness)
{
    RenderCommandCircleOutline* cmd = render_command_push(&renderer->command_buffer, RenderCommandCircleOutline);
    if (!cmd)
        return;
    cmd->center = center;
    cmd->radius = radius;
    cmd->color = color;
    cmd->thickness = thickness;
}

void draw_circle_sector(Renderer* renderer, v2 center, f32 radius, f32 start_angle, f32 end_angle, v4 color, f32 rotation)
{
    RenderCommandCircleSector* cmd = render_command_push(&renderer->command_buffer, RenderCommandCircleSector);
    if (!cmd)
        return;
    cmd->center = center;
    cmd->color = color;
    cmd->radius = radius;
    cmd->start_angle = start_angle;
    cmd->end_angle = end_angle;
    cmd->rotation = rotation;
}

void draw_ring(Renderer* renderer, v2 center, f32 outer_radius, f32 inner_radius, f32 start_angle, f32 end_angle,
               v4 color, f32 rotation)
{
    RenderCommandRing* cmd = render_command_push(&renderer->command_buffer, RenderCommandRing);
    if (!cmd)
        return;
    cmd->center = center;
    cmd->color = color;
    cmd->outer_radius = outer_radius;
    cmd->inner_radius = inner_radius;
    cmd->start_angle = start_angle;
    cmd->end_angle = end_angle;
    cmd->rotation = rotation;
}

void draw_ring_outline(Renderer* renderer, v2 center, f32 outer_radius, f32 inner_radius, f32 start_angle,
                       f32 end_angle, v4 color, f32 rotation, f32 thickness)
{
    RenderCommandRingOutline* cmd = render_command_push(&renderer->command_buffer, RenderCommandRingOutline);
    if (!cmd)
        return;
    cmd->center = center;
    cmd->color = color;
    cmd->outer_radius = outer_radius;
    cmd->inner_radius = inner_radius;
    cmd->start_angle = start_angle;
    cmd->end_angle = end_angle;
    cmd->rotation = rotation;
    cmd->thickness = thickness;
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

    cmd->text = text;
    cmd->text.string = s8_copy(text.string, &renderer->scratch_arena);
}

void draw_scissor_test(Renderer* renderer, rect clip)
{
    RenderCommandScissorTest* cmd = render_command_push(&renderer->command_buffer, RenderCommandScissorTest);
    if (!cmd)
        return;
    cmd->clip = clip;
}

u32 renderer_next_tex_id(Renderer* renderer)
{
    u32 id = 0;
    
    if (renderer->tex_index <= countof(renderer->tex_ids))
    {
        id = renderer->tex_ids[renderer->tex_index].id;
        renderer->tex_ids[renderer->tex_index].used = true;
    }

    return id;
}

void renderer_push_texture(Renderer* renderer, Texture tex)
{
    if (renderer->tex_index <= countof(renderer->tex_ids))
    {
        renderer->textures_to_generate[renderer->tex_index] = tex;
        ++renderer->tex_index;
    }
}
