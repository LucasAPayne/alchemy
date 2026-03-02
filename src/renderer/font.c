#include "alchemy/renderer/font.h"
#include "alchemy/renderer/renderer.h"
#include "alchemy/util/math.h"
#include "alchemy/util/memory.h"
#include "alchemy/util/str.h"
#include "alchemy/util/types.h"

#include <glad/gl.h>

#include <freetype/ftoutln.h>

// Generate the grayscale texture for one glyph from a font
internal Glyph cache_glyph(Font* font, u32 glyph_idx, u32 charcode)
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

// TODO(lucas): Rework glyph caching to cache an atlas instead of individual textures.
// TODO(lucas): Almost all FreeType functions return an error. Check each of these.
// TODO(lucas): Some fields of the font need to be checked before using certain metrics.
// See here: https://freetype.org/freetype2/docs/tutorial/step2.html

Font font_load_from_file(const char* filename, u32 px, MemoryArena* arena)
{
    Font font = {0};
    FT_Library ft;

    // Init FreeType and load the font at the specific px size.
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
    font.has_kerning = FT_HAS_KERNING(font.face);
    font.glyph_cache = push_array(arena, font.face->num_glyphs, Glyph);
    zero_array(font.glyph_cache, font.face->num_glyphs, Glyph);

    // NOTE(lucas): By default, OpenGL requires that textures are aligned on 4-byte boundaries,
    // but we need 1-byte alignment for grayscale glyph bitmaps
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Generate and cache textures for all glyphs in the font.
    // They are stored at the original px size, and the user can scale them as needed.
    u32 glyph_idx = 0;
    u32 charcode = FT_Get_First_Char(font.face, &glyph_idx);
    while (glyph_idx != 0)
    {
        font.glyph_cache[glyph_idx] = cache_glyph(&font, glyph_idx, charcode);

        if (FT_Load_Glyph(font.face, glyph_idx, FT_LOAD_NO_BITMAP) == 0)
        {
            /* TODO(lucas): For now, a font will cache the metric for the tallest A-Z character for the
             * purposes of vertical alignment.
             * In the future, it will probably make more sense to get this measurement for a specific line of text
             * to get more consistency.
             */
            if (charcode >= 'A' && charcode <= 'Z')
            {
                FT_GlyphSlot slot = font.face->glyph;

                FT_BBox bbox;
                FT_Outline_Get_CBox(&slot->outline, &bbox);

                f32 top = (f32)(bbox.yMax >> 6); // Distance above baseline
                font.max_top = max(top, font.max_top);
            }
        }

        charcode = FT_Get_Next_Char(font.face, charcode, &glyph_idx);
    }

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

    // Since the px size changed, the measurements that depend on it need to change as well.
    text->scale.y = text->px / (f32)text->font->px;
    text->scale.x = text->scale.x == 0.0f ? 0.0f : text->scale.y;
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

// Returns whether all characters in the string of a `Text` object are whitespace.
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
    v2 text_pos = v2(result.bounds.x, result.bounds.y);
    result.text = text_init(str, font, text_pos, text_size_px);
    return result;
}

void text_area_scale(TextArea* text_area, f32 factor)
{
    text_area->bounds.size = v2_scale(text_area->bounds.size, factor);
    text_scale(&text_area->text, factor);
}

// Get the minimum pixel height required by the text in a text area, assuming the text wraps.
// Other text styling, such as being flush on both the top and bottom bounds, does not affect this measurement.
internal f32 get_text_height(TextArea* text_area)
{
    i32 lines_req = 1;

    Text* text = &text_area->text;
    s8 s = text_area->text.string;
    Font* font = text->font;
    f32 max_width = text_area->bounds.width;

    Text word = {0};
    Text space = {0};

    f32 line_width = 0.0f;
    size i = 0;
    while (i < s.len)
    {
        // Handle explicit newline
        if (s.data[i] == '\n')
        {
            ++lines_req;
            line_width = 0.0f;

            ++i;
            continue;
        }

        // Consume a word
        size word_begin = i;
        while (i < s.len && !char_is_whitespace(s.data[i]) && s.data[i] != '\n')
            ++i;

        s8 word_slice = s8_slice(s, word_begin, i);
        word = text_init(word_slice, font, v2_zero(), text->px);

        // Chedk if the word overflowed the line
        if (line_width + word.string_width > max_width && line_width > 0.0f)
        {
            // Move to the next line and reset the metrics
            ++lines_req;
            line_width = 0.0f;
        }

        line_width += word.string_width;

        // Consume whitespace
        size space_begin = i;
        while (i < s.len && char_is_whitespace(s.data[i]) && s.data[i] != '\n')
            ++i;

        // Check if there was any whitesapce
        if (i > space_begin)
        {
            // NOTE(lucas): While the space could overflow the line, it is simpler to catch all overflow
            // with the next word that gets consumed.
            s8 space_slice = s8_slice(s, space_begin, i);
            space = text_init(space_slice, font, v2_zero(), text->px);
            line_width += space.string_width;
        }
    }

    f32 text_height = (f32)(lines_req)*text_area->text.line_height;
    return text_height;
}

// Find the best font size to fit the text area bounds as exactly as possible when
// the text must shrink to fit and wrap.
internal void find_best_px(TextArea* text_area)
{
    f32 h = 0.0f;
    f32 max_h = text_area->bounds.height;

    f32 low = 0.0f;
    f32 high = text_area->text.px;
    f32 eps = 0.1f; // Acceptable error on font's pixel size
    b32 done = false;

    // Binary search
    while (!done)
    {
        f32 mid = low + (high - low) / 2.0f;
        text_set_size_px(&text_area->text, mid);
        h = get_text_height(text_area);

        if (h > max_h)
            high = mid;
        else
            low = mid;

        if (h <= max_h && abs_f32(high - low) < eps)
            done = true;
    }

    text_set_size_px(&text_area->text, low);
}

// Submit a draw command for a line of text in a `TextArea`.
internal void flush_line(Renderer* renderer, TextArea* text_area, Text line_text, f32 line_width, i32 spaces_in_line,
    b32 last_line)
{
    Text* text = &text_area->text;
    f32 max_width = text_area->bounds.width;

    f32 width_remaining = max_width - line_width;
    ASSERT(width_remaining >= 0.0f, "Negative width remaining");
    ASSERT(spaces_in_line >= 0, "Negative spaces in line");
    switch (text_area->horiz_alignment)
    {
        case TEXT_ALIGN_HORIZ_LEFT:   break;
        case TEXT_ALIGN_HORIZ_RIGHT:  line_text.position.x += width_remaining;      break;
        case TEXT_ALIGN_HORIZ_CENTER: line_text.position.x += 0.5f*width_remaining; break;

        case TEXT_ALIGN_HORIZ_JUSTIFIED:
        {
            // If the text is justified, distribute the remaining width among all spaces in the line.
            // However, the final line in a text area should not be justified.
            if (!last_line && spaces_in_line != 0)
                line_text.extra_width_per_space = width_remaining / (f32)spaces_in_line;
        } break;

        INVALID_DEFAULT_CASE();
    }

    draw_text(renderer, line_text);
}

// Parse text line by line, according to `TextArea` style, and submit draw commands for each line.
internal void parse_and_draw_text(Renderer* renderer, TextArea* text_area)
{
    // If the text should not wrap, simply perform horizontal alignment and draw the text as-is.
    if (!(text_area->style & TEXT_AREA_WRAP))
    {
        f32 width_remaining = text_area->bounds.width - text_area->text.string_width;
        ASSERT(width_remaining >= 0.0f, "Negative width remaining");

        /* NOTE(lucas): Directly submitting text_area->text to draw_text() results in access violations with
         * the /Og flag enabled in MSVC.
         * Slicing the whole string and making another copy fixes it and matches the behavior of other paths.
         */
        s8 line_slice = s8_slice(text_area->text.string, 0, text_area->text.string.len);
        v2 pos = text_area->text.position;
        Text line_text = text_init(line_slice, text_area->text.font, pos, text_area->text.px);
        line_text.scale = text_area->text.scale;

        switch (text_area->horiz_alignment)
        {
            case TEXT_ALIGN_HORIZ_LEFT:      break;
            case TEXT_ALIGN_HORIZ_JUSTIFIED: break; // One line of justified text is the same as left-aligned.
            case TEXT_ALIGN_HORIZ_CENTER:    line_text.position.x += 0.5f * width_remaining; break;
            case TEXT_ALIGN_HORIZ_RIGHT:     line_text.position.x += width_remaining;        break;
            INVALID_DEFAULT_CASE();
        }
        draw_text(renderer, line_text);
        return;
    }

    Text* text = &text_area->text;
    s8 s = text->string;
    Font* font = text->font;
    f32 max_width = text_area->bounds.width;

    size line_begin = 0;
    size word_begin = 0;
    size space_begin = 0;
    f32 line_width = 0.0f;
    i32 spaces_in_line = 0;
    i32 line_idx = 0;

    Text word = {0};
    Text space = {0};

    size i = 0;
    while (i < s.len)
    {
        // Handle explicit newline (no horizontal justification)
        if (s.data[i] == '\n')
        {
            s8 line_slice = s8_slice(s, line_begin, i);
            v2 pos = v2(text->position.x, text->position.y + line_idx*text->line_height);

            Text line_text = text_init(line_slice, font, pos, text->px);
            // Pass max_width instead of line_width to avoid justification
            flush_line(renderer, text_area, line_text, max_width, spaces_in_line, false);

            // Reset everything and move immediately to the next line.
            ++line_idx;
            ++i;
            line_begin = i;
            line_width = 0.0f;
            spaces_in_line = 0;
            continue;
        }

        // Consume a word
        word_begin = i;
        while (i < s.len && !char_is_whitespace(s.data[i]) && s.data[i] != '\n')
            ++i;

        s8 word_slice = s8_slice(s, word_begin, i);
        word = text_init(word_slice, font, v2_zero(), text->px);

        // Check if the word overflowed the line
        if (line_width + word.string_width > max_width && line_width > 0.0f)
        {
            s8 line_slice = s8_slice(s, line_begin, space_begin);
            v2 pos = v2(text->position.x, text->position.y + line_idx*text->line_height);

            Text line_text = text_init(line_slice, font, pos, text->px);

            // When a word overflows the line, the space following the last word of the line
            // should not be counted.
            line_width -= space.string_width;
            flush_line(renderer, text_area, line_text, line_width, spaces_in_line-1, false);

            // Move to the next line and reset the metrics
            ++line_idx;
            line_begin = word_begin;
            line_width = 0.0f;
            spaces_in_line = 0;
        }

        line_width += word.string_width;

        // Consume whitespace
        space_begin = i;
        while (i < s.len && char_is_whitespace(s.data[i]) && s.data[i] != '\n')
            ++i;

        // Check if there was any whitespace
        if (i > space_begin)
        {
            s8 space_slice = s8_slice(s, space_begin, i);
            space = text_init(space_slice, font, v2_zero(), text->px);

            // NOTE(lucas): While the space could overflow the line, it is simpler to catch all overflow
            // with the next word that gets consumed.
            ++spaces_in_line;
            line_width += space.string_width;
        }
    }

    // Flush final line
    if (line_begin < s.len)
    {
        s8 line_slice = s8_slice(s, line_begin, s.len);
        v2 pos = v2(text->position.x, text->position.y + line_idx*text->line_height);

        Text line_text = text_init(line_slice, font, pos, text->px);
        flush_line(renderer, text_area, line_text, line_width, spaces_in_line, true);
    }
}

void draw_text_area(Renderer* renderer, TextArea* text_area)
{
    f32 text_height = 0.0f;
    text_area->text.position = v2(text_area->bounds.x, text_area->bounds.y);
    if (text_area->style & TEXT_AREA_SHRINK_TO_FIT)
    {
        // Shrink to fit, and the text height is larger than the overall bounds height.
        // TODO(lucas): Consider calculating the tallest character in the string, rather than the tallest overall.
        text_height = text_area->text.font->max_top*text_area->text.scale.y;
        if (text_height > text_area->bounds.height)
        {
            // Resize the text to be exactly the same size as the bounds.
            // The text needs to move down by the difference in the original size of the text and the bounds size.
            f32 delta = text_height - text_area->bounds.height;
            text_area->text.position.y += delta;
            f32 new_size = text_area->bounds.height;
            text_set_size_px(&text_area->text, new_size);
            text_height = text_area->text.font->max_top*text_area->text.scale.y;
        }

        // Wrap text and shrink to fit.
        if (text_area->style & TEXT_AREA_WRAP)
        {
            // If text height exceeds bounds, find the best pixel size to fit the bounds as closely as possible.
            text_height = get_text_height(text_area);
            if (text_height > text_area->bounds.height)
            {
                find_best_px(text_area);
                text_height = get_text_height(text_area);
            }

            // TODO(lucas): Test with very small text area
            if (text_area->text.px < 1.0f)
                return;
        }
        // Don't wrap but shrink text to fit.
        else
        {
            // If text width exceeds bounds, squeeze text horizontally to fit.
            text_height = text_area->text.font->max_top*text_area->text.scale.y;
            f32 text_width = text_get_width(&text_area->text);

            // NOTE(lucas): The text's x scale starts at 0, so in addition to scaling with relation to the bounds,
            // it also needs to inherit the y scale.
            if (text_width > text_area->bounds.width)
                text_area->text.scale.x = (text_area->bounds.width / text_width)*text_area->text.scale.y;
        }
    }
    // Wrap but don't shrink to fit
    else if (text_area->style & TEXT_AREA_WRAP)
    {
        text_height = get_text_height(text_area);
    }
    // Don't wrap or shrink to fit
    else
    {
        text_height = text_area->text.font->max_top*text_area->text.scale.y;
    }

    f32 vert_space_remaining = text_area->bounds.height - text_height;

    // The y position of the text area is the top, which the text will view as the baseline.
    // To correct this, use the ascender (baseline to highest glyph position) to make the text flush with the top bound.
    f32 ascent = text_area->text.font->max_top*text_area->text.scale.y;
    text_area->text.position.y += ascent;

    /* TODO(lucas): Currently, the descender not subtracted from bottom-aligned text.
    * This means that descending characters like "y" can extend below the box, and everything is flush with the bottom bound.
    * Revisit whether this seems "correct" after using it more.
    */
    // f32 descent = -(f32)(text_area->text.font->face->descender >> 6);

    switch (text_area->vert_alignment)
    {
        case TEXT_ALIGN_VERT_TOP:    break;
        case TEXT_ALIGN_VERT_BOTTOM: text_area->text.position.y += vert_space_remaining;      break;
        case TEXT_ALIGN_VERT_CENTER: text_area->text.position.y += 0.5f*vert_space_remaining; break;
        INVALID_DEFAULT_CASE();
    }

    // TODO(lucas): If text is not shrink to fit, discard any text that overflows the y bound
    parse_and_draw_text(renderer, text_area);
}
