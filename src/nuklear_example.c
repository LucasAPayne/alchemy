#include "nuklear_example.h"

/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */

#ifndef NK_ALCHEMY_DOUBLE_CLICK_LO
#define NK_ALCHEMY_DOUBLE_CLICK_LO 0.02
#endif
#ifndef NK_ALCHEMY_DOUBLE_CLICK_HI
#define NK_ALCHEMY_DOUBLE_CLICK_HI 0.2
#endif

typedef struct nk_alchemy_vertex {
    float position[2];
    float uv[2];
    nk_byte col[4];
} nk_alchemy_vertex;

#ifdef __APPLE__
  #define NK_SHADER_VERSION "#version 150\n"
#else
  #define NK_SHADER_VERSION "#version 300 es\n"
#endif

NK_API void
nk_alchemy_device_create(nk_alchemy_state* state)
{
    GLint status;
    static const GLchar *vertex_shader =
        NK_SHADER_VERSION
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 TexCoord;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main() {\n"
        "   Frag_UV = TexCoord;\n"
        "   Frag_Color = Color;\n"
        "   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
        "}\n";
    static const GLchar *fragment_shader =
        NK_SHADER_VERSION
        "precision mediump float;\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main(){\n"
        "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    nk_alchemy_device *dev = &state->ogl;
    nk_buffer_init_default(&dev->cmds);
    dev->prog = glCreateProgram();
    dev->vert_shdr = glCreateShader(GL_VERTEX_SHADER);
    dev->frag_shdr = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(dev->vert_shdr, 1, &vertex_shader, 0);
    glShaderSource(dev->frag_shdr, 1, &fragment_shader, 0);
    glCompileShader(dev->vert_shdr);
    glCompileShader(dev->frag_shdr);
    glGetShaderiv(dev->vert_shdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glGetShaderiv(dev->frag_shdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glAttachShader(dev->prog, dev->vert_shdr);
    glAttachShader(dev->prog, dev->frag_shdr);
    glLinkProgram(dev->prog);
    glGetProgramiv(dev->prog, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);

    dev->uniform_tex = glGetUniformLocation(dev->prog, "Texture");
    dev->uniform_proj = glGetUniformLocation(dev->prog, "ProjMtx");
    dev->attrib_pos = glGetAttribLocation(dev->prog, "Position");
    dev->attrib_uv = glGetAttribLocation(dev->prog, "TexCoord");
    dev->attrib_col = glGetAttribLocation(dev->prog, "Color");

    {
        /* buffer setup */
        GLsizei vs = sizeof(nk_alchemy_vertex);
        size_t vp = offsetof(nk_alchemy_vertex, position);
        size_t vt = offsetof(nk_alchemy_vertex, uv);
        size_t vc = offsetof(nk_alchemy_vertex, col);

        glGenBuffers(1, &dev->vbo);
        glGenBuffers(1, &dev->ebo);
        glGenVertexArrays(1, &dev->vao);

        glBindVertexArray(dev->vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

        glEnableVertexAttribArray((GLuint)dev->attrib_pos);
        glEnableVertexAttribArray((GLuint)dev->attrib_uv);
        glEnableVertexAttribArray((GLuint)dev->attrib_col);

        glVertexAttribPointer((GLuint)dev->attrib_pos, 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
        glVertexAttribPointer((GLuint)dev->attrib_uv, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
        glVertexAttribPointer((GLuint)dev->attrib_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

NK_INTERN void
nk_alchemy_device_upload_atlas(nk_alchemy_state* state, const void *image, int width, int height)
{
    nk_alchemy_device *dev = &state->ogl;
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, image);
}

NK_API void
nk_alchemy_device_destroy(nk_alchemy_state* state)
{
    nk_alchemy_device *dev = &state->ogl;
    glDetachShader(dev->prog, dev->vert_shdr);
    glDetachShader(dev->prog, dev->frag_shdr);
    glDeleteShader(dev->vert_shdr);
    glDeleteShader(dev->frag_shdr);
    glDeleteProgram(dev->prog);
    glDeleteTextures(1, &dev->font_tex);
    glDeleteBuffers(1, &dev->vbo);
    glDeleteBuffers(1, &dev->ebo);
    nk_buffer_free(&dev->cmds);
}

NK_API void
nk_glfw3_render(nk_alchemy_state* state, enum nk_anti_aliasing AA, int max_vertex_buffer, int max_element_buffer)
{
    nk_alchemy_device *dev = &state->ogl;
    struct nk_buffer vbuf, ebuf;
    GLfloat ortho[4][4] = {
        {2.0f, 0.0f, 0.0f, 0.0f},
        {0.0f,-2.0f, 0.0f, 0.0f},
        {0.0f, 0.0f,-1.0f, 0.0f},
        {-1.0f,1.0f, 0.0f, 1.0f},
    };
    ortho[0][0] /= (GLfloat)state->width;
    ortho[1][1] /= (GLfloat)state->height;

    /* setup global state */
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    /* setup program */
    glUseProgram(dev->prog);
    glUniform1i(dev->uniform_tex, 0);
    glUniformMatrix4fv(dev->uniform_proj, 1, GL_FALSE, &ortho[0][0]);
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

        glBufferData(GL_ARRAY_BUFFER, max_vertex_buffer, NULL, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_element_buffer, NULL, GL_STREAM_DRAW);

        /* load draw vertices & elements directly into vertex + element buffer */
        vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        {
            /* fill convert configuration */
            struct nk_convert_config config;
            static const struct nk_draw_vertex_layout_element vertex_layout[] = {
                {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(nk_alchemy_vertex, position)},
                {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(nk_alchemy_vertex, uv)},
                {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(nk_alchemy_vertex, col)},
                {NK_VERTEX_LAYOUT_END}
            };
            memset(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(nk_alchemy_vertex);
            config.vertex_alignment = NK_ALIGNOF(nk_alchemy_vertex);
            config.tex_null = dev->tex_null;
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = AA;
            config.line_AA = AA;

            /* setup buffers to load vertices and elements */
            nk_buffer_init_fixed(&vbuf, vertices, (size_t)max_vertex_buffer);
            nk_buffer_init_fixed(&ebuf, elements, (size_t)max_element_buffer);
            nk_convert(&state->ctx, &dev->cmds, &vbuf, &ebuf, &config);
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        /* iterate over and execute each draw command */
        nk_draw_foreach(cmd, &state->ctx, &dev->cmds)
        {
            if (!cmd->elem_count) continue;
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
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
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
}

NK_API void
nk_glfw3_char_callback(nk_alchemy_state* state, unsigned int codepoint)
{
    // nk_alchemy_state* state = glfwGetWindowUserPointer(win);
    if (state->text_len < NK_ALCHEMY_TEXT_MAX)
        state->text[state->text_len++] = codepoint;
}

// TODO(lucas): Mouse scrolling
// NK_API void
// nk_gflw3_scroll_callback(Mouse* mouse, double xoff, double yoff)
// {
//     (void)xoff;
//     mouse->scroll.x += (float)xoff;
//     mouse->scroll.y += (float)yoff;
// }

// TODO(lucas): get delta time
// NK_API void
// nk_glfw3_mouse_button_callback(Mouse* mouse, int button, int action, int mods)
// {
//     NK_UNUSED(mods);
//     if (is_mouse_button_pressed(mouse, MOUSE_LEFT))  {
//         double dt = glfwGetTime() - glfw->last_button_click;
//         if (dt > NK_GLFW_DOUBLE_CLICK_LO && dt < NK_GLFW_DOUBLE_CLICK_HI) {
//             glfw->is_double_click_down = nk_true;
//             glfw->double_click_pos = nk_vec2((float)x, (float)y);
//         }
//         glfw->last_button_click = glfwGetTime();
//     } else glfw->is_double_click_down = nk_false;
// }

// TODO(lucas): Clipboard
// NK_INTERN void
// nk_glfw3_clipboard_paste(nk_handle usr, struct nk_text_edit *edit)
// {
//     nk_alchemy_state* glfw = (nk_alchemy_state*)usr.ptr;
//     const char *text = glfwGetClipboardString(glfw->win);
//     if (text) nk_textedit_paste(edit, text, nk_strlen(text));
//     (void)usr;
// }

// NK_INTERN void
// nk_glfw3_clipboard_copy(nk_handle usr, const char *text, int len)
// {
//     nk_alchemy_state* glfw = (nk_alchemy_state*)usr.ptr;
//     char *str = 0;
//     if (!len) return;
//     str = (char*)malloc((size_t)len+1);
//     if (!str) return;
//     memcpy(str, text, (size_t)len);
//     str[len] = '\0';
//     glfwSetClipboardString(glfw->win, str);
//     free(str);
// }

NK_API struct nk_context*
nk_glfw3_init(nk_alchemy_state* state, enum nk_alchemy_init_state init_state)
{
    // glfw->win = win;
    // if (init_state == NK_ALCHEMY_INSTALL_CALLBACKS) {
    //     glfwSetScrollCallback(win, nk_gflw3_scroll_callback);
    //     glfwSetCharCallback(win, nk_glfw3_char_callback);
    //     glfwSetMouseButtonCallback(win, nk_glfw3_mouse_button_callback);
    // }
    nk_init_default(&state->ctx, 0);
    // state->ctx.clip.copy = nk_glfw3_clipboard_copy;
    // state->ctx.clip.paste = nk_glfw3_clipboard_paste;
    state->ctx.clip.copy = 0;
    state->ctx.clip.paste = 0;
    state->ctx.clip.userdata = nk_handle_ptr(&state);
    state->last_button_click = 0;
    nk_glfw3_device_create(state);

    state->is_double_click_down = nk_false;
    state->double_click_pos = nk_vec2(0, 0);

    return &state->ctx;
}

NK_API void
nk_glfw3_font_stash_begin(nk_alchemy_state* state, struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(&state->atlas);
    nk_font_atlas_begin(&state->atlas);
    *atlas = &state->atlas;
}

// NK_API void
// nk_glfw3_font_stash_end(nk_alchemy_state* state)
// {
//     const void *image; int w, h;
//     image = nk_font_atlas_bake(&state->atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
//     nk_glfw3_device_upload_atlas(state, image, w, h);
//     nk_font_atlas_end(&state->atlas, nk_handle_id((int)state->ogl.font_tex), &state->ogl.tex_null);
//     if (state->atlas.default_font)
//         nk_style_set_font(&state->ctx, &state->atlas.default_font->handle);
// }

NK_API void
nk_glfw3_new_frame(nk_alchemy_state* state)
{
    int i;
    struct nk_context *ctx = &state->ctx;
    // struct GLFWwindow *win = state->win;

    // glfwGetWindowSize(win, &glfw->width, &glfw->height);
    // glfwGetFramebufferSize(win, &glfw->display_width, &glfw->display_height);
    state->fb_scale.x = (float)state->display_width/(float)state->width;
    state->fb_scale.y = (float)state->display_height/(float)state->height;

    nk_input_begin(ctx);
    for (i = 0; i < state->text_len; ++i)
        nk_input_unicode(ctx, state->text[i]);

#ifdef NK_GLFW_GL3_MOUSE_GRABBING
    /* optional grabbing behavior */
    if (ctx->input.mouse.grab)
        glfwSetInputMode(glfw.win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    else if (ctx->input.mouse.ungrab)
        glfwSetInputMode(glfw->win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#endif

    nk_input_key(ctx, NK_KEY_DEL, is_key_pressed(&state->keyboard, KEY_DEL));
    nk_input_key(ctx, NK_KEY_ENTER, is_key_pressed(&state->keyboard, KEY_ENTER));
    nk_input_key(ctx, NK_KEY_TAB, is_key_pressed(&state->keyboard, KEY_TAB));
    nk_input_key(ctx, NK_KEY_BACKSPACE, is_key_pressed(&state->keyboard, KEY_BACKSPACE));
    nk_input_key(ctx, NK_KEY_UP, is_key_pressed(&state->keyboard, KEY_UP));
    nk_input_key(ctx, NK_KEY_DOWN, is_key_pressed(&state->keyboard, KEY_DOWN));
    nk_input_key(ctx, NK_KEY_TEXT_START, is_key_pressed(&state->keyboard, KEY_HOME));
    nk_input_key(ctx, NK_KEY_TEXT_END, is_key_pressed(&state->keyboard, KEY_END));
    nk_input_key(ctx, NK_KEY_SCROLL_START, is_key_pressed(&state->keyboard, KEY_HOME));
    nk_input_key(ctx, NK_KEY_SCROLL_END, is_key_pressed(&state->keyboard, KEY_END));
    nk_input_key(ctx, NK_KEY_SCROLL_DOWN, is_key_pressed(&state->keyboard, KEY_PAGEDOWN));
    nk_input_key(ctx, NK_KEY_SCROLL_UP, is_key_pressed(&state->keyboard, KEY_PAGEUP));
    nk_input_key(ctx, NK_KEY_SHIFT, is_key_pressed(&state->keyboard, KEY_LSHIFT) || is_key_pressed(&state->keyboard, KEY_RSHIFT));

    if (is_key_pressed(&state->keyboard, KEY_LCONTROL) || is_key_pressed(&state->keyboard, KEY_RCONTROL))
    {
        nk_input_key(ctx, NK_KEY_COPY, is_key_pressed(&state->keyboard, KEY_C));
        nk_input_key(ctx, NK_KEY_PASTE, is_key_pressed(&state->keyboard, KEY_V));
        nk_input_key(ctx, NK_KEY_CUT, is_key_pressed(&state->keyboard, KEY_X));
        nk_input_key(ctx, NK_KEY_TEXT_UNDO, is_key_pressed(&state->keyboard, KEY_Z));
        nk_input_key(ctx, NK_KEY_TEXT_REDO, is_key_pressed(&state->keyboard, KEY_Y));
        nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, is_key_pressed(&state->keyboard, KEY_LEFT));
        nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, is_key_pressed(&state->keyboard, KEY_RIGHT));
        nk_input_key(ctx, NK_KEY_TEXT_LINE_START, is_key_pressed(&state->keyboard, KEY_B));
        nk_input_key(ctx, NK_KEY_TEXT_LINE_END, is_key_pressed(&state->keyboard, KEY_E));
    }
    else
    {
        nk_input_key(ctx, NK_KEY_LEFT, is_key_pressed(&state->keyboard, KEY_LEFT));
        nk_input_key(ctx, NK_KEY_RIGHT, is_key_pressed(&state->keyboard, KEY_RIGHT));
        nk_input_key(ctx, NK_KEY_COPY, 0);
        nk_input_key(ctx, NK_KEY_PASTE, 0);
        nk_input_key(ctx, NK_KEY_CUT, 0);
        nk_input_key(ctx, NK_KEY_SHIFT, 0);
    }

    nk_input_motion(ctx, state->mouse.x, state->mouse.y);
#ifdef NK_GLFW_GL3_MOUSE_GRABBING
    if (ctx->input.mouse.grabbed) {
        glfwSetCursorPos(glfw->win, ctx->input.mouse.prev.x, ctx->input.mouse.prev.y);
        ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
        ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
    }
#endif
    nk_input_button(ctx, NK_BUTTON_LEFT, state->mouse.x, state->mouse.y, is_mouse_button_pressed(&state->mouse, MOUSE_LEFT));
    nk_input_button(ctx, NK_BUTTON_MIDDLE, state->mouse.x, state->mouse.y, is_mouse_button_pressed(&state->mouse, MOUSE_MIDDLE));
    nk_input_button(ctx, NK_BUTTON_RIGHT, state->mouse.x, state->mouse.y, is_mouse_button_pressed(&state->mouse, MOUSE_RIGHT));
    nk_input_button(ctx, NK_BUTTON_DOUBLE, (int)state->double_click_pos.x, (int)state->double_click_pos.y, state->is_double_click_down);
    nk_input_scroll(ctx, state->scroll);
    nk_input_end(&state->ctx);
    state->text_len = 0;
    state->scroll = nk_vec2(0,0);
}

NK_API
void nk_glfw3_shutdown(nk_alchemy_state* state)
{
    nk_font_atlas_clear(&state->atlas);
    nk_free(&state->ctx);
    nk_glfw3_device_destroy(state);
    memset(state, 0, sizeof(*state));
}
