#pragma once
#include <memory>
#include "vulkan/vulkan.hpp"
#include "context.hpp"
#include "shader.hpp"
#include "swapchain.hpp"
#include "render_process.hpp"
#include "renderer.hpp"

namespace engine
{
    class Engine final
    {
    private:
        std::unique_ptr<Context> context;
        std::unique_ptr<Shader> shader;
        std::unique_ptr<Swapchain> swapchain;
        std::unique_ptr<RenderProcess> renderProcess;
        std::unique_ptr<Renderer> renderer;

    public:
        Engine() = default;
        ~Engine() = default;

        void Init(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface, int width, int height);
        void Quit();
        void Tick();
    };
}