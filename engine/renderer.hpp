#pragma once

#include "vulkan/vulkan.hpp"

namespace engine
{
    class Renderer final
    {
    public:
        Renderer();
        ~Renderer();

        void Render();

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
    };

}