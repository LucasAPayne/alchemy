#include <alchemy/util/log.h>
#define STBIW_ASSERT(x) ASSERT(x, "stbi_image assertion failed")
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>
