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

        std::vector<Vertex> vertices;
        void *data;
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
    };
}