#include "Image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace engine
{
    Image::Image(const std::string &path)
    {
        pixels = stbi_load(path.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);

        if (!pixels)
        {
            throw std::runtime_error("Failed to load image: " + path);
        }
    }

    Image::~Image()
    {
        stbi_image_free(pixels);
    }
}