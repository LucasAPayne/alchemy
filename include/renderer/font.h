#pragma once

#include "util/types.h"

#include <ft2build.h>
#include FT_FREETYPE_H

// NOTE(lucas): For now, there will be a font renderer for each different font
typedef struct FontRenderer
{
    u32 shader;
    u32 vao;
    u32 vbo;
    u32 ibo;
    FT_Face face;
} FontRenderer;

void font_renderer_init(FontRenderer* font_renderer, u32 shader, const char* filename);
void font_renderer_delete(FontRenderer* font_renderer);
void render_text(FontRenderer* font_renderer, const char* text, v2 position, u32 pt, v4 color);
