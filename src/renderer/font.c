#include "renderer/font.h"
#include "renderer/shader.h"
#include "renderer/texture.h"
#include "util/types.h"

#include <glad/glad.h>

void font_renderer_init(FontRenderer* font_renderer, u32 shader, const char* filename)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        // TODO(lucas): Diagnostic, could not init freetype lib
    }

    if (FT_New_Face(ft, filename, 0, &font_renderer->face))
    {
        // TODO(lucas): Diagnostic, could not open font
    }

    u32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    u32 vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // NOTE(lucas): While vertex buffer data changes a lot, the order in which indices are drawn
    // does not. So, indices and index buffer can be defined here.
    u32 indices[] = 
    {
      0, 1, 3,
      1, 2, 3  
    };

    u32 ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    // TODO(lucas): Usage is GL_STATIC_DRAW for now, while vbo uses GL_DYNAMIC_DRAW.
    // Is this acceptable since the ibo will not change, while the vbo will frequently?
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (void*)(2*sizeof(f32)));

    glBindVertexArray(0);

    font_renderer->shader = shader;
    font_renderer->vao = vao;
    font_renderer->vbo = vbo;
    font_renderer->ibo = ibo;
}

void font_renderer_delete(FontRenderer* font_renderer)
{
    glDeleteVertexArrays(1, &font_renderer->vao);
    glDeleteBuffers(1, &font_renderer->vbo);
    glDeleteBuffers(1, &font_renderer->ibo);
    shader_delete(font_renderer->shader);
}

void text_draw(FontRenderer* font_renderer, const char* text, v2 position, u32 pt, v4 color)
{
    // Set font size in pt
    FT_Set_Pixel_Sizes(font_renderer->face, 0, pt);
    FT_GlyphSlot glyph = font_renderer->face->glyph;

    shader_set_v4(font_renderer->shader, "text_color", color);
    glBindVertexArray(font_renderer->vao);

    // NOTE(lucas): By default, OpenGL requires that textures are aligned on 4-byte boundaries,
    // but we need 1-byte alignment for grayscale glyph bitmaps
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    Texture texture = texture_generate();

    const char *c;
    for (c = text; *c; ++c)
    {
        if (!FT_Load_Char(font_renderer->face, *c, FT_LOAD_RENDER))
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

        f32 x2 =  position.x + glyph->bitmap_left;
        f32 y2 = position.y - glyph->bitmap_top;
        f32 w = (f32)glyph->bitmap.width;
        f32 h = (f32)glyph->bitmap.rows;
    
        f32 vertices[] = {
            x2 + w, y2    , 1.0f, 0.0f,
            x2 + w, y2 + h, 1.0f, 1.0f,
            x2,     y2 + h, 0.0f, 1.0f,
            x2,     y2    , 0.0f, 0.0f,
        };

        // Update VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, font_renderer->vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Advance cursor for next glyph
        position.x += glyph->advance.x/64;
        position.y += glyph->advance.y/64;
    }

    glBindVertexArray(0);
    texture_unbind();
    texture_delete(&texture);
}
