#include "engine/engine.hpp"

namespace engine
{
    void Init(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface, int w, int h)
    {
        Context::Init(extensions, createSurface);
        Context::GetInstance().InitSwapchain(w, h);
    }

    void Quit()
    {
        Context::GetInstance().DestroySwapchain();
        Context::Quit();
    }
}
