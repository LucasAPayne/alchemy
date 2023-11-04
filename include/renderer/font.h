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

    v2 position;
    v4 color;

    u32 px;
    u32 px_width;
    f32 string_width;
    f32 line_height;

    char* string;
} Text;

typedef enum TextAlignment
{
    TEXT_ALIGN_LEFT = 0,
    TEXT_ALIGN_RIGHT,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_JUSTIFIED
} TextAlignment;

// TODO(lucas): Background style?
typedef enum TextAreaStyle
{
    TEXT_AREA_WRAP          = (1 << 0),
    TEXT_AREA_SHRINK_TO_FIT = (1 << 1),
} TextAreaStyle;

typedef struct TextArea
{
    rect bounds;
    Text text;
    TextAlignment alignment;
    TextAreaStyle style;
} TextArea;

Font font_load_from_file(const char* filename);

Text text_init(char* string, Font* font, v2 position, u32 px);

void draw_text(Renderer* renderer, Text text);

TextArea text_area_init(rect bounds, Text text);
void draw_text_area(Renderer* renderer, TextArea text_area);
