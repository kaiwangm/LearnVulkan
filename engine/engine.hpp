#pragma once

#include "vulkan/vulkan.hpp"
#include "context.hpp"
#include "shader.hpp"

namespace engine
{
    void Init(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface, int w, int h);
    void Quit();
}