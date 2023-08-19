/*
 * Nuklear - 1.32.0 - public domain
 * no warrenty implied; use at your own risk.
 * authored from 2015-2016 by Micha Mettke
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#pragma once

#include "glad/include/glad/glad.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_KEYSTATE_BASED_INPUT
#include <nuklear/nuklear.h>

#include "input.h"

#include <assert.h>

enum nk_alchemy_init_state{
    NK_ALCHEMY_DEFAULT=0,
    NK_ALCHEMY_INSTALL_CALLBACKS
};

#ifndef NK_ALCHEMY_TEXT_MAX
#define NK_ALCHEMY_TEXT_MAX 256
#endif

typedef struct nk_alchemy_device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture tex_null;
    GLuint vbo, vao, ebo;
    GLuint prog;
    GLuint vert_shdr;
    GLuint frag_shdr;
    GLint attrib_pos;
    GLint attrib_uv;
    GLint attrib_col;
    GLint uniform_tex;
    GLint uniform_proj;
    GLuint font_tex;
} nk_alchemy_device;

typedef struct nk_alchemy_state {
    int width, height;
    int display_width, display_height;
    nk_alchemy_device ogl;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_vec2 fb_scale;
    u32 text[NK_ALCHEMY_TEXT_MAX];
    int text_len;
    struct nk_vec2 scroll;

    Keyboard* keyboard;
    Mouse* mouse;
} nk_alchemy_state;

struct nk_context    nk_alchemy_init(nk_alchemy_state* state, enum nk_alchemy_init_state);
void                 nk_alchemy_shutdown(nk_alchemy_state* state);
void                 nk_alchemy_font_stash_begin(nk_alchemy_state* state, struct nk_font_atlas **atlas);
void                 nk_alchemy_font_stash_end(nk_alchemy_state* state);
void                 nk_alchemy_new_frame(nk_alchemy_state* state, u32 window_width, u32 window_height);
void                 nk_alchemy_render(nk_alchemy_state* state, enum nk_anti_aliasing, int max_vertex_buffer, int max_element_buffer);

void                 nk_alchemy_device_destroy(nk_alchemy_state* state);
void                 nk_alchemy_device_create(nk_alchemy_state* state);
