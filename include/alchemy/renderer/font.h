#pragma once

#include "alchemy/renderer/texture.h"
#include "alchemy/util/math.h"
#include "alchemy/util/memory.h"
#include "alchemy/util/str.h"
#include "alchemy/util/types.h"

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct Renderer Renderer;
typedef struct RenderCommandText RenderCommandText;

typedef struct
{
    u32 tex_id;  // Texture ID of the glyph
    v2 dim;      // Dimensions of the texture in pixels (based on original font px size)
    v2 bearing;  // Offset, from the baseline, of the leftmost and topmost border of the glyph bitmap
    u32 advance; // The distance, in pixels, to advance the pen after drawing the glyph
} Glyph;

typedef struct Font
{
    // FreeType 2 font data
    FT_Face face;

    // The px size at which the font was loaded, typically at least the largest px size that will be used
    // so that text is only ever scaled down.
    // px size refers to the height of the scaled EM square in pixels.
    u32 px;

    // Whether the font supports kerning (adjusting space between specific pairs of characters))
    b32 has_kerning;

    // Vertical distance from baseline to highest A-Z character coordinate of the font.
    f32 max_top;

    // Array of glyph textures and metrics for the font
    Glyph* glyph_cache;
} Font;

typedef struct Text
{
    Font* font;

    v2 position; // The top-left screen coordinate, in pixels, of the text
    v4 color;

    // px size refers to the height of the scaled EM square in pixels
    f32 px;
    f32 px_original;

    // The ratio of the text's current px size to the px size the at which the font was loaded
    f32 font_scale;

    // Another scale factor on top of font_scale. Defaults to `v2(1.0f, 1.0f)`.
    // Used to stretch and compress text in the x or y direction.
    // Commonly used to compress text to shrink to fit text area bounds.
    v2 scale;

    f32 string_width; // The total width, in pixels, of the string (not affected by line breaks)
    f32 line_height;  // The distance, in pixels, from one baseline to another

    f32 extra_width_per_space; // Extra width to apply to each space to horizontally justify text

    s8 string;
} Text;

// Text horizontal alignment
typedef enum TextAlignH
{
    // Text is horizontally aligned to the left border of the text area bounds.
    // All leftover space goes to the right side.
    TEXT_ALIGN_H_LEFT = 0,

    // Text is horizontally aligned to the right border of the text area bounds.
    // All leftover space goes to the left side.
    TEXT_ALIGN_H_RIGHT,

    // Text is horizontally aligned to the center of the text area bounds.
    // Leftover space is evenly divided between the left and right sides.
    TEXT_ALIGN_H_CENTER,

    // Text is horizontally aligned to both the left and right borders of the text area bounds.
    // Leftover space is evenly distributed between each word.
    // If text wraps, the last line is not justified.
    TEXT_ALIGN_H_JUSTIFIED,

    // Same as justified, but the last line of wrapping text is justified.
    TEXT_ALIGN_H_FLUSH,
} TextAlignH;

// Text vertical alignment
typedef enum TextAlignV
{
    // Text is vertically aligned to the top border of the text area bounds.
    // All leftover space goes to the bottom.
    TEXT_ALIGN_V_TOP = 0,

    // Text is vertically aligned to the bottom border of the text area bounds.
    // All leftover space goes to the top.
    TEXT_ALIGN_V_BOTTOM,

    // Text is vertically aligned to the center of the text area bounds.
    // Leftover space is evenly divided between the bottom and top.
    TEXT_ALIGN_V_CENTER,

    // Text is vertically aligned to both the top and bottom of the text area bounds.
    // Leftover space is evenly distributed between each word.
    TEXT_ALIGN_V_DISTRIBUTED,
} TextAlignV;

// TODO(lucas): Background style?
typedef enum TextAreaStyle
{
    // If text does not fit in the text area's bounds in one line, wrap remaining text into additional lines.
    TEXT_AREA_WRAP          = (1 << 0),

    // Shrink text to fit within the text bounds if it does not.
    // If the text should **not** wrap, squeeze the text horizontally to fit.
    // If the text **should** wrap, but still overflows the bounds, find the best font size to fit the bounds
    // as closely as possible.
    TEXT_AREA_SHRINK_TO_FIT = (1 << 1),
} TextAreaStyle;

// TODO(lucas): Margins, borders, and other options
typedef struct TextArea
{
    TextAlignH horiz_alignment;
    TextAlignV vert_alignment;
    TextAreaStyle style;
    rect bounds;
    Text text;

    // Scales the spacing between lines for wrapping text (default 1.0)
    f32 line_spacing;

    // Used for vertically distributed alignment to evenly distribute leftover space among each line
    f32 extra_height_per_line;
} TextArea;

/* Load a font from a file. Glyph data is cached in a memory arena for the font size specified on load.
 * When drawing text at different font sizes, these glyphs are scaled, so it is recommended to load a font
 * at the maximum font size that will be used, or greater, to avoid scaling up and appearing blurry.
 */
Font font_load_from_file(const char* filename, u32 px, MemoryArena* arena);

Text text_init(s8 string, Font* font, v2 position, f32 px);

// Set the px size of text and recalculate sizing values.
void text_set_size_px(Text* text, f32 px);

// Scale the px size of text based on a scaling factor, and recalculate sizing values.
void text_scale(Text* text, f32 factor);

// Get the pixel width of text (not affected by line breaks).
f32 text_get_width(Text* text);

// Draw text to the screen (internal engine function)
void output_text(Renderer* renderer, RenderCommandText* cmd);

// Initialize a TextArea. A TextArea is like text, but includes bounds and more styling options.
TextArea text_area_init(rect bounds, s8 str, Font* font, f32 text_size_px);

// Scale a text area and its text by a scaling factor, and recalculate sizing values of the text.
void text_area_scale(TextArea* text_area, f32 factor);

// Submit a text area to the renderer to be drawn.
void draw_text_area(Renderer* renderer, TextArea* text_area);
