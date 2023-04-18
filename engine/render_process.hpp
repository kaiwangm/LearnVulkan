#pragma once

#include "vulkan/vulkan.hpp"
#include "shader.hpp"

namespace engine
{
    class Swapchain;

    class RenderProcess final
    {
    public:
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;
        vk::RenderPass renderPass;

        RenderProcess(const engine::Context *context)
        {
            this->context = context;
        }
        ~RenderProcess();

        void InitLayout();
        void InitRenderPass(const Swapchain *swapchain, vk::Format depthFormat);
        void InitPipeline(const Shader *shader, int width, int height);

    private:
        const engine::Context *context;
    };
}