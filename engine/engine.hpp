#pragma once

#include "vulkan/vulkan.hpp"
#include "context.hpp"

namespace engine
{
    void Init(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface);
    void Quit();
}