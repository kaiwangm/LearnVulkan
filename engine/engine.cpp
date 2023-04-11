#include "engine/engine.hpp"

namespace engine
{
    void Init(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface)
    {
        Context::Init(extensions, createSurface);
    }

    void Quit()
    {
        Context::Quit();
    }
}
