#pragma once

#include "util/alchemy_math.h"
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
    char* string;
    f32 string_width;
} Text;

typedef struct TextArea
{
    // TODO(lucas): Properties/flags like alignment
    rect bounds;
    Text text;
} TextArea;

Font font_load_from_file(const char* filename);

Text text_init(char* string, Font* font, v2 position, u32 px);

void draw_text(Renderer* renderer, Text text);

TextArea text_area_init(rect bounds, Text text);
void draw_text_area(Renderer* renderer, TextArea text_area);
