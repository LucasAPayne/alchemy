#include "renderer/font.h"
#include "renderer/renderer.h"
#include "util/alchemy_math.h"
#include "util/types.h"

#include <glad/glad.h>

// TODO(lucas): Replace with custom methods
#include <string.h> // strncpy
#include <stdlib.h> // malloc

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

    FT_Set_Pixel_Sizes(text.font->face, 0, text.px);
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

Text text_init(char* string, Font* font, v2 position, u32 px)
{
    Text text = {0};

    text.string = string;
    text.font = font;
    text.position = position;
    text.px = px;
    text.color = (v4){1.0f, 1.0f, 1.0f, 1.0f};

    text.string_width = text_get_width(text);
    text.line_height = (f32)font->face->size->metrics.height/64;

    return text;
}

void draw_text(Renderer* renderer, Text text)
{
    // Set font size in pixels
    FT_Set_Pixel_Sizes(text.font->face, 0, text.px);
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
    TextNode* first_node;
} ParsedText;

internal void parsed_text_push(ParsedText* parsed_text, Text* text)
{
    TextNode* new_node = malloc(sizeof(TextNode));
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

internal void parsed_text_clear(ParsedText* parsed_text)
{
    TextNode* current = parsed_text->first_node;
    TextNode* next = 0;

    while (current)
    {
        next = current->next;
        free(current);
        current = next;
    }

    parsed_text->first_node = 0;
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
internal void text_chop(Text* text, usize len)
{
    char* new_str = malloc(len);
    ASSERT(new_str);
    strncpy(new_str, text->string, len);
    new_str[len] = '\0';
    text->string = new_str;
}

internal Text tokenizer_process_token(Tokenizer* tokenizer, ParsedText* parsed_text, Text token)
{
    // TODO(lucas): Memory arena and prevent memory leak
    // TODO(lucas): Custom strncpy
    Text result = token;
    usize word_len = tokenizer->at - token.string;
    text_chop(&result, word_len);

    result.string_width = text_get_width(result);

    return result;
}

internal ParsedText parse_text(Tokenizer* tokenizer, TextArea text_area, OverflowText* overflow_text)
{
    ParsedText parsed_text = {0};
    Text token = text_area.text;
    token.string = tokenizer->at;

    f32 line_width = 0.0f;
    int num_spaces = 0;
    b32 line_overflowed = false;

    if (overflow_text->word.string)
    {
        parsed_text_push(&parsed_text, &overflow_text->word);
        parsed_text_push(&parsed_text, &overflow_text->space);
        f32 width = overflow_text->word.string_width + overflow_text->space.string_width;
        line_width += width;
        token.position.x = text_area.text.position.x + line_width;
        ++num_spaces;
    }

    // NOTE(lucas): Break up into words. For now, keep punctuation with the word as one node.
    b32 parsing = true;
    while (parsing)
    {
        switch(tokenizer->at[0])
        {
            // TODO(lucas): Start a new line and adjust cursor when \n or \r\n is encountered.
            // A line containing a newline should not be justified.

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
                Text word = tokenizer_process_token(tokenizer, &parsed_text, token);
                line_width += word.string_width;
                token.string = tokenizer->at;

                while (tokenizer->at[0] && char_is_whitespace(tokenizer->at[0]))
                    ++tokenizer->at;

                Text space = tokenizer_process_token(tokenizer, &parsed_text, token);
                ++num_spaces;
                line_width += space.string_width;
                token.string = tokenizer->at;

                if (line_width - space.string_width > text_area.bounds.width)
                {
                    line_overflowed = true;
                    // NOTE(lucas): Parsing line will usually leave a word and space leftover.
                    // If this is the case, they need to be added at the beginning
                    // of the next line.
                    word.position.x = text_area.text.position.x;
                    word.position.y -= text_area.text.line_height;
                    space.position.x = text_area.text.position.x + word.string_width;
                    space.position.y -= text_area.text.line_height;

                    overflow_text->word = word;
                    overflow_text->space = space;

                    // NOTE(lucas): Number of spaces is overcounted by 2: space after final word and overflow space
                    // Subtract back the width of the overflow word and 2 spaces to get the actual line width
                    num_spaces -= 2;
                    line_width -= (word.string_width + 2.0f*space.string_width);
                    parsing = false;
                }
                else
                {
                    // Push new tokens onto parsed_text and put the string at the tokenizer position
                    parsed_text_push(&parsed_text, &word);
                    parsed_text_push(&parsed_text, &space);
                    token.position.x = text_area.text.position.x + line_width;
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
    f32 width_remaining = text_area.bounds.width - line_width;
    switch(text_area.alignment)
    {
        case TEXT_ALIGN_JUSTIFIED:
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
                if ((node->text.string[0] == ' ') && node->next)
                {
                    node->next->text.position.x += width_per_space * (f32)count;
                    ++count;
                }
            }
        } break;

        case TEXT_ALIGN_RIGHT:
        {
            // NOTE(lucas): Add all additional space to the leftmost (first) space by
            // shifting each word over by the entire width remaining
            for (TextNode* node = parsed_text.first_node; node; node = node->next)
                node->text.position.x += width_remaining;
        } break;

        case TEXT_ALIGN_CENTER:
        {
            // NOTE(lucas): Divide all additional space between the leftmost (first) and rightmost (last) spaces by
            // shifting each word over by half the width remaining.
            for (TextNode* node = parsed_text.first_node; node; node = node->next)
                node->text.position.x += 0.5f*width_remaining;
        } break;
    }

    return parsed_text;
}

TextArea text_area_init(rect bounds, Text text)
{
    TextArea result = {0};
    result.bounds = bounds;
    result.text = text;
    return result;
}

void draw_text_area(Renderer* renderer, TextArea text_area)
{
    // NOTE(lucas): Parse and process the text in the text area,
    // then reconstruct one Text object and render it.
    Tokenizer tokenizer = {0};
    tokenizer.at = text_area.text.string;
    
    OverflowText overflow = {0};
    while (tokenizer.at[0])
    {
        ParsedText parsed_text = parse_text(&tokenizer, text_area, &overflow);
        for (TextNode* node = parsed_text.first_node; node; node = node->next)
            draw_text(renderer, node->text);

        text_area.text.position.y -= text_area.text.line_height;

        parsed_text_clear(&parsed_text);
    }
}
 