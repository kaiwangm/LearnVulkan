#pragma once

#include "vulkan/vulkan.hpp"
#include <iostream>
#include <memory>
#include <optional>
#include <memory>
#include <iostream>
#include <functional>

#include "glm/glm.hpp"

namespace engine
{
    using CreateSurfaceFunction = std::function<VkSurfaceKHR(vk::Instance)>;
    struct Vertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texCoord;
    };

    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

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
        std::vector<vk::DescriptorSet> descriptorSets;
        vk::DescriptorSetLayout descriptorSetLayout;

    public:
        void CreateInstance(const std::vector<const char *> &extensions);
        void pickupPhysicalDevice();
        void createLogicalDevice();
        void queryQueueFamilyIndices();
        void createDescriptorPool();
        void createDescriptorSetLayout();
        void createDescriptorSets(std::vector<vk::Buffer>& uniformBuffers, uint32_t swapChainImagesCount);
    };
}