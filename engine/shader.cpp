#include "shader.hpp"
#include "context.hpp"
#include <fstream>

namespace engine
{
    std::string ReadWholeFile(const std::string &filePath)
    {
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + filePath);
        }

        size_t fileSize = (size_t)file.tellg();
        std::string buffer(fileSize, ' ');
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    Shader::Shader(const engine::Context *context, const std::string &vertexPath, const std::string &fragmentPath)
    {
        this->context = context;

        std::string vertexSource = ReadWholeFile(vertexPath);
        std::string fragmentSource = ReadWholeFile(fragmentPath);

        vk::ShaderModuleCreateInfo vertexInfo;
        vertexInfo.codeSize = vertexSource.size();
        vertexInfo.pCode = reinterpret_cast<const uint32_t *>(vertexSource.data());

        vertexModule = context->device.createShaderModule(vertexInfo);

        vk::ShaderModuleCreateInfo fragmentInfo;
        fragmentInfo.codeSize = fragmentSource.size();
        fragmentInfo.pCode = reinterpret_cast<const uint32_t *>(fragmentSource.data());

        fragmentModule = context->device.createShaderModule(fragmentInfo);

        initStage();
    }

    Shader::~Shader()
    {
        context->device.destroyShaderModule(vertexModule);
        context->device.destroyShaderModule(fragmentModule);
    }

    void Shader::initStage()
    {
        stage_.resize(2);
        stage_[0].setStage(vk::ShaderStageFlagBits::eVertex).setModule(vertexModule).setPName("main");
        stage_[1].setStage(vk::ShaderStageFlagBits::eFragment).setModule(fragmentModule).setPName("main");
    }

}