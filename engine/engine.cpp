#include "engine.hpp"
#include <string>

namespace engine
{
    void Engine::Init(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface, int width, int height)
    {
        // Create context
        context = std::make_unique<Context>(extensions, createSurface);
        swapchain = std::make_unique<Swapchain>(context.get(), width, height);

        // Create shader
        shader = std::make_unique<Shader>(context.get(), "assets/shaders/shader.vert.spv", "assets/shaders/shader.frag.spv");

        // Create render process
        renderProcess = std::make_unique<RenderProcess>(context.get());
        renderProcess->InitRenderPass(swapchain.get());
        renderProcess->InitLayout();

        // Create swapchain
        swapchain->createFramebuffers(renderProcess.get(), width, height);

        // Create pipeline
        renderProcess->InitPipeline(shader.get(), width, height);

        // Create renderer
        renderer = std::make_unique<Renderer>(context.get(), renderProcess.get(), swapchain.get());
    }

    void Engine::Quit()
    {
        context->device.waitIdle();
        renderer.reset();
        renderProcess.reset();
        shader.reset();
        swapchain.reset();
        context.reset();
    }

    void Engine::Tick()
    {
        renderer->Render();
    }
}
