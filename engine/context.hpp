#pragma once

#include "vulkan/vulkan.hpp"
#include <iostream>
#include <memory>
#include <optional>
#include <memory>
#include <iostream>
#include <functional>

namespace engine
{
    using CreateSurfaceFunction = std::function<VkSurfaceKHR(vk::Instance)>;

    class Context final
    {
    public:
        Context(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface);
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
        QueueFamilyIndices queueFamilyIndices;
        vk::DescriptorPool descriptorPool;

    private:
        void CreateInstance(const std::vector<const char *> &extensions);
        void pickupPhysicalDevice();
        void createLogicalDevice();
        void queryQueueFamilyIndices();
        void createDescriptorPool();
    };
}