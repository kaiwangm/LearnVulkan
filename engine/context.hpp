#pragma once

#include "vulkan/vulkan.hpp"
#include <iostream>
#include <memory>
#include <optional>

namespace engine
{
    class Context final
    {
    public:
        static void Init();
        static void Quit();
        static Context &GetInstance();

        ~Context();

        struct QueueFamilyIndices final
        {
            std::optional<uint32_t> graphicsQueue;
        };

        vk::Instance instance;
        vk::PhysicalDevice phyDevice;
        vk::Device device;
        vk::Queue graphicsQueue;
        QueueFamilyIndices queueFamilyIndices;

    private:
        static std::unique_ptr<Context> instance_;
        Context();

        void CreateInstance();
        void pickupPhysicalDevice();
        void createLogicalDevice();
        void getQueues();
        void queryQueueFamilyIndices();
    };
}