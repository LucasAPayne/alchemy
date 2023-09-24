#pragma once

#include "util/types.h"

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct Renderer Renderer;

// NOTE(lucas): For now, there will be a font renderer for each different font
typedef struct Font
{
    FT_Face face;
} Font;

typedef struct Text
{
    Font* font;
    u32 px;
    v2 position;
    v4 color;
    const char* string;
} Text;

Font font_load_from_file(const char* filename);

Text text_init(const char* string, Font* font, v2 position, u32 px);
void draw_text(Renderer* renderer, Text text);
