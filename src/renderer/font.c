#include "renderer/font.h"
#include "renderer/renderer.h"
#include "util/types.h"

#include <glad/glad.h>

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

Text text_init(const char* string, Font* font, v2 position, u32 px)
{
    Text text = {0};

    text.string = string;
    text.font = font;
    text.position = position;
    text.px = px;
    text.color = (v4){1.0f, 1.0f, 1.0f, 1.0f};

    return text;
}

void draw_text(Renderer* renderer, Text text)
{
    // Set font size in pixels
    FT_Set_Pixel_Sizes(text.font->face, 0, text.px);
    FT_GlyphSlot glyph = text.font->face->glyph;

    shader_set_v4(renderer->font_renderer.shader, "text_color", text.color);
    glBindVertexArray(renderer->font_renderer.vao);

    // NOTE(lucas): By default, OpenGL requires that textures are aligned on 4-byte boundaries,
    // but we need 1-byte alignment for grayscale glyph bitmaps
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    Texture texture = texture_generate();

    const char* c;
    for (c = text.string; *c; ++c)
    {
        if (!FT_Load_Char(text.font->face, *c, FT_LOAD_RENDER))
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

        f32 x2 = text.position.x + glyph->bitmap_left;
        f32 y2 = text.position.y + glyph->bitmap_top;
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
        text.position.x += glyph->advance.x/64;
        text.position.y += glyph->advance.y/64;
    }

    glBindVertexArray(0);
    texture_unbind();
    texture_delete(&texture);
}
