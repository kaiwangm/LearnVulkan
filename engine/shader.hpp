#pragma once

#include "vulkan/vulkan.hpp"

namespace engine
{
    class Shader
    {
    public:
        Shader(const std::string &vertexPath, const std::string &fragmentPath);
        ~Shader();

        const vk::ShaderModule &getVertexModule() const
        {
            return vertexModule;
        }

        const vk::ShaderModule &getFragmentModule() const
        {
            return fragmentModule;
        }

        std::vector<vk::PipelineShaderStageCreateInfo> GetStage() const
        {
            return stage_;
        }

    private:
        vk::ShaderModule vertexModule;
        vk::ShaderModule fragmentModule;
        std::vector<vk::PipelineShaderStageCreateInfo> stage_;

        void initStage();
    };
}