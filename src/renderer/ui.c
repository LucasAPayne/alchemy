#include "alchemy/renderer/ui.h"
#include "alchemy/renderer/renderer.h"
#include "alchemy/util/types.h"
#include "alchemy/util/str.h"

#include <glad/glad.h>

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

typedef struct Vertex {
    f32 position[2];
    f32 uv[2];
    nk_byte col[4];
} Vertex;

// NOTE(lucas): Parameter list must match nk_text_width_f
internal f32 nk_alchemy_font_get_text_width(nk_handle handle, f32 height, const char* str, int len)
{
    f32 result = 0.0f;

    s8 s = (s8){(u8*)str, len};
    Font* font = (Font*)handle.ptr;
    Text text = text_init(s, font, v2_zero(), (u32)height);
    result = text_get_width(&text);

    return result;
}

internal v4 nk_color_to_v4(struct nk_color color)
{
    struct nk_colorf cf = nk_color_cf(color);
    v4 result = {cf.r, cf.g, cf.b, cf.a};
    return result;
}

/* FIXME(Lucas): There is a sneaky issue here that only occurs when optimizations are enabled.
 * At some point in the call stack, there will always be a read access violation, whether it is on the text area
 * or something to do with the text area parsing. For now, optimizations have been disabled on all the offending
 * functions, but clearly this is just a temporary solution.
 * Look for undefined behavior, pointer aliasing, alignment issues, etc.
 * Try enabling only specific optimizations to try and pinpoint what optimization actually causes the problem.
 * This might make it easier to determine the root cause.
 */
/* TODO(lucas): Currently, this only works for very static UIs since it uses absolute position relative to the window.
 * The position needs to be relative to the parent widget.
 * Maybe automatic like the other UI components with an optional additional offset.
 */
typedef struct UITextArea
{
    Renderer* renderer;
    TextArea* text_area;
} UITextArea;
internal void nk_draw_text_area(void* canvas, i16 x, i16 y, u16 w, u16 h, nk_handle callback_data)
{
    UITextArea* ui_text_area = (UITextArea*)callback_data.ptr;
    Renderer* renderer = ui_text_area->renderer;
    TextArea* text_area = ui_text_area->text_area;
    v2 temp = text_area->bounds.position;
    text_area->bounds.x = (f32)x;
    text_area->bounds.y = (f32)y;
    draw_text_area(renderer, text_area);
    text_area->bounds.position = temp;
}

void ui_draw_text_area(Renderer* renderer, TextArea* text_area, v2 offset)
{
    UITextArea* ui_text_area = push_struct(&renderer->scratch_arena, UITextArea);
    ui_text_area->renderer = renderer;
    ui_text_area->text_area = text_area;

    struct nk_context* ctx = &renderer->ui_state.ctx; 
    struct nk_command_buffer* out = nk_window_get_canvas(ctx);
    struct nk_rect bounds = nk_rect(text_area->bounds.x + offset.x, text_area->bounds.y + offset.y,
                                    text_area->bounds.width, text_area->bounds.height);
    nk_handle data = nk_handle_ptr(ui_text_area);
    nk_push_custom(out, bounds, nk_draw_text_area, data);
}

void ui_render(Renderer* renderer, enum nk_anti_aliasing aa)
{
    UIState* state = &renderer->ui_state;
    u32 shader = renderer->ui_renderer.shader;

    m4 projection = m4_ortho(0.0f, (f32)state->width, (f32)state->height, 0.0f, -1.0f, 1.0f);

    shader_bind(shader);
    shader_set_i32(shader, "tex", 0);
    shader_set_m4(shader, "projection", projection, false);
    renderer_viewport(renderer, rect_min_dim(v2_zero(), v2((f32)renderer->window_width, (f32)renderer->window_height)));

    const struct nk_command* cmd;
    nk_foreach(cmd, &state->ctx)
    {
        // TODO(lucas): The lines and points on the chart don't look quite right. The points are offset and the lines
        // don't connect very gracefully. Also had to swap begin and end y in line drawing. Mismatch in coordinate systems?
        // TODO(lucas): The mouse slightly obscures tooltips. Mismatch in coordinate systems?
        switch (cmd->type)
        {
            case NK_COMMAND_NOP: break;

            case NK_COMMAND_SCISSOR:
            {
                const struct nk_command_scissor* s = (const struct nk_command_scissor*)cmd;
                f32 y = (f32)(renderer->window_height - s->h - s->y);
                rect clip = rect_min_dim(v2((f32)s->x, y), v2((f32)s->w, (f32)s->h));
                draw_scissor_test(renderer, clip);
            } break;

            case NK_COMMAND_LINE:
            {
                const struct nk_command_line* l = (const struct nk_command_line*)cmd;
                v4 color = nk_color_to_v4(l->color);
                v2 start = {(f32)l->begin.x, (f32)l->end.y};
                v2 end = {(f32)l->end.x, (f32)l->begin.y};
                draw_line(renderer, start, end, color, (f32)l->line_thickness, 0.0f);
            } break;

            // TODO(lucas): Rounding
            case NK_COMMAND_RECT:
            {
                const struct nk_command_rect* r = (const struct nk_command_rect*)cmd;
                v4 color = nk_color_to_v4(r->color);
                v2 pos = {(f32)r->x, (f32)r->y};
                v2 size = {(f32)r->w, (f32)r->h};
                draw_quad_outline(renderer, pos, size, color, r->line_thickness, 0.0f);
            } break;

            // TODO(lucas): Rounding
            case NK_COMMAND_RECT_FILLED:
            {
                const struct nk_command_rect_filled* r = (const struct nk_command_rect_filled*)cmd;
                v4 color = nk_color_to_v4(r->color);
                v2 pos = {(f32)r->x, (f32)r->y};
                v2 size = {(f32)r->w, (f32)r->h};
                draw_quad(renderer, pos, size, color, 0.0f);
            } break;

            // TODO(lucas): Verify color order
            case NK_COMMAND_RECT_MULTI_COLOR:
            {
                const struct nk_command_rect_multi_color* r = (const struct nk_command_rect_multi_color*)cmd;
                v4 bl = nk_color_to_v4(r->left);
                v4 br = nk_color_to_v4(r->bottom);
                v4 tr = nk_color_to_v4(r->right);
                v4 tl = nk_color_to_v4(r->top);
                v2 pos = {(f32)r->x, (f32)r->y};
                v2 size = {(f32)r->w, (f32)r->h};
                draw_quad_gradient(renderer, pos, size, bl, br, tr, tl, 0.0f);
            } break;

            // TODO(lucas): This command has width and height, so it probably expects more of an ellipse
            // TODO(lucas): Verify center
            case NK_COMMAND_CIRCLE:
            {
                const struct nk_command_circle* c = (const struct nk_command_circle*)cmd;
                v4 color = nk_color_to_v4(c->color);
                v2 center = {(f32)c->x + (f32)c->w/2.0f, (f32)c->y + (f32)c->h/2.0f};
                draw_circle_outline(renderer, center, c->w, color, (f32)c->line_thickness);
            } break;

            case NK_COMMAND_CIRCLE_FILLED:
            {
                const struct nk_command_circle_filled* c = (const struct nk_command_circle_filled*)cmd;
                v4 color = nk_color_to_v4(c->color);
                v2 center = {(f32)c->x + (f32)c->w/2.0f, (f32)c->y + (f32)c->h/2.0f};
                draw_circle(renderer, center, c->w, color);
            } break;

            // TODO(lucas): Verify angles
            case NK_COMMAND_ARC:
            {
                const struct nk_command_arc* a = (const struct nk_command_arc*)cmd;
                v4 color = nk_color_to_v4(a->color);
                v2 center = {(f32)a->cx, (f32)a->cy};
                f32 start_angle = deg_f32(a->a[0]);
                f32 end_angle = deg_f32(a->a[1]);
                draw_ring_outline(renderer, center, a->r, 0.0f, start_angle, end_angle, color, 0.0f, a->line_thickness);
            } break;

            case NK_COMMAND_ARC_FILLED:
            {
                const struct nk_command_arc_filled* a = (const struct nk_command_arc_filled*)cmd;
                v4 color = nk_color_to_v4(a->color);
                v2 center = {(f32)a->cx, (f32)a->cy};
                f32 start_angle = deg_f32(a->a[0]);
                f32 end_angle = deg_f32(a->a[1]);
                draw_ring(renderer, center, a->r, 0.0f, start_angle, end_angle, color, 0.0f);
            };

            case NK_COMMAND_TRIANGLE:
            {
                const struct nk_command_triangle* t = (const struct nk_command_triangle*)cmd;
                v4 color = nk_color_to_v4(t->color);
                v2 a = {(f32)t->a.x, (f32)t->a.y};
                v2 b = {(f32)t->b.x, (f32)t->b.y};
                v2 c = {(f32)t->c.x, (f32)t->c.y};
                draw_triangle_outline(renderer, a, b, c, color, 0.0f, t->line_thickness);
            } break;

            case NK_COMMAND_TRIANGLE_FILLED:
            {
                const struct nk_command_triangle_filled* t = (const struct nk_command_triangle_filled*)cmd;
                v4 color = nk_color_to_v4(t->color);
                v2 a = {(f32)t->a.x, (f32)t->a.y};
                v2 b = {(f32)t->b.x, (f32)t->b.y};
                v2 c = {(f32)t->c.x, (f32)t->c.y};
                draw_triangle(renderer, a, b, c, color, 0.0f);
            } break;

            // TODO(lucas): background color
            // TODO(lucas): Default font (check if font is null)
            case NK_COMMAND_TEXT:
            {
                const struct nk_command_text* t = (const struct nk_command_text*)cmd;
                v4 color = nk_color_to_v4(t->foreground);
                Font* font = (Font*)t->font->userdata.ptr;
                v2 pos = {(f32)t->x, (f32)t->y + (f32)t->font->height*0.75f};
                s8 s = {(u8*)t->string, (size)str_len(t->string)};
                Text text = text_init(s, font, pos, (u32)t->font->height);
                text.color = color;
                draw_text(renderer, text);
            } break;

            case NK_COMMAND_IMAGE:
            {
                const struct nk_command_image* i = (const struct nk_command_image*)cmd;
                v4 tint = nk_color_to_v4(i->col);
                v2 pos = {(f32)i->x, (f32)i->y};
                v2 size = {(f32)i->w, (f32)i->h};
                Texture* tex = (Texture*)i->img.handle.ptr;
                Sprite sprite = sprite_init(tex);
                sprite.color = tint;
                sprite.position = pos;
                sprite.size = size;
                draw_sprite(renderer, sprite);
            } break;

            case NK_COMMAND_CUSTOM:
            {
                const struct nk_command_custom* c = (const struct nk_command_custom*)cmd;
                c->callback(NULL, c->x, c->y, c->w, c->h, c->callback_data);
            }

            default: break;
        }
    }
    nk_clear(&state->ctx);
    shader_unbind();
}

internal void ui_enter_char(UIState* state, u64 code)
{
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

void ui_state_init(Renderer* renderer, Font font, u32 font_size, MemoryArena* arena)
{
    UIState state = {0};

    Font* new_font = push_struct(arena, Font);
    *new_font = font;

    struct nk_user_font user_font = {0};
    user_font.userdata = nk_handle_ptr(new_font);
    user_font.height = (f32)font_size;
    user_font.width = nk_alchemy_font_get_text_width;
    nk_init_default(&state.ctx, &user_font);
    state.user_font = user_font;

    state.ctx.clip.copy = ui_clipboard_copy;
    state.ctx.clip.paste = ui_clipboard_paste;
    // state.ctx.clip.userdata = nk_handle_ptr(&state);

    renderer->ui_state = state;
}

void ui_new_frame(Renderer* renderer, u32 window_width, u32 window_height)
{
    UIState* state = &renderer->ui_state;
    struct nk_context* ctx = &state->ctx;
    Keyboard* keyboard = state->keyboard;
    Mouse* mouse = state->mouse;

    state->width = window_width;
    state->height = window_height;

    nk_input_begin(ctx);

    // text input
    if (keyboard)
    {
        ui_enter_char(state, keyboard->current_char);
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

// TODO(lucas): Clear user font and user data
void ui_state_delete(UIState* state)
{
    nk_free(&state->ctx);
    memset(state, 0, sizeof(*state));
}
