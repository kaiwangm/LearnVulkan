#pragma once

#include "vulkan/vulkan.hpp"
#include "shader.hpp"

namespace engine
{
    class RenderProcess final
    {
    public:
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;
        vk::RenderPass renderPass;

        ~RenderProcess();

        void InitLayout();
        void InitRenderPass();
        void InitPipeline(const Shader& shader, int width, int height);
    };
}