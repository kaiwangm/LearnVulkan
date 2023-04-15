#pragma once

#include "vulkan/vulkan.hpp"
#include "context.hpp"

namespace engine
{
    class RenderProcess;
    class Swapchain;

    class Renderer final
    {
    public:
        Renderer(const engine::Context *context, const RenderProcess *renderProcess, const Swapchain *swapchain);
        ~Renderer();

        void Render(VkFramebuffer framebuffer);

    private:
        vk::CommandPool cmdPool;
        vk::CommandBuffer cmdBuffer;

        vk::Semaphore imageAvailableSemaphore;
        vk::Semaphore imageDrawFinishedSemaphore;
        vk::Fence cmdAvailableFence;

        void initCmdPool();
        void allocateCmdBuffer();
        void createSems();
        void createFence();

        const engine::Context *context;
        const RenderProcess *renderProcess;
        const Swapchain *swapchain;
    };

}