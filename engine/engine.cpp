#include "engine/engine.hpp"
#include <string>

namespace engine
{
    Shader* shader = nullptr;
    void Init(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface, int w, int h)
    {
        Context::Init(extensions, createSurface);
        Context::GetInstance().InitSwapchain(w, h);
        shader = new Shader(std::string("shaders/shader.vert.spv"), std::string("shaders/shader.frag.spv"));
    }

    void Quit()
    {
        delete shader;
        Context::GetInstance().DestroySwapchain();
        Context::Quit();
    }
}
