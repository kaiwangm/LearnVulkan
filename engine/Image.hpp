#pragma once

#include "context.hpp"

#include "stb_image.h"

namespace engine
{
    class Image
    {
    public:
        Image(const std::string &path);
        ~Image();
        uint64_t get_device_size() const { return width * height * 4; }
        stbi_uc *get_pixels() const { return pixels; }
        uint32_t get_width() const { return width; }
        uint32_t get_height() const { return height; }

    private:
        stbi_uc *pixels;
        int width, height, nrChannels;
    };
}