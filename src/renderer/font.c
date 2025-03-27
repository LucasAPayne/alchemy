#include "alchemy/renderer/font.h"
#include "alchemy/renderer/renderer.h"
#include "alchemy/util/math.h"
#include "alchemy/util/memory.h"
#include "alchemy/util/str.h"
#include "alchemy/util/types.h"

#include <glad/glad.h>

// TODO(lucas): Almost all FreeType functions return an error. Check each of these.

Font font_load_from_file(const char* filename)
{
    Font font = {0};
    FT_Library ft;

    if (FT_Init_FreeType(&ft))
        log_error("FreeType2 error: Failed to iniitialize FreeType");

    if (FT_New_Face(ft, filename, 0, &font.face))
        log_error("FreeType2 error: Failed to open font %s", filename);

    return font;
}

// NOTE(lucas): Determine width of string in pixels
f32 text_get_width(Text* text)
{
    f32 result = 0.0f;

    FT_Set_Pixel_Sizes(text->font->face, text->px_width, text->px);
    FT_GlyphSlot glyph = text->font->face->glyph;

    for (size i = 0; i < text->string.len; ++i)
    {
        u32 charcode = utf8_get_codepoint(text->string.data + i);
        if (FT_Load_Char(text->font->face, charcode, FT_LOAD_NO_BITMAP))
        {
            u8 c[5] = {0};
            utf8_from_codepoint(c, charcode);
            log_error("FreeType2 error: Failed to load character %s (codepoint %u)", c, charcode);
            if (utf8_get_num_bytes(*c) == 4)
                log_debug("4-byte UTF-8 characters may fail to display in the terminal.");
        }

        result += text->font->face->glyph->advance.x/64;
    }

    return result;
}

void text_set_size_px(Text* text, u32 px)
{
    text->px = px;
    FT_Set_Pixel_Sizes(text->font->face, 0, text->px);
    text->px_width = FT_MulFix(text->font->face->units_per_EM, text->font->face->size->metrics.x_scale) / 64;

    text->string_width = text_get_width(text);
    text->line_height = (f32)text->font->face->size->metrics.height/64;
}

void text_scale(Text* text, f32 factor)
{
    u32 new_size = (u32)((f32)text->px*factor);
    text_set_size_px(text, new_size);
}

Text text_init(s8 string, Font* font, v2 position, u32 px)
{
    Text text = {0};

    text.string = string;
    text.font = font;
    text.position = position;
    text.color = color_black();

    text_set_size_px(&text, px);

    return text;
}

void output_text(Renderer* renderer, RenderCommandText* cmd)
{
    Text text = cmd->text;
    
    // Set font size in pixels
    FT_Set_Pixel_Sizes(text.font->face, text.px_width, text.px);
    FT_Face face = text.font->face;
    FT_GlyphSlot glyph = face->glyph;
    FT_Bool use_kerning = FT_HAS_KERNING(text.font->face);
    FT_UInt glyph_index = 0;
    FT_UInt previous_glyph_index = 0;

    shader_set_v4(renderer->font_renderer.shader, "text_color", text.color);
    glBindVertexArray(renderer->font_renderer.vao);

    // NOTE(lucas): By default, OpenGL requires that textures are aligned on 4-byte boundaries,
    // but we need 1-byte alignment for grayscale glyph bitmaps
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    Texture texture = texture_generate(0);

    f32 x = text.position.x;
    f32 y = text.position.y;

    for (size i = 0; i < text.string.len; ++i)
    {
        u8* c = text.string.data + i;

        // TODO(lucas): Decode entire string at once.
        u32 charcode = utf8_get_codepoint(c);
        int num_bytes = utf8_get_num_bytes(*c);
        i += num_bytes-1;
        glyph_index = FT_Get_Char_Index(face, charcode);

        // When appropriate, retrieve kerning information and advance cursor
        if (use_kerning && previous_glyph_index && glyph_index)
        {
            FT_Vector delta;
            FT_Get_Kerning(face, previous_glyph_index, glyph_index, FT_KERNING_DEFAULT, &delta);
            x += (f32)delta.x/64;
        }

        previous_glyph_index = glyph_index;

        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER))
        {
            u8 err[5] = {0};
            utf8_from_codepoint(err, charcode);
            log_error("FreeType2 error: Failed to load character %c (codepoint: %u, glyph index: %u)", err, charcode, glyph_index);
            if (utf8_get_num_bytes(*err) == 4)
                log_debug("4-byte UTF-8 characters may fail to display in the terminal.");
        }

        // TODO(lucas): Only do texture generation once when the font is loaded.
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RED,
                     glyph->bitmap.width,
                     glyph->bitmap.rows,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     glyph->bitmap.buffer);

        f32 x2 = x + (f32)glyph->bitmap_left;
        f32 y2 = y - (f32)glyph->bitmap_top;
        f32 w = (f32)glyph->bitmap.width;
        f32 h = (f32)glyph->bitmap.rows;

        f32 vertices[] =
        {
            x2 + w, y2, 1.0f, 0.0f,
            x2 + w, y2 + h, 1.0f, 1.0f,
            x2,     y2 + h, 0.0f, 1.0f,
            x2,     y2, 0.0f, 0.0f,
        };

        // TODO(lucas): Update with glBufferSubData?
        // Update VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, renderer->font_renderer.vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Advance cursor for next glyph
        if ((*c == '\r') && (*(c+1) == '\n'))
        {
            // If \r\n is used to end a line, need to skip the next character (\n)
            y += text.line_height;
            x = text.position.x;
            ++i;
        }
        else if ((*c == '\n'))
        {
            y += text.line_height;
            x = text.position.x;
        }
        else
            x += glyph->advance.x/64;
    }

    glBindVertexArray(0);
    texture_unbind(0);
    texture_delete(&texture);
}

typedef struct TextNode TextNode;
struct TextNode
{
    Text text;
    TextNode* next;
};

typedef struct OverflowText
{
    Text word;
    Text space;
} OverflowText;

typedef struct ParsedText
{
    f32 width;
    f32 height;
    TextNode* first_node;
} ParsedText;

#pragma optimize("", off)
internal void parsed_text_push(ParsedText* parsed_text, Text* text, MemoryArena* arena)
{
    TextNode* new_node = push_struct(arena, TextNode);
    new_node->text = *text;
    new_node->next = 0;

    TextNode* it = parsed_text->first_node;
    if (!it)
        parsed_text->first_node = new_node;
    else
    {
        while (it->next)
            it = it->next;
        
        it->next = new_node;
    }
}
#pragma optimize("", on)

internal b32 char_is_whitespace(char c)
{
    b32 result = (c == ' ' ) || (c == '\t') || (c == '\v') || (c == '\f');
    return result;
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

// TODO(lucas): Take another look at the tokenizer structure
internal Text tokenizer_process_token(s8_iter* tokenizer, ParsedText* parsed_text, Text token, MemoryArena* arena)
{
    Text result = token;

    usize word_len = tokenizer->at - token.string.data;
    result.string = s8_copyn(token.string, word_len, arena);
    result.string_width = text_get_width(&result);
    return result;
}

#pragma optimize("", off)
internal ParsedText parse_text(s8_iter* tokenizer, TextArea* text_area, OverflowText* overflow_text, MemoryArena* arena)
{
    ParsedText parsed_text = {0};
    Text token = text_area->text;
    token.string.data = tokenizer->at;

    int num_spaces = 0;
    b32 line_overflowed = false;

    if (overflow_text->word.string.data)
    {
        parsed_text_push(&parsed_text, &overflow_text->word, arena);
        parsed_text_push(&parsed_text, &overflow_text->space, arena);
        parsed_text.width += overflow_text->word.string_width + overflow_text->space.string_width;
        token.position.x = text_area->text.position.x + parsed_text.width;
        ++num_spaces;
    }

    // NOTE(lucas): After overflow has been written, make sure to reset it again.
    overflow_text->word = (Text){0};
    overflow_text->space = (Text){0};

    // NOTE(lucas): Break up into words. For now, keep punctuation with the word as one node.
    b32 parsing = true;
    while (parsing)
    {
        size len = tokenizer->at - token.string.data;
        b32 final_token = (tokenizer->idx == token.string.len) || (tokenizer->at[0] == '\0');
        if (char_is_whitespace(tokenizer->at[0]) || final_token)
        {
                Text word = tokenizer_process_token(tokenizer, &parsed_text, token, arena);
                parsed_text.width += word.string_width;
                token.string.data = tokenizer->at;

                if (!final_token)
                {
                    while (tokenizer->at[0] && char_is_whitespace(tokenizer->at[0]))
                        s8_iter_move(tokenizer, 1);
                }

                Text space = tokenizer_process_token(tokenizer, &parsed_text, token, arena);
                ++num_spaces;
                parsed_text.width += space.string_width;
                token.string.data = tokenizer->at;

                if (parsed_text.width - space.string_width > text_area->bounds.width)
                {
                    line_overflowed = true;
                    // NOTE(lucas): Parsing line will usually leave a word and space leftover.
                    // If this is the case, they need to be added at the beginning
                    // of the next line.
                    parsed_text.height += text_area->text.line_height; 
                    word.position.x = text_area->text.position.x;
                    word.position.y += text_area->text.line_height;
                    space.position.x = text_area->text.position.x + word.string_width;
                    space.position.y += text_area->text.line_height;

                    overflow_text->word = word;
                    overflow_text->space = space;

                    // NOTE(lucas): Number of spaces is overcounted by 2: space after final word and overflow space
                    // Subtract back the width of the overflow word and 2 spaces to get the actual line width
                    num_spaces -= 2;
                    parsed_text.width -= (word.string_width + 2.0f*space.string_width);
                    parsing = false;
                }
                else
                {
                    // Push new tokens onto parsed_text and put the string at the tokenizer position
                    parsed_text_push(&parsed_text, &word, arena);
                    parsed_text_push(&parsed_text, &space, arena);
                    token.position.x = text_area->text.position.x + parsed_text.width;
                }

                // NOTE(lucas): End the line if the null terminator or newline is encountered.
                // In the latter case, advance the tokenizer past the newline escapes
                if (final_token)
                    parsing = false;
                else if (tokenizer->at[0] == '\n')
                {
                    s8_iter_move(tokenizer, 1);
                    parsing = false;
                }
                else if (tokenizer->at[0] == '\r' && tokenizer->at[1] == '\n')
                {
                    s8_iter_move(tokenizer, 2);
                    parsing = false;
                }
        }
        else
            s8_iter_move(tokenizer, 1);
    }

    // TODO(lucas): Draw while aligning?
    f32 width_remaining = text_area->bounds.width - parsed_text.width;
    switch(text_area->horiz_alignment)
    {
        case TEXT_ALIGN_HORIZ_JUSTIFIED:
        {
            // Line is only justified if it does not overflow to another line
            if (!line_overflowed)
                break;

            f32 width_per_space = width_remaining / (f32)num_spaces;
            int count = 1;

            // NOTE(lucas): Evenly distribute remaining width to all spaces except any trailing space.
            // For each node, if it is a space, increase the position of the next node (if it exists)
            for (TextNode* node = parsed_text.first_node; node; node = node->next)
            {
                // TODO(lucas): Consider other whitespace
                if ((node->text.string.data[0] == ' ') && node->next)
                {
                    node->next->text.position.x += width_per_space * (f32)count;
                    ++count;
                }
            }
        } break;

        case TEXT_ALIGN_HORIZ_RIGHT:
        {
            // NOTE(lucas): Add all additional space to the leftmost (first) space by
            // shifting each word over by the entire width remaining
            for (TextNode* node = parsed_text.first_node; node; node = node->next)
                node->text.position.x += width_remaining;
        } break;

        case TEXT_ALIGN_HORIZ_CENTER:
        {
            // NOTE(lucas): Divide all additional space between the leftmost (first) and rightmost (last) spaces by
            // shifting each word over by half the width remaining.
            for (TextNode* node = parsed_text.first_node; node; node = node->next)
                node->text.position.x += 0.5f*width_remaining;
        } break;

        // NOTE(lucas): Assume left-align and do nothing.
        default: break;
    }

    return parsed_text;
}
#pragma optimize("", on)

TextArea text_area_init(Renderer* renderer, rect bounds, s8 str, Font* font, u32 text_size_px)
{
    TextArea result = {0};
    result.bounds = bounds;
    v2 text_pos = {bounds.x, result.bounds.y + result.bounds.height - (f32)text_size_px};
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
    i32 lines_req = ceil_f32(text_area->text.string_width / text_area->bounds.width);
    f32 text_height = (f32)(lines_req) * text_area->text.line_height;
    return text_height;
}

internal b32 text_in_bounds(TextArea* text_area)
{
    b32 result = text_area->text.position.y < text_area->bounds.position.y + text_area->bounds.height;
    return result;
}

#pragma optimize("", off)
void draw_text_area(Renderer* renderer, TextArea* text_area)
{
    // NOTE(lucas): Parse and process the text in the text area,
    // then reconstruct one Text object and render it.
    s8_iter tokenizer = {0};
    s8_iter test_tokenizer = {0};
    tokenizer.at = test_tokenizer.at = text_area->text.string.data;
    text_area->text.position = text_area->bounds.position;

    // TODO(lucas): Text is not completely flush with the top of a text area unless only a fraction of the px
    // size is used. It also seems to vary slightly across different fonts.
    // Figure out a way to be more exact about this.

    f32 text_height = 0.0f;
    if (text_area->style & TEXT_AREA_SHRINK_TO_FIT)
    {
        if (text_area->text.px > (u32)text_area->bounds.height)
        {
            f32 delta = (f32)text_area->text.px - text_area->bounds.height;
            u32 new_size = text_area->text.px - (u32)delta;
            text_area->text.position.y += delta;
            text_set_size_px(&text_area->text, new_size);
        }

        // NOTE(lucas): Wrap text and shrink to fit.
        // Wrap text, and if text height exceeds bounds, decrease font size.
        if (text_area->style & TEXT_AREA_WRAP)
        {
            text_height = get_text_height(text_area);
            while (text_height > text_area->bounds.height)
            {
                text_set_size_px(&text_area->text, text_area->text.px-1);
                text_height = get_text_height(text_area);
            }
        }
        else
        {
            // NOTE(lucas): Don't wrap but shrink text to fit.
            text_height = (f32)text_area->text.px;
            f32 text_width = text_get_width(&text_area->text);
            while (text_width > text_area->bounds.width)
            {
                --text_area->text.px_width;
                text_width = text_get_width(&text_area->text);
            }
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
        text_height = text_area->text.line_height;
        text_area->bounds.height = text_area->text.line_height;
    }

    f32 vert_space_remaining = text_area->bounds.height - text_height + text_area->text.line_height;
    switch (text_area->vert_alignment)
    {
        case TEXT_ALIGN_VERT_BOTTOM:
        {
            text_area->text.position.y += vert_space_remaining;
        } break;

        case TEXT_ALIGN_VERT_CENTER:
        {
            text_area->text.position.y += 0.5f*vert_space_remaining;
        } break;

        case TEXT_ALIGN_VERT_TOP:
        {
            text_area->text.position.y += text_area->text.px;
        } break;

        default: break; 
    }

    OverflowText overflow = {0};
    while (tokenizer.at[0])
    {
        ParsedText parsed_text = parse_text(&tokenizer, text_area, &overflow, &renderer->scratch_arena);
        for (TextNode* node = parsed_text.first_node; node; node = node->next)
        {
            node->text.px = text_area->text.px;
            draw_text(renderer, node->text);
        }

        text_area->text.position.y += text_area->text.line_height;

        // NOTE(lucas): This should only be hit if NOT shrink to fit.
        // Discard any text that overflows y bound
        if (!text_in_bounds(text_area))
            break;
    }

    // NOTE(lucas): Draw the overflow text if there is any.
    // Overflow text should only be drawn if the text area is shrink to fit or if there is room for the extra line.
    if (overflow.word.string.data && ((text_area->style & TEXT_AREA_SHRINK_TO_FIT) || text_in_bounds(text_area)))
        draw_text(renderer, overflow.word);
}
#pragma optimize("", on)
 