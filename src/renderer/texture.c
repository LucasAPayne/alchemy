#include "alchemy/renderer/texture.h"
#include "alchemy/renderer/renderer.h"
#include "alchemy/util/file.h"
#include "alchemy/util/intrin.h"
#include "alchemy/util/log.h"

#include <glad/glad.h>
#include <stb_image/stb_image.h>

// TODO(lucas): Full bitmap support should separate the BMP header from the DIB header
// and allow using different versions of the DIB header.
#pragma pack(push, 1)
typedef struct BitmapHeader
{
    // BMP Header
    u16 file_type;
    u32 file_size;
    u16 reserved1;
    u16 reserved2;
    u32 pixel_array_offset;

    // DIB Header
    u32 size;
    i32 width;
    i32 height;
    u16 planes;
    u16 bits_per_pixel;
    u32 compression;
    u32 bitmap_size;
    i32 horiz_resolution;
    i32 vert_resolution;
    u32 colors_used;
    u32 colors_important;
    /*
     * NOTE(lucas): When compression is set to 3, it indicates the use of bitfield encoding.
     * In this case, there are masks for the RGB channels, indicating their location.
     * The masks and locations of these channels may be different in any given bitmap.
     */
    u32 red_mask;
    u32 green_mask;
    u32 blue_mask;
} BitmapHeader;
#pragma pack(pop)

internal Texture load_bmp_from_file(void* file, size file_size, Renderer* renderer, MemoryArena* arena)
{
    u8* file_contents = push_size(arena, file_size);
    file_read(file, file_contents, file_size);

    Texture tex = {0};
    if (file_size)
    {
        BitmapHeader* header = (BitmapHeader*)file_contents;

        u32 bytes_per_pixel = header->bits_per_pixel / 8;

        tex.id = renderer_next_tex_id(renderer);
        tex.size.x = (f32)header->width;
        tex.size.y = (f32)header->height;
        tex.channels = bytes_per_pixel;

        switch(header->compression)
        {
            case 0: // BI_RGB: no compression
            {
                // BGR -> RGB
                u8* pixels = file_contents + header->pixel_array_offset;
                tex.data = pixels;
                u32 row_size = ((header->width * bytes_per_pixel + 3) & ~3); // Each row is padded to multiple of 4 bytes
                for (i32 y = 0; y < header->height; ++y)
                {
                    u8* row = pixels + y * row_size; // BMP is bottom-up
                    for (i32 x = 0; x < header->width; ++x)
                    {
                        u8* pixel = row + x * bytes_per_pixel;

                        u8 temp = pixel[0];
                        pixel[0] = pixel[2];
                        pixel[2] = temp;
                    }
                }
            } break;

            case 3: // BI_BITFIELDS
            {
                u32* pixels = (u32*)(file_contents + header->pixel_array_offset);
                tex.data = (u8*)pixels;

                u32 red_mask = header->red_mask;
                u32 green_mask = header->green_mask;
                u32 blue_mask = header->blue_mask;
                u32 alpha_mask = ~(red_mask | green_mask | blue_mask);

                BitScanResult red_scan   = find_least_significant_bit(red_mask);
                BitScanResult green_scan = find_least_significant_bit(green_mask);
                BitScanResult blue_scan  = find_least_significant_bit(blue_mask);
                BitScanResult alpha_scan = find_least_significant_bit(alpha_mask);

                ASSERT(red_scan.found, "Scan for red mask failed");
                ASSERT(green_scan.found, "Scan for green mask failed");
                ASSERT(blue_scan.found, "Scan for blue mask failed");
                ASSERT(alpha_scan.found, "Scan for alpha mask failed");

                i32 red_shift_down   = (i32)red_scan.index;
                i32 green_shift_down = (i32)green_scan.index;
                i32 blue_shift_down  = (i32)blue_scan.index;
                i32 alpha_shift_down = (i32)alpha_scan.index;

                u32* pixel = pixels;
                for (i32 y = 0; y < header->height; ++y)
                {
                    for (i32 x = 0; x < header->width; ++x)
                    {
                        u32 c = *pixel;

                        v4 texel = {(f32)((c & red_mask)   >> red_shift_down),
                            (f32)((c & green_mask) >> green_shift_down),
                            (f32)((c & blue_mask)  >> blue_shift_down),
                            (f32)((c & alpha_mask) >> alpha_shift_down)};
                        texel = srgb255_to_linear1(texel);

                        // BGR -> RGB
                        v3 temp = v3_scale(v3(texel.b, texel.g, texel.r), texel.a);
                        texel.r = temp.r;
                        texel.g = temp.g;
                        texel.b = temp.b;

                        texel = linear1_to_srgb255(texel);
                        *pixel++ = ((u32)(texel.a + 0.5f) << 24) |
                                   ((u32)(texel.r + 0.5f) << 16) |
                                   ((u32)(texel.g + 0.5f) << 8)  |
                                   ((u32)(texel.b + 0.5f) << 0);
                    }
                }
            } break;

            default: log_error("Unsupported/invalid compression type (%u)", header->compression); break;
        }
    }

    ASSERT(tex.data, "Failed to load texture");
    return tex;
}

internal Texture load_any_texture_from_file(char* filename, Renderer* renderer)
{
    stbi_set_flip_vertically_on_load(true);

    Texture tex = {0};
    tex.id = renderer_next_tex_id(renderer);

    // Load image for texture
    int size_x, size_y;
    tex.data = stbi_load(filename, &size_x, &size_y, &tex.channels, 0);
    tex.size = v2((f32)size_x, (f32)size_y);

    return tex;
}

Texture texture_generate(int samples)
{
    GLenum target = (samples > 0) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

    // Generate texture
    Texture texture = {0};
    glGenTextures(1, &texture.id);
    glBindTexture(target, texture.id);

    // Texture options
    if (samples <= 0)
    {
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    return texture;
}

void texture_fill_empty_data(Texture* texture, int width, int height, int samples)
{
    texture_bind(texture, samples);

    // TODO(lucas): Using irregular sampling causes the framebuffer to be incomplete
    if (samples > 0)
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
    else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    texture_unbind(samples);
}

Texture texture_load_from_file(const char* filename, Renderer* renderer, MemoryArena* arena)
{
    size file_size = file_get_size(filename);
    void* file = file_open(filename, FileMode_Read);
    u16 signature = 0;
    file_read(file, &signature, sizeof(signature));
    b32 is_bmp = (signature == 0x4D42);

    Texture tex = {0};
    if (is_bmp)
    {
        file_seek(file, 0, FileSeek_Begin);
        tex = load_bmp_from_file(file, file_size, renderer, arena);
        file_close(file);
    }
    else
    {
        file_close(file);
        tex = load_any_texture_from_file(filename, renderer);
    }
    ASSERT(tex.data, "Failed to load texture");

    renderer_push_texture(renderer, tex);
    return tex;
}

Texture texture_load_from_memory(Renderer* renderer, int width, int height, int channels, ubyte* memory)
{
    Texture tex = {0};
    tex.id = renderer_next_tex_id(renderer);
    tex.data = memory;
    tex.size = v2((f32)width, (f32)height);
    tex.channels = channels;

    renderer_push_texture(renderer, tex);
    return tex;
}

void texture_bind_id(u32 id, int samples)
{
    GLenum target = (samples > 0) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    // TODO(lucas): Use slots?
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(target, id);
}

void texture_bind(Texture* tex, int samples)
{
    texture_bind_id(tex->id, samples);
}

void texture_unbind(int samples)
{
    GLenum target = (samples > 0) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    glBindTexture(target, 0);
}

void texture_delete(Texture* tex)
{
    glDeleteTextures(1, &tex->id);
    if (tex->data)
        stbi_image_free(tex->data);
}
