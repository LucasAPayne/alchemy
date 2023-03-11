#pragma once

#include "types.h"

// NOTE(lucas): #pragma statements silence warnings from FreeType
#pragma warning(push, 0)
#include <ft2build.h>
#include FT_FREETYPE_H
#pragma warning(pop)

// TODO(lucas): TEMPORARY
#include <map>

struct FontCharacter
{
    u32 texture_id;
    glm::ivec2 size;
    glm::ivec2 bearing;
    u32 advance;
};

// NOTE(lucas): For now, there will be a font renderer for each different font
struct FontRenderer
{
    u32 shader;
    u32 vao;
    u32 vbo;
    u32 ibo;
    // TODO(lucas): Replace std::map with custom lookup tabe/BST to get rid of STL dependency
    // It may also be sufficient (or better) to use a simple array, and index using character codes
    std::map<char, FontCharacter> characters;
};

void init_font_renderer(FontRenderer* renderer, u32 shader);
void delete_font_renderer(FontRenderer* renderer);

void load_font(FontRenderer* font_renderer, const char* filename, u32 font_size);
void render_text(FontRenderer* font_renderer, const char* text, glm::vec2 position, f32 scale, glm::vec3 color);
