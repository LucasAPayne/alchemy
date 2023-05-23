#include "font.h"
#include "shader.h"
#include "texture.h"
#include "types.h"

#include <glad/glad.h>

void init_font_renderer(FontRenderer* font_renderer, u32 shader)
{
    u32 vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    u32 vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(f32), NULL, GL_DYNAMIC_DRAW);

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

void delete_font_renderer(FontRenderer* font_renderer)
{
    glDeleteVertexArrays(1, &font_renderer->vao);
    glDeleteBuffers(1, &font_renderer->vbo);
    glDeleteBuffers(1, &font_renderer->ibo);
    glDeleteProgram(font_renderer->shader);
}

void load_font(FontRenderer* font_renderer, const char* filename, u32 font_size)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        // TODO(lucas): Logging
    }

    FT_Face face;
    if (FT_New_Face(ft, filename, 0, &face))
    {
        // TODO(lucas): Logging
    }

    FT_Set_Pixel_Sizes(face, 0, 48);

    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
        // TODO(lucas): Logging
    }

    // NOTE(lucas): By default, OpenGL requires that textures are aligned on 4-byte boundaries.
    // However, these are grayscale images and only need 1 byte, so disable the restriction
    // (While glyphs are processed as grayscale, color can be added through shaders)
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // NOTE(lucas): Currently, only the first 128 ACII characters are considered.
    for (ubyte c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            // TODO(lucas): Logging
        }

        i32 width = (i32)face->glyph->bitmap.width;
        i32 height = (i32)face->glyph->bitmap.rows;
        u32 texture = generate_font_texture(width, height, face->glyph->bitmap.buffer);

        // Store character
        FontCharacter character = {texture,
            {width, height},
            {face->glyph->bitmap_left, face->glyph->bitmap_top},
            (u32)face->glyph->advance.x};
        font_renderer->characters.insert({c, character});
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void render_text(FontRenderer* font_renderer, const char* text, vec2 position, f32 scale, vec3 color)
{
    shader_set_vec3f(font_renderer->shader, "text_color", color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(font_renderer->vao);

    for (int i = 0; i < strlen(text); i++)
    {
        char c = text[i];
        FontCharacter ch = font_renderer->characters[c];

        // Calculate glyph origin
        // NOTE(lucas): The calculation for y_pos draws quads lower for glyps such as 'p' that go below the baseline
        f32 x_pos = position[0] + ch.bearing[0] * scale;
        f32 y_pos = position[1] + (font_renderer->characters['H'].bearing[1] - ch.bearing[1]) * scale;

        // Calculate glyph size
        f32 width = ch.size[0] * scale;
        f32 height = ch.size[1] * scale;

        // Update vertex buffer for each character
        f32 vertices[] = 
        {
            // pos                         // tex
            x_pos + width, y_pos         , 1.0f, 0.0f, // top right
            x_pos + width, y_pos + height, 1.0f, 1.0f, // bottom right
            x_pos        , y_pos + height, 0.0f, 1.0f, // bottom left
            x_pos        , y_pos         , 0.0f, 0.0f  // top left
        };

        // Render glyph texture over quad
        bind_texture(ch.texture_id, 0);

        // Update VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, font_renderer->vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Advance cursor for next glyph
        // NOTE(lucas): advance is given in 1/64th pixels, so multiply by 2^6 = 64 to get pixels
        position[0] += (ch.advance >> 6) * scale;
    }
    glBindVertexArray(0);
    unbind_texture();
}
