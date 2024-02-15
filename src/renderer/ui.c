#include "alchemy/renderer/ui.h"
#include "alchemy/renderer/renderer.h"
#include "alchemy/util/types.h"

#include <glad/glad.h>

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

typedef struct Vertex {
    f32 position[2];
    f32 uv[2];
    nk_byte col[4];
} Vertex;

internal UIDevice ui_device_create(u32 ui_shader)
{
    UIDevice dev = {0};
    nk_buffer_init_default(&dev.cmds);
    dev.shader = ui_shader;

    dev.attrib_pos = glGetAttribLocation(dev.shader, "position");
    dev.attrib_uv = glGetAttribLocation(dev.shader, "tex_coord");
    dev.attrib_col = glGetAttribLocation(dev.shader, "color");

    {
        /* buffer setup */
        GLsizei vs = sizeof(Vertex);
        usize vp = offsetof(Vertex, position);
        usize vt = offsetof(Vertex, uv);
        usize vc = offsetof(Vertex, col);

        glGenBuffers(1, &dev.vbo);
        glGenBuffers(1, &dev.ebo);
        glGenVertexArrays(1, &dev.vao);

        glBindVertexArray(dev.vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev.vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev.ebo);

        glEnableVertexAttribArray((GLuint)dev.attrib_pos);
        glEnableVertexAttribArray((GLuint)dev.attrib_uv);
        glEnableVertexAttribArray((GLuint)dev.attrib_col);

        glVertexAttribPointer((GLuint)dev.attrib_pos, 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
        glVertexAttribPointer((GLuint)dev.attrib_uv, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
        glVertexAttribPointer((GLuint)dev.attrib_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);
    }

    texture_unbind(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return dev;
}

internal void ui_device_upload_atlas(Renderer* renderer, const void *image, int width, int height)
{
    UIDevice *dev = &renderer->ui_render_state.device;
    dev->font_tex = texture_generate(0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, image);
}

internal void ui_device_destroy(UIDevice* dev)
{
    shader_delete(dev->shader);
    texture_delete(&dev->font_tex);
    glDeleteBuffers(1, &dev->vbo);
    glDeleteBuffers(1, &dev->ebo);
    nk_buffer_free(&dev->cmds);
}

void ui_render(Renderer* renderer, enum nk_anti_aliasing aa)
{
    UIRenderState* state = &renderer->ui_render_state;
    UIDevice *dev = &state->device;
    struct nk_buffer vbuf, ebuf;

    m4 ortho = (m4)
    {
        2.0f,  0.0f,  0.0f, 0.0f,
        0.0f, -2.0f,  0.0f, 0.0f,
        0.0f,  0.0f, -1.0f, 0.0f,
       -1.0f,  1.0f,  0.0f, 1.0f,
    };
    ortho.m00 /= (f32)state->width;
    ortho.m11 /= (f32)state->height;

    /* setup global state */
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    /* setup program */
    shader_bind(dev->shader);
    shader_set_i32(dev->shader, "tex", 0);
    shader_set_m4(dev->shader, "projection", ortho, false);
    glViewport(0,0,(GLsizei)state->display_width,(GLsizei)state->display_height);
    {
        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        void *vertices, *elements;
        nk_size offset = 0;

        /* allocate vertex and element buffer */
        glBindVertexArray(dev->vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

        glBufferData(GL_ARRAY_BUFFER, MAX_VERTEX_BUFFER, NULL, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_ELEMENT_BUFFER, NULL, GL_STREAM_DRAW);

        /* load draw vertices & elements directly into vertex + element buffer */
        vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        {
            /* fill convert configuration */
            struct nk_convert_config config;
            persist const struct nk_draw_vertex_layout_element vertex_layout[] =
            {
                {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(Vertex, position)},
                {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(Vertex, uv)},
                {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(Vertex, col)},
                {NK_VERTEX_LAYOUT_END}
            };
            memset(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(Vertex);
            config.vertex_alignment = NK_ALIGNOF(Vertex);
            config.tex_null = dev->tex_null;
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = aa;
            config.line_AA = aa;

            /* setup buffers to load vertices and elements */
            nk_buffer_init_fixed(&vbuf, vertices, (usize)MAX_VERTEX_BUFFER);
            nk_buffer_init_fixed(&ebuf, elements, (usize)MAX_ELEMENT_BUFFER);
            nk_convert(&state->ctx, &dev->cmds, &vbuf, &ebuf, &config);
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        /* iterate over and execute each draw command */
        nk_draw_foreach(cmd, &state->ctx, &dev->cmds)
        {
            if (!cmd->elem_count) continue;
            texture_bind_id((u32)cmd->texture.id, 0);
            glScissor(
                (GLint)(cmd->clip_rect.x * state->fb_scale.x),
                (GLint)((state->height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * state->fb_scale.y),
                (GLint)(cmd->clip_rect.w * state->fb_scale.x),
                (GLint)(cmd->clip_rect.h * state->fb_scale.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, (const void*) offset);
            offset += cmd->elem_count * sizeof(nk_draw_index);
        }
        nk_clear(&state->ctx);
        nk_buffer_clear(&dev->cmds);
    }

    /* default OpenGL state */
    shader_unbind();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_SCISSOR_TEST);
}

internal void ui_enter_char(Renderer* renderer, u64 code)
{
    UIRenderState* state = &renderer->ui_render_state;
    /* NOTE(lucas): Make sure character code is at least 32, which excludes the NULL character, backspace, and so on.
     * If these keys are not disregarded here, they may perform their other functions, but they will also
     * place a question mark in the text box.
     */
    if (state->text_len < NK_ALCHEMY_TEXT_MAX && code >= 32)
        state->text[state->text_len++] = (u32)code;
}

internal void ui_clipboard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    char *text = clipboard_read_string();
    if (text)
        nk_textedit_paste(edit, text, nk_strlen(text));
}

internal void ui_clipboard_copy(nk_handle usr, const char *text, int len)
{
    char *str = 0;
    if (!len) return;
    str = (char*)malloc((usize)len+1);
    if (!str) return;
    memcpy(str, text, (usize)len);
    str[len] = '\0';
    clipboard_write_string(str);
    free(str);
}

UIRenderState ui_render_state_init(u32 shader)
{
    UIRenderState state = {0};
    nk_init_default(&state.ctx, 0);
    state.ctx.clip.copy = ui_clipboard_copy;
    state.ctx.clip.paste = ui_clipboard_paste;
    state.ctx.clip.userdata = nk_handle_ptr(&state);
    state.device = ui_device_create(shader);

    return state;
}

void ui_font_stash_begin(Renderer* renderer, struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(&renderer->ui_render_state.atlas);
    nk_font_atlas_begin(&renderer->ui_render_state.atlas);
    *atlas = &renderer->ui_render_state.atlas;
}

void ui_font_stash_end(Renderer* renderer)
{
    UIRenderState* state = &renderer->ui_render_state;
    const void *image; int w, h;
    image = nk_font_atlas_bake(&state->atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    ui_device_upload_atlas(renderer, image, w, h);
    nk_font_atlas_end(&state->atlas, nk_handle_id((int)state->device.font_tex.id), &state->device.tex_null);
    if (state->atlas.default_font)
        nk_style_set_font(&state->ctx, &state->atlas.default_font->handle);
}

void ui_new_frame(Renderer* renderer, u32 window_width, u32 window_height)
{
    UIRenderState* state = &renderer->ui_render_state;
    struct nk_context* ctx = &state->ctx;
    Keyboard* keyboard = state->keyboard;
    Mouse* mouse = state->mouse;

    state->width = window_width;
    state->height = window_height;
    state->display_width = window_width;
    state->display_height = window_height;
    state->fb_scale.x = (f32)state->display_width/(f32)state->width;
    state->fb_scale.y = (f32)state->display_height/(f32)state->height;

    nk_input_begin(ctx);

    // text input
    if (keyboard)
    {
        ui_enter_char(renderer, keyboard->current_char);
        for (int i = 0; i < state->text_len; ++i)
            nk_input_unicode(ctx, state->text[i]);

        nk_input_key(ctx, NK_KEY_DEL,          key_pressed(keyboard, KEY_DEL));
        nk_input_key(ctx, NK_KEY_ENTER,        key_pressed(keyboard, KEY_ENTER));
        nk_input_key(ctx, NK_KEY_TAB,          key_pressed(keyboard, KEY_TAB));
        nk_input_key(ctx, NK_KEY_BACKSPACE,    key_pressed(keyboard, KEY_BACKSPACE));
        nk_input_key(ctx, NK_KEY_UP,           key_pressed(keyboard, KEY_UP));
        nk_input_key(ctx, NK_KEY_DOWN,         key_pressed(keyboard, KEY_DOWN));
        nk_input_key(ctx, NK_KEY_TEXT_START,   key_pressed(keyboard, KEY_HOME));
        nk_input_key(ctx, NK_KEY_TEXT_END,     key_pressed(keyboard, KEY_END));
        nk_input_key(ctx, NK_KEY_SCROLL_START, key_pressed(keyboard, KEY_HOME));
        nk_input_key(ctx, NK_KEY_SCROLL_END,   key_pressed(keyboard, KEY_END));
        nk_input_key(ctx, NK_KEY_SCROLL_DOWN,  key_pressed(keyboard, KEY_PAGEDOWN));
        nk_input_key(ctx, NK_KEY_SCROLL_UP,    key_pressed(keyboard, KEY_PAGEUP));
        nk_input_key(ctx, NK_KEY_SHIFT,        key_pressed(keyboard, KEY_LSHIFT) ||
                                            key_pressed(keyboard, KEY_RSHIFT));

        if (key_pressed(keyboard, KEY_LCONTROL) || key_pressed(keyboard, KEY_RCONTROL))
        {
            nk_input_key(ctx, NK_KEY_COPY,            key_pressed(keyboard, KEY_C));
            nk_input_key(ctx, NK_KEY_PASTE,           key_pressed(keyboard, KEY_V));
            nk_input_key(ctx, NK_KEY_CUT,             key_pressed(keyboard, KEY_X));
            nk_input_key(ctx, NK_KEY_TEXT_UNDO,       key_pressed(keyboard, KEY_Z));
            nk_input_key(ctx, NK_KEY_TEXT_REDO,       key_pressed(keyboard, KEY_Y));
            nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT,  key_pressed(keyboard, KEY_LEFT));
            nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, key_pressed(keyboard, KEY_RIGHT));
            nk_input_key(ctx, NK_KEY_TEXT_LINE_START, key_pressed(keyboard, KEY_B));
            nk_input_key(ctx, NK_KEY_TEXT_LINE_END,   key_pressed(keyboard, KEY_E));
        }
        else
        {
            nk_input_key(ctx, NK_KEY_LEFT, key_pressed(keyboard, KEY_LEFT));
            nk_input_key(ctx, NK_KEY_RIGHT, key_pressed(keyboard, KEY_RIGHT));
            nk_input_key(ctx, NK_KEY_COPY, 0);
            nk_input_key(ctx, NK_KEY_PASTE, 0);
            nk_input_key(ctx, NK_KEY_CUT, 0);
            nk_input_key(ctx, NK_KEY_SHIFT, 0);
        }
    }

    if (mouse)
    {
        nk_input_motion(ctx, mouse->x, mouse->y);
        nk_input_scroll(ctx, nk_vec2(0.0f, (f32)mouse->scroll));

        nk_input_button(ctx, NK_BUTTON_LEFT, mouse->x, mouse->y,   mouse_button_pressed(mouse, MOUSE_LEFT));
        nk_input_button(ctx, NK_BUTTON_MIDDLE, mouse->x, mouse->y, mouse_button_pressed(mouse, MOUSE_MIDDLE));
        nk_input_button(ctx, NK_BUTTON_RIGHT, mouse->x, mouse->y,  mouse_button_pressed(mouse, MOUSE_RIGHT));
        nk_input_button(ctx, NK_BUTTON_DOUBLE, mouse->x, mouse->y, mouse_button_double_clicked(mouse, MOUSE_LEFT));
        nk_input_end(&state->ctx);
    }
    state->text_len = 0;
}

void ui_render_state_shutdown(UIRenderState* state)
{
    nk_font_atlas_clear(&state->atlas);
    nk_free(&state->ctx);
    ui_device_destroy(&state->device);
    memset(state, 0, sizeof(*state));
}
