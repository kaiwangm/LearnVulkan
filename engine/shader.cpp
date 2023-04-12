#include "shader.hpp"
#include "context.hpp"
#include <fstream>

namespace engine
{
    std::string ReadWholeFile(const std::string& filePath)
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

    Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
    {
        std::string vertexSource = ReadWholeFile(vertexPath);
        std::string fragmentSource = ReadWholeFile(fragmentPath);

        vk::ShaderModuleCreateInfo vertexInfo;
        vertexInfo.codeSize = vertexSource.size();
        vertexInfo.pCode = reinterpret_cast<const uint32_t*>(vertexSource.data());

        vertexModule = Context::GetInstance().device.createShaderModule(vertexInfo);

        vk::ShaderModuleCreateInfo fragmentInfo;
        fragmentInfo.codeSize = fragmentSource.size();
        fragmentInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentSource.data());

        fragmentModule = Context::GetInstance().device.createShaderModule(fragmentInfo);
    }

    Shader::~Shader()
    {
        auto& device = Context::GetInstance().device;
        device.destroyShaderModule(vertexModule);
        device.destroyShaderModule(fragmentModule);
    }

}