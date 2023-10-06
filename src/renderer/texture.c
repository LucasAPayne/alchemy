#include "renderer/texture.h"

#include <glad/glad.h>
#include <stb_image/stb_image.h>

Texture texture_generate()
{
    // Generate texture
    Texture texture = {0};
    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    
    // Texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return texture;
}

void texture_fill_empty_data(int width, int height)
{
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
}

Texture texture_load_from_file(const char* filename)
{
    Texture tex = {0};

    // Load image for texture
    ubyte* tex_data = stbi_load(filename, &tex.size.x, &tex.size.y, &tex.channels, 0);

    if (!tex_data)
    {
        // TODO(lucas): Logging
    }

    // Generate texture
    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);

    // TODO(lucas): Make options configurable
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = 0;
    switch(tex.channels)
    {
        case 1: format = GL_RED;  break;
        case 2: format = GL_RG;   break;
        case 3: format = GL_RGB;  break;
        case 4: format = GL_RGBA; break;
        default: break;
    }

    // TODO(lucas): Internal format is supposed to be like GL_RGBA8
    glTexImage2D(GL_TEXTURE_2D, 0, format, tex.size.x, tex.size.y, 0, format, GL_UNSIGNED_BYTE, tex_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    if (tex_data)
        stbi_image_free(tex_data);

    return tex;
}

void texture_bind_id(u32 id)
{
    // TODO(lucas): Use slots?
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
}

void texture_bind(Texture* tex)
{
    texture_bind_id(tex->id);
}

void texture_unbind(void)
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void texture_delete(Texture* tex)
{
    glDeleteTextures(1, &tex->id);
}
