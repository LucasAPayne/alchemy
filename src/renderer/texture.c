#include "renderer/texture.h"

#include <glad/glad.h>

#include <stb_image/stb_image.h>

u32 generate_texture_from_file(const char* filename)
{
    // Load image for texture
    int tex_width;
    int tex_height;
    int tex_channels;
    ubyte* tex_data = stbi_load(filename, &tex_width, &tex_height, &tex_channels, 0);

    if (!tex_data)
    {
        // TODO(lucas): Logging
    }

    // Generate texture
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // TODO(lucas): Make options configurable
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = 0;
    switch(tex_channels)
    {
        case 1: format = GL_RED;  break;
        case 2: format = GL_RG;   break;
        case 3: format = GL_RGB;  break;
        case 4: format = GL_RGBA; break;
        default: break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, tex_width, tex_height, 0, format, GL_UNSIGNED_BYTE, tex_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    if (tex_data)
        stbi_image_free(tex_data);

    return tex;
}

u32 generate_texture()
{
    // Generate texture
    u32 texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}

void texture_bind(u32 texture, u32 slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture);
}

void texture_unbind(void)
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void texture_delete(u32 texture)
{
    glDeleteTextures(1, &texture);
}
