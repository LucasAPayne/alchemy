#pragma once

#include "util/alchemy_math.h"
#include "util/alchemy_memory.h"
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

typedef enum TextAlignmentHoriz
{
    TEXT_ALIGN_HORIZ_LEFT = 0,
    TEXT_ALIGN_HORIZ_RIGHT,
    TEXT_ALIGN_HORIZ_CENTER,
    TEXT_ALIGN_HORIZ_JUSTIFIED
} TextAlignmentHoriz;

typedef enum TextAlignmentVert
{
    TEXT_ALIGN_VERT_TOP = 0,
    TEXT_ALIGN_VERT_BOTTOM,
    TEXT_ALIGN_VERT_CENTER
} TextAlignmentVert;

// TODO(lucas): Background style?
typedef enum TextAreaStyle
{
    TEXT_AREA_WRAP          = (1 << 0),
    TEXT_AREA_SHRINK_TO_FIT = (1 << 1),
} TextAreaStyle;

// TODO(lucas): Margins, borders, and other options
typedef struct TextArea
{
    rect bounds;
    Text text;
    TextAlignmentHoriz horiz_alignment;
    TextAlignmentVert vert_alignment;
    TextAreaStyle style;
} TextArea;

Font font_load_from_file(const char* filename);

Text text_init(char* string, Font* font, v2 position, u32 px);
void text_set_size_px(Text* text, u32 px);
void text_scale(Text* text, f32 factor);

void output_text(Renderer* renderer, Text text);

TextArea text_area_init(rect bounds, char* str, Font* font, u32 text_size_px);
void text_area_scale(TextArea* text_area, f32 factor);

void draw_text_area(Renderer* renderer, TextArea text_area);
