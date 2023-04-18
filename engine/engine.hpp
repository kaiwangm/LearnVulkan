#pragma once
#include <memory>
#include <chrono>
#include "vulkan/vulkan.hpp"
#include "context.hpp"
#include "shader.hpp"
#include "swapchain.hpp"
#include "render_process.hpp"
#include "renderer.hpp"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "StaticMesh.hpp"
#include "Image.hpp"

namespace engine
{
    class Engine final
    {
    public:
        std::unique_ptr<Context> context;
        std::unique_ptr<Shader> shader;
        std::unique_ptr<Swapchain> swapchain;
        std::unique_ptr<RenderProcess> renderProcess;
        std::unique_ptr<Renderer> renderer;

        SDL_Window *window;
        ImGui_ImplVulkanH_Window g_MainWindowData;
        bool g_SwapChainRebuild = false;

        void SetupVulkanWindow(ImGui_ImplVulkanH_Window *wd, VkSurfaceKHR surface, int width, int height);
        void CleanupVulkanWindow();
        void InitImGui(SDL_Window *window, int width, int height);
        void RenderGui(bool &shouldClose);
        void FrameRender(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data);
        void FramePresent(ImGui_ImplVulkanH_Window *wd);

        // find memory type
        auto findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
        {
            vk::PhysicalDeviceMemoryProperties memProperties;
            context->phyDevice.getMemoryProperties(&memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
            {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
                {
                    return i;
                }
            }

            throw std::runtime_error("Failed to find suitable memory type");
        };

        void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer &buffer, vk::DeviceMemory &bufferMemory)
        {
            vk::BufferCreateInfo bufferInfo = {};
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = vk::SharingMode::eExclusive;

            if (context->device.createBuffer(&bufferInfo, nullptr, &buffer) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to create buffer");
            }

            vk::MemoryRequirements memRequirements;
            context->device.getBufferMemoryRequirements(buffer, &memRequirements);

            vk::MemoryAllocateInfo allocInfo = {};
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

            if (context->device.allocateMemory(&allocInfo, nullptr, &bufferMemory) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to allocate buffer memory");
            }

            context->device.bindBufferMemory(buffer, bufferMemory, 0);
        }

        void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size){
            vk::CommandPool commandPool;
            vk::CommandPoolCreateInfo poolInfo = {};
            poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;

            if (context->device.createCommandPool(&poolInfo, nullptr, &commandPool) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to create command pool");
            }

            vk::CommandBufferAllocateInfo allocInfo = {};
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;

            vk::CommandBuffer commandBuffer;
            if(context->device.allocateCommandBuffers(&allocInfo, &commandBuffer) != vk::Result::eSuccess){
                throw std::runtime_error("Failed to allocate command buffer");
            }

            vk::CommandBufferBeginInfo beginInfo = {};
            beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

            if(commandBuffer.begin(&beginInfo) != vk::Result::eSuccess){
                throw std::runtime_error("Failed to begin recording command buffer");
            }

            vk::BufferCopy copyRegion = {};
            copyRegion.size = size;
            commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

            commandBuffer.end();

            vk::SubmitInfo submitInfo = {};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            if(context->graphicsQueue.submit(1, &submitInfo, nullptr) != vk::Result::eSuccess){
                throw std::runtime_error("Failed to submit copy buffer command");
            }
            context->graphicsQueue.waitIdle();

            context->device.freeCommandBuffers(commandPool, 1, &commandBuffer);

            context->device.destroyCommandPool(commandPool, nullptr);
        }

        std::vector<Vertex> vertices;
        vk::Buffer vertexBuffer;
        vk::DeviceMemory vertexBufferMemory;
        void CreateObjects();
        void DestroyObjects();

        std::vector<uint32_t> indices;
        vk::Buffer indexBuffer;
        vk::DeviceMemory indexBufferMemory;
        void CreateIndexBuffer();
        void DestroyIndexBuffer();

        std::vector<vk::Buffer> uniformBuffers;
        std::vector<vk::DeviceMemory> uniformBuffersMemory;
        void CreateUniformBuffers();
        void DestroyUniformBuffers();
        void UpdateUniformBuffer(uint32_t currentImage);



    public:
        Engine() = default;
        ~Engine() = default;

        void Init(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface, int width, int height);
        void Quit();
        void Tick(bool &shouldClose);

    private:
        int width;
        int height;
        std::unique_ptr<StaticMesh> staticMesh;
        std::unique_ptr<Image> image;
    };
}