#include "alchemy/renderer/texture.h"
#include "alchemy/renderer/renderer.h"

#include <glad/glad.h>
#include <stb_image/stb_image.h>

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

Texture texture_load_from_file(Renderer* renderer, const char* filename)
{
    Texture tex = {0};

    tex.id = renderer_next_tex_id(renderer);

    // Load image for texture
    int size_x, size_y;
    tex.data = stbi_load(filename, &size_x, &size_y, &tex.channels, 0);
    tex.size = (v2){(f32)size_x, (f32)size_y};

    ASSERT(tex.data);

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
