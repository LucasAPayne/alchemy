#include "alchemy/renderer/font.h"
#include "alchemy/renderer/renderer.h"
#include "alchemy/util/math.h"
#include "alchemy/util/memory.h"
#include "alchemy/util/str.h"
#include "alchemy/util/types.h"

#include <glad/gl.h>

#include <freetype/ftoutln.h>

#define MAX_CHARS_PER_DRAW 200

Glyph cache_glyph(Font* font, u32 glyph_idx, u32 charcode)
{
    Glyph new_glyph = {0};
    if (FT_Load_Glyph(font->face, glyph_idx, FT_LOAD_RENDER))
    {
        u8 err[5] = {0};
        utf8_from_codepoint(err, charcode);
        log_error("FreeType2 error: Failed to load character %c (codepoint: %u, glyph index: %u)",
                  err, charcode, glyph_idx);
        if (utf8_get_num_bytes(*err) == 4)
            log_debug("4-byte UTF-8 characters may fail to display in the terminal.");
    }

    FT_Bitmap bmp = font->face->glyph->bitmap;

    glGenTextures(1, &new_glyph.tex_id);
    glBindTexture(GL_TEXTURE_2D, new_glyph.tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bmp.width, bmp.rows, 0, GL_RED, GL_UNSIGNED_BYTE, bmp.buffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    new_glyph.dim = v2((f32)bmp.width, (f32)bmp.rows);
    new_glyph.bearing = v2((f32)font->face->glyph->bitmap_left, (f32)font->face->glyph->bitmap_top);
    new_glyph.advance = font->face->glyph->advance.x;

    return new_glyph;
}

// TODO(lucas): Almost all FreeType functions return an error. Check each of these.

Font font_load_from_file(const char* filename, u32 px, MemoryArena* arena, b32 small_caps)
{
    Font font = {0};
    FT_Library ft;

    if (FT_Init_FreeType(&ft))
        log_error("FreeType2 error: Failed to iniitialize FreeType");

    if (FT_New_Face(ft, filename, 0, &font.face))
        log_error("FreeType2 error: Failed to open font %s", filename);

    if (FT_Set_Pixel_Sizes(font.face, 0, px))
        log_error("FreeType2 error: Failed to set pixel size");

    // TODO(lucas): font.face->num_glyphs seems to be the size of the glyph index space, rather than the
    // number of supported glyphs in the font. This works for now because the glyph index is used as a direct index.
    // However, depending on the font, the glyph cache could become pretty sparse.
    font.px = px;
    font.small_caps = small_caps;
    font.has_kerning = FT_HAS_KERNING(font.face);
    font.glyph_cache = push_array(arena, font.face->num_glyphs, Glyph);
    zero_array(font.glyph_cache, font.face->num_glyphs, Glyph);

    // NOTE(lucas): By default, OpenGL requires that textures are aligned on 4-byte boundaries,
    // but we need 1-byte alignment for grayscale glyph bitmaps
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    f32 max_top = 0.0f;

    u32 glyph_idx = 0;
    u32 charcode = FT_Get_First_Char(font.face, &glyph_idx);
    while (glyph_idx != 0)
    {
        font.glyph_cache[glyph_idx] = cache_glyph(&font, glyph_idx, charcode);

        /* TODO(lucas): For small caps fonts, the ascender is currently set to the tallest capital ASCII letter,
         * assuming there might be a difference in height between them.
         * I'm not sure if this is really correct handling, but it at least works well for all my test fonts.
         * However, using the default ascender, at least for my test fonts, gives the height of lowercase letters
         * so that capital letters stick over the bounds. And using the max height glyph over the entire range gives
         * an ascender that is too large.
         */
        if (small_caps)
        {
            if (FT_Load_Glyph(font.face, glyph_idx, FT_LOAD_NO_BITMAP) == 0)
            {
                if (charcode >= 'A' && charcode <= 'Z')
                {
                    FT_GlyphSlot slot = font.face->glyph;

                    FT_BBox bbox;
                    FT_Outline_Get_CBox(&slot->outline, &bbox);

                    f32 top = (f32)(bbox.yMax >> 6); // Distance above baseline
                    max_top = max(top, max_top);
                }
            }
        }

        charcode = FT_Get_Next_Char(font.face, charcode, &glyph_idx);
    }

    if (small_caps)
        font.ascender = max_top;
    else
        font.ascender = (f32)(font.face->ascender >> 6);

    return font;
}

// NOTE(lucas): Determine width of string in pixels
f32 text_get_width(Text* text)
{
    f32 result = 0.0f;
    f32 scale = (f32)text->px / (f32)text->font->px;
    f32 scale_x = text->scale.x > 0.0f ? text->scale.x : scale;

    for (size i = 0; i < text->string.len; ++i)
    {
        u32 charcode = utf8_get_codepoint(text->string.data + i);
        int num_bytes = utf8_get_num_bytes(text->string.data[i]);
        i += num_bytes - 1;
        if (FT_Load_Char(text->font->face, charcode, FT_LOAD_NO_BITMAP))
        {
            u8 c[5] = {0};
            utf8_from_codepoint(c, charcode);
            log_error("FreeType2 error: Failed to load character %s (codepoint %u)", c, charcode);
            if (utf8_get_num_bytes(*c) == 4)
                log_debug("4-byte UTF-8 characters may fail to display in the terminal.");
        }
        result += (f32)(text->font->face->glyph->advance.x >> 6)*scale_x;
    }

    return result;
}

void text_set_size_px(Text* text, f32 px)
{
    text->px = px;
    text->scale.y = text->px / (f32)text->font->px;
    text->scale.x *= text->scale.y;
    text->string_width = text_get_width(text);
    text->line_height = (f32)(text->font->face->size->metrics.height >> 6)*text->scale.y;
}

void text_scale(Text* text, f32 factor)
{
    f32 new_size = text->px_original*factor;
    text_set_size_px(text, new_size);
}

Text text_init(s8 string, Font* font, v2 position, f32 px)
{
    Text text = {0};

    text.string = string;
    text.font = font;
    text.position = position;
    text.color = color_black();
    text.px_original = px;

    text_set_size_px(&text, px);

    return text;
}

void output_text(Renderer* renderer, RenderCommandText* cmd)
{
    Text text = cmd->text;
    Font* font = text.font;

    f32 scale_y = text.scale.y;
    f32 scale_x = text.scale.x > 0.0f ? text.scale.x : scale_y;

    FT_Face face = font->face;
    FT_GlyphSlot glyph = face->glyph;
    FT_UInt glyph_index = 0;
    FT_UInt previous_glyph_index = 0;

    shader_set_v4(renderer->font_renderer.shader, "text_color", text.color);
    glBindVertexArray(renderer->font_renderer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->font_renderer.vbo);

    f32 x = text.position.x;
    f32 y = text.position.y;

    for (size i = 0; i < text.string.len; ++i)
    {
        u8* c = text.string.data + i;

        if ((*c == '\r') && (*(c+1) == '\n'))
        {
            // If \r\n is used to end a line, need to skip the next character (\n)
            y += text.line_height;
            x = text.position.x;
            ++i;
            previous_glyph_index = 0;
            continue;
        }
        else if (*c == '\n')
        {
            y += text.line_height;
            x = text.position.x;
            previous_glyph_index = 0;
            continue;
        }

        // TODO(lucas): Decode entire string at once.
        int num_bytes = utf8_get_num_bytes(*c);
        u32 charcode = utf8_get_codepoint(c);
        i += num_bytes-1;
        glyph_index = FT_Get_Char_Index(face, charcode);

        // When appropriate, retrieve kerning information and advance cursor
        if (font->has_kerning && previous_glyph_index && glyph_index)
        {
            FT_Vector delta;
            FT_Get_Kerning(face, previous_glyph_index, glyph_index, FT_KERNING_DEFAULT, &delta);
            x += (f32)(delta.x >> 6)*scale_x;
            previous_glyph_index = glyph_index;
        }

        // TODO(lucas): Extra width should only be added once
        Glyph* g = font->glyph_cache + glyph_index;
        if (*c == ' ')
        {
            x += (f32)(g->advance >> 6)*scale_x + text.extra_width_per_space;
            continue;
        }

        f32 x2 = x + (f32)g->bearing.x*scale_x;
        f32 y2 = y - (f32)g->bearing.y*scale_y;
        f32 w = g->dim.x*scale_x;
        f32 h = g->dim.y*scale_y;

        m4 transform = transform = m4_translate(m4_identity(), v3(x2, y2, 0.0f));
        transform = m4_scale(transform, v3(w, h , 0.0f));
        shader_set_m4(renderer->font_renderer.shader, "transform", transform, false);
        texture_bind_id(g->tex_id, 0);
        glBindBuffer(GL_ARRAY_BUFFER, renderer->font_renderer.vbo);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);

        x += (g->advance >> 6)*scale_x;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

internal b32 text_is_whitespace(Text text)
{
    b32 result = false;

    for (size i = 0; i < text.string.len; ++i)
    {
        if (!char_is_whitespace(text.string.data[i]))
            break;

        result = true;
    }

    return result;
}

TextArea text_area_init(Renderer* renderer, rect bounds, s8 str, Font* font, f32 text_size_px)
{
    TextArea result = {0};
    result.bounds = bounds;
    // v2 text_pos = {bounds.x, result.bounds.y + result.bounds.height - text_size_px};
    v2 text_pos = v2(result.bounds.x, result.bounds.y);
    result.text = text_init(str, font, text_pos, text_size_px);
    return result;
}

void text_area_scale(TextArea* text_area, f32 factor)
{
    text_area->bounds.size = v2_scale(text_area->bounds.size, factor);
    text_scale(&text_area->text, factor);
}

internal f32 get_text_height(TextArea* text_area)
{
    i32 lines_req = 1;

    s8 s = text_area->text.string;
    size begin = 0;
    size whitespace_begin = 0;
    Text token = {0};
    Text whitespace = {0};

    f32 line_width = 0.0f;
    for (size i = 0; i < s.len; ++i)
    {
        u8 c = s.data[i];

        if (c == '\n')
        {
            ++lines_req;
            line_width = 0.0f;
            continue;
        }

        u32 num_whitespaces = 0;
        while (char_is_whitespace(c))
        {
            whitespace_begin = i;
            ++num_whitespaces;

            if (num_whitespaces == 1)
            {
                s8 slice = s8_slice(s, begin, i-1);
                token = text_init(slice, text_area->text.font, v2_zero(), text_area->text.px);
                line_width += token.string_width;

            }

            if (line_width > text_area->bounds.width)
            {
                // If a word pushes the line over the bounds, then carry over its width.
                ++lines_req;
                line_width = token.string_width;
            }

            ++i;
            c = s.data[i];
        }

        if (num_whitespaces)
        {
            s8 s_whitespace = s8_slice(s, whitespace_begin, i-1);
            whitespace = text_init(s_whitespace, text_area->text.font, v2_zero(), text_area->text.px);
            line_width += whitespace.string_width;
            if (line_width > text_area->bounds.width)
            {
                // If whitespace pushes the line over the bounds, then reset the width.
                ++lines_req;
                line_width = 0.0f;

            }
            begin = i;

            // NOTE(lucas): When the end of whitespace is reached, i is at the beginning of the next word.
            // But the end of the loop will also increment it, so it needs to go back by 1.
            --i;
        }
    }

    s8 slice = s8_slice(s, begin, s.len);
    token = text_init(slice, text_area->text.font, v2_zero(), text_area->text.px);
    line_width += token.string_width;
    if (line_width > text_area->bounds.width)
        ++lines_req;

    f32 text_height = (f32)(lines_req)*text_area->text.line_height;
    return text_height;
}

// Find the best font size to fit the text area bounds as exactly as possible when
// the text must shrink to fit and wrap.
internal void find_best_px(TextArea* text_area)
{
    f32 h = 0.0f;
    f32 max_h = text_area->bounds.height;

    u32 attempts = 0;

    f32 low = 0.0f;
    f32 high = text_area->text.px;
    f32 eps = 0.1f; // Acceptable error on font's pixel size
    while (abs_f32(high - low) >= eps)
    {
        ++attempts;
        f32 mid = low + (high - low) / 2.0f;
        text_set_size_px(&text_area->text, mid);
        h = get_text_height(text_area);

        if (h > max_h)
            high = mid;
        else
            low = mid;
    }

    text_set_size_px(&text_area->text, low);

    log_debug("Binary search took %u iterations", attempts);
}

internal b32 text_in_bounds(TextArea* text_area)
{
    b32 result = text_area->text.position.y < text_area->bounds.position.y + text_area->bounds.height;
    return result;
}

internal void flush_line(Renderer* renderer, TextArea* text_area, Text line_text, f32 line_width, u32 spaces_in_line,
    b32 last_line)
{
    Text* text  = &text_area->text;
    Font* font = text->font;
    f32 max_width = text_area->bounds.width;

    f32 width_remaining = max_width - line_width;
    switch (text_area->horiz_alignment)
    {
        case TEXT_ALIGN_HORIZ_LEFT: break;

        case TEXT_ALIGN_HORIZ_JUSTIFIED:
        {
            if (!last_line)
                line_text.extra_width_per_space = width_remaining / (f32)spaces_in_line;
        } break;

        case TEXT_ALIGN_HORIZ_RIGHT:
        {
            line_text.position = v2_add(line_text.position, v2(width_remaining, 0.0f));
        } break;

        case TEXT_ALIGN_HORIZ_CENTER:
        {
            line_text.position = v2_add(line_text.position, v2(0.5f*width_remaining, 0.0f));
        } break;

        INVALID_DEFAULT_CASE();
    }

    draw_text(renderer, line_text);
}

internal void parse_and_draw_text(Renderer* renderer, TextArea* text_area)
{
    s8 s = text_area->text.string;
    Text* text  = &text_area->text;
    Font* font = text->font;
    f32 max_width = text_area->bounds.width;

    i32 line = 0;
    i32 spaces_in_line = 0;

    size line_begin = 0;
    size begin = 0;
    size whitespace_begin = 0;
    Text token = {0};
    Text whitespace = {0};

    f32 line_width = 0.0f;
    for (size i = 0; i < s.len; ++i)
    {
        u8 c = s.data[i];

        if (c == '\n')
        {
            line_width = 0.0f;
            continue;
        }

        u32 num_whitespaces = 0;
        while (char_is_whitespace(c))
        {
            whitespace_begin = i;
            ++num_whitespaces;

            if (num_whitespaces == 1)
            {
                ++spaces_in_line;

                s8 slice = s8_slice(s, begin, i-1);
                token = text_init(slice, font, v2_zero(), text->px);
                line_width += token.string_width;
            }

            ++i;
            c = s.data[i];

            if (line_width > max_width)
            {
                spaces_in_line -= 2;
                size whitespace_end = whitespace_begin + num_whitespaces;
                s8 s_whitespace = s8_slice(s, whitespace_begin, whitespace_begin);
                whitespace = text_init(s_whitespace, font, v2_zero(), text->px);

                size token_end = whitespace_begin;
                size line_end = token_end - token.string.len - 2;

                line_width -= (token.string_width + whitespace.string_width);

                s8 line_slice = s8_slice(s, line_begin, line_end);
                v2 pos = v2(text->position.x, text->position.y + ((f32)line*text->line_height));
                Text line_text = text_init(line_slice, font, pos, text->px);

                flush_line(renderer, text_area, line_text, line_width, spaces_in_line, false);

                // If a word pushes the line over the bounds, then carry over its width.
                line_width = token.string_width;
                ++line;
                spaces_in_line = 1; // Because there's an overflow word, there is an extra space for the next line
                line_begin = line_end+2;
            }
        }

        if (num_whitespaces)
        {
            s8 s_whitespace = s8_slice(s, whitespace_begin, i-1);
            whitespace = text_init(s_whitespace, font, v2_zero(), text->px);
            line_width += whitespace.string_width;
            if (line_width > max_width)
            {
                --spaces_in_line;
                size line_end = whitespace_begin;

                line_width -= whitespace.string_width;

                s8 line_slice = s8_slice(s, line_begin, line_end);
                v2 pos = v2(text->position.x, text->position.y + ((f32)line*text->line_height));
                Text line_text = text_init(line_slice, font, pos, text->px);
                flush_line(renderer, text_area, line_text, line_width, spaces_in_line, false);

                // If whitespace pushes the line over the bounds, then reset the width.
                line_width = 0.0f;
                ++line;
                spaces_in_line = 0;
                line_begin = line_end+s_whitespace.len;
            }
            begin = i;

            // NOTE(lucas): When the end of whitespace is reached, i is at the beginning of the next word.
            // But the end of the loop will also increment it, so it needs to go back by 1.
            --i;
        }
    }

    if (line == 0)
        begin = 0;
    s8 slice = s8_slice(s, begin, s.len-1);
    token = text_init(slice, font, v2_zero(), text->px);
    line_width = token.string_width;

    line_begin = begin;
    size line_end = s.len-1;
    s8 line_slice = s8_slice(s, line_begin, line_end);
    v2 pos = v2(text->position.x, text->position.y + ((f32)line*text->line_height));
    Text line_text = text_init(line_slice, font, pos, text->px);
    flush_line(renderer, text_area, line_text, line_width, spaces_in_line, true);
}

void draw_text_area(Renderer* renderer, TextArea* text_area)
{
    f32 text_height = 0.0f;
    text_area->text.position = v2(text_area->bounds.x, text_area->bounds.y);
    if (text_area->style & TEXT_AREA_SHRINK_TO_FIT)
    {
        if (text_area->text.px > text_area->bounds.height)
        {
            f32 delta = text_area->text.px - text_area->bounds.height;
            f32 new_size = text_area->text.px - delta;
            text_area->text.position.y += delta;
            text_set_size_px(&text_area->text, new_size);
        }

        // NOTE(lucas): Wrap text and shrink to fit.
        // Wrap text, and if text height exceeds bounds, decrease font size.
        if (text_area->style & TEXT_AREA_WRAP)
        {
            text_height = get_text_height(text_area);
            if (text_height > text_area->bounds.height)
            {
                find_best_px(text_area);
                text_height = get_text_height(text_area);
            }

            if (text_area->text.px < 1.0f)
                return;
        }
        else
        {
            // NOTE(lucas): Don't wrap but shrink text to fit.
            text_height = text_area->text.px;
            f32 text_width = text_get_width(&text_area->text);
            if (text_width > text_area->bounds.width)
                text_area->text.scale.x = (text_area->bounds.width / text_width) * (0.99f * text_area->text.scale.y);
        }
    }
    // NOTE(lucas): Wrap but don't shrink to fit
    else if (text_area->style & TEXT_AREA_WRAP)
    {
        text_height = get_text_height(text_area);
    }
    // NOTE(lucas): Don't wrap or shrink to fit
    else
    {
        text_height = text_area->text.px;
    }

    f32 vert_space_remaining = text_area->bounds.height - text_height;

    // TODO(lucas): Cache the tallest character and use that as the ascender
    // The y position of the text area is the top, which the text will view as the baseline.
    // To correct this, use the ascender (baseline to highest glyph position) to make the text flush with the top bound.
    FT_Face face = text_area->text.font->face;
    f32 ascent = text_area->text.font->ascender;
    if (text_area->text.font->small_caps)
        ascent *= text_area->text.scale.y;

    f32 descent = -(f32)(text_area->text.font->face->descender >> 6);
    text_area->text.position.y = text_area->bounds.position.y + ascent;

    // TODO(lucas): Currently, the descender is added to bottom-aligned text.
    // This means that descending characters like "y" can extend below the box, and everything is flush with the bottom bound.
    // Revisit whether this seems "correct" after using it.
    switch (text_area->vert_alignment)
    {
        case TEXT_ALIGN_VERT_TOP:    break;
        case TEXT_ALIGN_VERT_BOTTOM: text_area->text.position.y += vert_space_remaining + descent; break;
        case TEXT_ALIGN_VERT_CENTER: text_area->text.position.y += 0.5f*vert_space_remaining;      break;
        INVALID_DEFAULT_CASE();
    }

    // TODO(lucas): If text is not shrink to fit, discard any text that overflows the y bound
    parse_and_draw_text(renderer, text_area);
}
