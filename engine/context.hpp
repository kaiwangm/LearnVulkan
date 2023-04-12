#pragma once

#include "vulkan/vulkan.hpp"
#include <iostream>
#include <memory>
#include <optional>
#include <memory>
#include <cassert>
#include <iostream>
#include "swapchain.hpp"
#include "render_process.hpp"
#include "renderer.hpp"

namespace engine
{
    using CreateSurfaceFunction = std::function<VkSurfaceKHR(vk::Instance)>;

    class Context final
    {
    public:
        static void Init(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface);
        static void Quit();
        static Context &GetInstance();

        ~Context();

        struct QueueFamilyIndices final
        {
            std::optional<uint32_t> graphicsQueue;
            std::optional<uint32_t> presentQueue;

            operator bool() const
            {
                return graphicsQueue.has_value() && presentQueue.has_value();
            }
        };

        vk::Instance instance;
        vk::PhysicalDevice phyDevice;
        vk::Device device;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        vk::SurfaceKHR surface;
        std::unique_ptr<Swapchain> swapchain;
        std::unique_ptr<RenderProcess> renderProcess;
        std::unique_ptr<Renderer> renderer;
        QueueFamilyIndices queueFamilyIndices;

        void InitSwapchain(int width, int height){
            swapchain.reset(new Swapchain(width, height));
        }

        void DestroySwapchain(){
            swapchain.reset();
        }

        void InitRenderer(){
            renderer.reset(new Renderer());
        }

    private:
        static std::unique_ptr<Context> instance_;
        Context(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface);
        void CreateInstance(const std::vector<const char *> &extensions);
        void pickupPhysicalDevice();
        void createLogicalDevice();
        void getQueues();
        void queryQueueFamilyIndices();
    };
}