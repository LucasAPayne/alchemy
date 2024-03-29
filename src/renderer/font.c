#include "alchemy/renderer/font.h"
#include "alchemy/renderer/renderer.h"
#include "alchemy/util/math.h"
#include "alchemy/util/memory.h"
#include "alchemy/util/str.h"
#include "alchemy/util/types.h"

#include <glad/glad.h>

// TODO(lucas): Replace with custom methods
#include <string.h> // strncpy

Font font_load_from_file(const char* filename)
{
    Font font = {0};
    FT_Library ft;

    if (FT_Init_FreeType(&ft))
    {
        // TODO(lucas): Diagnostic, could not init freetype lib
    }

    if (FT_New_Face(ft, filename, 0, &font.face))
    {
        // TODO(lucas): Diagnostic, could not open font
    }

    return font;
}

// NOTE(lucas): Determine width of string in pixels
internal f32 text_get_width(Text text)
{
    f32 result = 0.0f;

    FT_Set_Pixel_Sizes(text.font->face, text.px_width, text.px);
    FT_GlyphSlot glyph = text.font->face->glyph;

    for (char* c = text.string; *c; ++c)
    {
        if (!FT_Load_Char(text.font->face, *c, FT_LOAD_NO_BITMAP))
        {
            // TODO(lucas): Diagnostic, could not load character
        }

        // TODO(lucas): Other types of whitespace
        if (*c == ' ')
            result += text.font->face->glyph->advance.x/64;
        else
            result += (f32)glyph->metrics.width/64;
    }

    return result;
}

void text_set_size_px(Text* text, u32 px)
{
    text->px = px;
    FT_Set_Pixel_Sizes(text->font->face, 0, text->px);
    text->px_width = FT_MulFix(text->font->face->units_per_EM, text->font->face->size->metrics.x_scale) / 64;

    text->string_width = text_get_width(*text);
    text->line_height = (f32)text->font->face->size->metrics.height/64;
}

void text_scale(Text* text, f32 factor)
{
    u32 new_size = (u32)((f32)text->px*factor);
    text_set_size_px(text, new_size);
}

Text text_init(Renderer* renderer, char* string, Font* font, v2 position, u32 px)
{
    Text text = {0};

    text.string = string;
    text.font = font;
    text.position = position;
    text.color = color_black();

    text_set_size_px(&text, px);

    return text;
}

void output_text(Renderer* renderer, Text text)
{
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

    for (char* c = text.string; *c; ++c)
    {
        glyph_index = FT_Get_Char_Index(face, *c);

        // When appropriate, retrieve kerning information and advance cursor
        if (use_kerning && previous_glyph_index && glyph_index)
        {
            FT_Vector delta;
            FT_Get_Kerning(face, previous_glyph_index, glyph_index, FT_KERNING_DEFAULT, &delta);
            x += (f32)delta.x/64;
        }

        previous_glyph_index = glyph_index;

        if (!FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER))
        {
            // TODO(lucas): Diagnostic, could not load character
        }

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RED,
                     glyph->bitmap.width,
                     glyph->bitmap.rows,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     glyph->bitmap.buffer);

        f32 x2 = x + glyph->bitmap_left;
        f32 y2 = y + glyph->bitmap_top;
        f32 w = (f32)glyph->bitmap.width;
        f32 h = (f32)glyph->bitmap.rows;

        f32 vertices[] =
        {
            x2 + w, y2    , 1.0f, 0.0f,
            x2 + w, y2 - h, 1.0f, 1.0f,
            x2,     y2 - h, 0.0f, 1.0f,
            x2,     y2    , 0.0f, 0.0f,
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
            y -= text.line_height;
            x = text.position.x;
            ++c;
        }
        else if ((*c == '\n') || (*c == '\r'))
        {
            y -= text.line_height;
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

typedef struct Tokenizer
{
    char* at;
} Tokenizer;

typedef struct ParsedText
{
    f32 width;
    f32 height;
    TextNode* first_node;
} ParsedText;

internal void parsed_text_push(ParsedText* parsed_text, Text* text, MemoryArena* arena)
{
    TextNode* new_node = push_struct(arena, TextNode);
    ASSERT(new_node);
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

internal b32 char_is_whitespace(char c)
{
    b32 result = (c == ' ' ) || (c == '\t') || (c == '\v') || (c == '\f');
    return result;
}

internal b32 text_is_whitespace(Text text)
{
    b32 result = false;

    for (char* c = text.string; *c; ++c)
    {
        if (!char_is_whitespace(*c))
            break;

        result = true;
    }

    return result;
}

// NOTE(lucas): Null-terminate text after len characters
internal void text_chop(Text* text, usize len, MemoryArena* arena)
{
    char* new_str = push_size(arena, len+1);
    ASSERT(new_str);
    strncpy(new_str, text->string, len);
    new_str[len] = '\0';
    text->string = new_str;
}

internal Text tokenizer_process_token(Tokenizer* tokenizer, ParsedText* parsed_text, Text token, MemoryArena* arena)
{
    // TODO(lucas): Custom strncpy
    Text result = token;
    usize word_len = tokenizer->at - token.string;
    text_chop(&result, word_len, arena);

    result.string_width = text_get_width(result);

    return result;
}

internal ParsedText parse_text(Tokenizer* tokenizer, TextArea text_area, OverflowText* overflow_text, MemoryArena* arena)
{
    ParsedText parsed_text = {0};
    Text token = text_area.text;
    token.string = tokenizer->at;

    int num_spaces = 0;
    b32 line_overflowed = false;

    if (overflow_text->word.string)
    {
        parsed_text_push(&parsed_text, &overflow_text->word, arena);
        parsed_text_push(&parsed_text, &overflow_text->space, arena);
        parsed_text.width += overflow_text->word.string_width + overflow_text->space.string_width;
        token.position.x = text_area.text.position.x + parsed_text.width;
        ++num_spaces;
    }

    // NOTE(lucas): Break up into words. For now, keep punctuation with the word as one node.
    b32 parsing = true;
    while (parsing)
    {
        switch(tokenizer->at[0])
        {
            // NOTE(lucas): Separate into words based on whitespace.
            // Also catch null terminator to get the last word in the string.
            case ' ':
            case '\t':
            case '\v':
            case '\f':
            case '\n':
            case '\r':
            case '\0':
            {
                Text word = tokenizer_process_token(tokenizer, &parsed_text, token, arena);
                parsed_text.width += word.string_width;
                token.string = tokenizer->at;

                while (tokenizer->at[0] && char_is_whitespace(tokenizer->at[0]))
                    ++tokenizer->at;

                Text space = tokenizer_process_token(tokenizer, &parsed_text, token, arena);
                ++num_spaces;
                parsed_text.width += space.string_width;
                token.string = tokenizer->at;

                if (parsed_text.width - space.string_width > text_area.bounds.width)
                {
                    line_overflowed = true;
                    // NOTE(lucas): Parsing line will usually leave a word and space leftover.
                    // If this is the case, they need to be added at the beginning
                    // of the next line.
                    parsed_text.height += text_area.text.line_height; 
                    word.position.x = text_area.text.position.x;
                    word.position.y -= text_area.text.line_height;
                    space.position.x = text_area.text.position.x + word.string_width;
                    space.position.y -= text_area.text.line_height;

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
                    token.position.x = text_area.text.position.x + parsed_text.width;
                }

                // NOTE(lucas): End the line if the null terminator or newline is encountered.
                // In the latter case, advance the tokenizer past the newline escapes
                if (tokenizer->at[0] == '\0')
                    parsing = false;
                else if (tokenizer->at[0] == '\n')
                {
                    ++tokenizer->at;
                    parsing = false;
                }
                else if (tokenizer->at[0] == '\r' && tokenizer->at[1] == '\n')
                {
                    tokenizer->at += 2;
                    parsing = false;
                }
            } break;

            default: ++tokenizer->at; break;
        };
    }

    // TODO(lucas): Draw while aligning?
    f32 width_remaining = text_area.bounds.width - parsed_text.width;
    switch(text_area.horiz_alignment)
    {
        case TEXT_ALIGN_HORIZ_JUSTIFIED:
        {
            // Line is only justified if it does not overflow to another line
            if (!line_overflowed)
                break;

            // TODO(lucas): Sometimes, it looks like the amount of space added is slightly underestimated.
            // Probably a precision issue, but could be with fonts.
            // Some fonts also just give completely incorrect results, so look into improving this later.
            f32 width_per_space = width_remaining / (f32)num_spaces;
            int count = 1;

            // NOTE(lucas): Evenly distribute remaining width to all spaces except any trailing space.
            // For each node, if it is a space, increase the position of the next node (if it exists)
            for (TextNode* node = parsed_text.first_node; node; node = node->next)
            {
                // TODO(lucas): Consider other whitespace
                if ((node->text.string[0] == ' ') && node->next)
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

TextArea text_area_init(Renderer* renderer, rect bounds, char* str, Font* font, u32 text_size_px)
{
    TextArea result = {0};
    result.bounds = bounds;
    v2 text_pos = {bounds.x, result.bounds.y + result.bounds.height - (f32)text_size_px};
    result.text = text_init(renderer, str, font, text_pos, text_size_px);
    return result;
}

void text_area_scale(TextArea* text_area, f32 factor)
{
    text_area->bounds.size = v2_scale(text_area->bounds.size, factor);
    text_scale(&text_area->text, factor);
}

// NOTE(lucas): IMPORTANT(lucas): The arena passed to this function should be cleared each frame
void draw_text_area(Renderer* renderer, TextArea text_area)
{
    // NOTE(lucas): Parse and process the text in the text area,
    // then reconstruct one Text object and render it.
    Tokenizer tokenizer = {0};
    Tokenizer test_tokenizer = {0};
    tokenizer.at = test_tokenizer.at = text_area.text.string;
    text_area.text.position = text_area.bounds.position;
    // TODO(lucas): Text is not completely flush with the top of a text area unless only a fraction of the px
    // size is used. It also seems to vary slightly across different fonts.
    // Figure out a way to be more exact about this.
    text_area.text.position.y += text_area.bounds.height - 0.65f*(f32)text_area.text.px;

    f32 text_height = 0.0f;
    if (text_area.style & TEXT_AREA_SHRINK_TO_FIT)
    {
        if (text_area.text.px > (u32)text_area.bounds.height)
        {
            f32 delta = (f32)text_area.text.px - text_area.bounds.height;
            u32 new_size = text_area.text.px - (u32)delta;
            text_area.text.position.y += delta;
            text_set_size_px(&text_area.text, new_size);
        }

        // NOTE(lucas): Wrap text and shrink to fit.
        // Wrap text, and if text height exceeds bounds, decrease font size.
        if (text_area.style & TEXT_AREA_WRAP)
        {
            i32 lines_req = ceil_f32(text_area.text.string_width / text_area.bounds.width) + 1;
            text_height = (f32)(lines_req - 1) * text_area.text.line_height + text_area.text.px;
            while (text_height > text_area.bounds.height)
            {
                text_set_size_px(&text_area.text, text_area.text.px-1);
                // NOTE(lucas): Lines required must be rounded up to be accurate.
                // lines_req = text_area.text.string_width / text_area.bounds.width + 0.5f;
                lines_req = ceil_f32(text_area.text.string_width / text_area.bounds.width) + 1;
                text_height = (f32)(lines_req - 1) * text_area.text.line_height + text_area.text.px;
            }
        }
        else
        {
            // NOTE(lucas): Don't wrap but shrink text to fit.
            text_height = (f32)text_area.text.px;
            f32 text_width = text_get_width(text_area.text);
            while (text_width > text_area.bounds.width)
            {
                --text_area.text.px_width;
                text_width = text_get_width(text_area.text);
            }
        }
    }

    // TODO(lucas): Something weird is going on with calculating the vertical space remaining.
    // For now, the spacing is fudged a little bit to look right, but this needs to be revisited and made more exact.
    f32 vert_space_remaining = text_area.bounds.height - text_height;
    switch (text_area.vert_alignment)
    {
        case TEXT_ALIGN_VERT_BOTTOM:
        {
            text_area.text.position.y -= 1.25f*vert_space_remaining;
        } break;

        case TEXT_ALIGN_VERT_CENTER:
        {
            text_area.text.position.y -= 0.7f*vert_space_remaining;
        } break;

        // NOTE(lucas): Assume top align and do nothing.
        default: break; 
    }

    OverflowText overflow = {0};
    while (tokenizer.at[0])
    {
        ParsedText parsed_text = parse_text(&tokenizer, text_area, &overflow, &renderer->scratch_arena);
        for (TextNode* node = parsed_text.first_node; node; node = node->next)
        {
            node->text.px = text_area.text.px;
            draw_text(renderer, node->text);
        }

        text_area.text.position.y -= text_area.text.line_height;

        // NOTE(lucas): This should only be hit if NOT shrink to fit.
        // Discard any text that overflows y bound
        if (text_area.text.position.y < text_area.bounds.position.y)
            break;
    }
}
 