#include "engine/engine.hpp"
#include <string>

namespace engine
{
    Shader* shader = nullptr;
    void Init(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface, int width, int height)
    {
        Context::Init(extensions, createSurface);
        Context::GetInstance().InitSwapchain(width, height);
        shader = new Shader(std::string("shaders/shader.vert.spv"), std::string("shaders/shader.frag.spv"));
        Context::GetInstance().renderProcess->InitRenderPass();
        Context::GetInstance().renderProcess->InitLayout();
        Context::GetInstance().swapchain->createFramebuffers(width, height);
        Context::GetInstance().renderProcess->InitPipeline(*shader, width, height);
    }

    void Quit()
    {
        Context::GetInstance().renderProcess.reset();
        delete shader;
        Context::GetInstance().DestroySwapchain();
        Context::Quit();
    }
}
