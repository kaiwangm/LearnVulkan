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

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

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

        vk::CommandBuffer beginSingleTimeCommands(vk::CommandPool commandPool)
        {
            vk::CommandBufferAllocateInfo allocInfo = {};
            allocInfo.level = vk::CommandBufferLevel::ePrimary;
            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;

            vk::CommandBuffer commandBuffer;
            if (context->device.allocateCommandBuffers(&allocInfo, &commandBuffer) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to allocate command buffer");
            }

            vk::CommandBufferBeginInfo beginInfo = {};
            beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

            if (commandBuffer.begin(&beginInfo) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to begin recording command buffer");
            }

            return commandBuffer;
        }

        void endSingleTimeCommands(vk::CommandBuffer commandBuffer, vk::CommandPool commandPool)
        {
            commandBuffer.end();

            vk::SubmitInfo submitInfo = {};
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            if (context->graphicsQueue.submit(1, &submitInfo, nullptr) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to submit copy buffer command");
            }
            context->graphicsQueue.waitIdle();

            context->device.freeCommandBuffers(commandPool, 1, &commandBuffer);
        }

        void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
        {
            vk::CommandPool commandPool;
            vk::CommandPoolCreateInfo poolInfo = {};
            poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;

            if (context->device.createCommandPool(&poolInfo, nullptr, &commandPool) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to create command pool");
            }

            vk::CommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

            vk::BufferCopy copyRegion = {};
            copyRegion.size = size;
            commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

            endSingleTimeCommands(commandBuffer, commandPool);

            context->device.destroyCommandPool(commandPool, nullptr);
        }

        vk::Image depthImage;
        vk::DeviceMemory depthImageMemory;
        vk::ImageView depthImageView;
        void CreateDepthResources();
        void DestroyDepthResources();
        vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
        {
            for (vk::Format format : candidates)
            {
                vk::FormatProperties props;
                context->phyDevice.getFormatProperties(format, &props);

                if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
                {
                    return format;
                }
                else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
                {
                    return format;
                }
            }

            throw std::runtime_error("Failed to find supported format");
        }
        vk::Format findDepthFormat()
        {
            return findSupportedFormat(
                {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
                vk::ImageTiling::eOptimal,
                vk::FormatFeatureFlagBits::eDepthStencilAttachment);
        }
        bool hasStencilComponent(vk::Format format)
        {
            return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
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

        vk::Image textureImage;
        vk::ImageView textureImageView;
        vk::Sampler textureSampler;
        vk::DeviceMemory textureImageMemory;
        void CreateTextureImage();
        void DestroyTextureImage();
        void createImage(uint32_t width, uint32_t height,
                         vk::Format format, vk::ImageTiling tiling,
                         vk::ImageUsageFlags usage,
                         vk::MemoryPropertyFlags properties,
                         vk::Image &image, vk::DeviceMemory &imageMemory)
        {
            vk::ImageCreateInfo imageInfo = {};
            imageInfo.imageType = vk::ImageType::e2D;
            imageInfo.extent.width = width;
            imageInfo.extent.height = height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = format;
            imageInfo.tiling = tiling;
            imageInfo.initialLayout = vk::ImageLayout::eUndefined;
            imageInfo.usage = usage;
            imageInfo.samples = vk::SampleCountFlagBits::e1;
            imageInfo.sharingMode = vk::SharingMode::eExclusive;

            if (context->device.createImage(&imageInfo, nullptr, &image) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to create image");
            }

            vk::MemoryRequirements memRequirements;
            context->device.getImageMemoryRequirements(image, &memRequirements);

            vk::MemoryAllocateInfo allocInfo = {};
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

            if (context->device.allocateMemory(&allocInfo, nullptr, &imageMemory) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to allocate image memory");
            }

            context->device.bindImageMemory(image, imageMemory, 0);
        }
        void transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
        {
            vk::CommandPool commandPool;
            vk::CommandPoolCreateInfo poolInfo = {};
            poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;

            if (context->device.createCommandPool(&poolInfo, nullptr, &commandPool) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to create command pool");
            }

            vk::CommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

            vk::ImageMemoryBarrier barrier = {};
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
            {
                barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

                if (hasStencilComponent(format))
                {
                    barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
                }
            }
            else
            {
                barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            }
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            vk::PipelineStageFlags sourceStage;
            vk::PipelineStageFlags destinationStage;

            if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
            {
                barrier.srcAccessMask = vk::AccessFlags();
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

                sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
                destinationStage = vk::PipelineStageFlagBits::eTransfer;
            }
            else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
            {
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

                sourceStage = vk::PipelineStageFlagBits::eTransfer;
                destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
            }
            else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
            {
                barrier.srcAccessMask = vk::AccessFlags();
                barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

                sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
                destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
            }
            else
            {
                throw std::invalid_argument("Unsupported layout transition");
            }

            commandBuffer.pipelineBarrier(
                sourceStage, destinationStage,
                vk::DependencyFlags(),
                0, nullptr,
                0, nullptr,
                1, &barrier);

            endSingleTimeCommands(commandBuffer, commandPool);

            context->device.destroyCommandPool(commandPool, nullptr);
        }
        void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
        {
            vk::CommandPool commandPool;
            vk::CommandPoolCreateInfo poolInfo = {};
            poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;

            if (context->device.createCommandPool(&poolInfo, nullptr, &commandPool) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to create command pool");
            }

            vk::CommandBuffer commandBuffer = beginSingleTimeCommands(commandPool);

            vk::BufferImageCopy region = {};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = vk::Offset3D{0, 0, 0};
            region.imageExtent = vk::Extent3D{width, height, 1};

            commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

            endSingleTimeCommands(commandBuffer, commandPool);

            context->device.destroyCommandPool(commandPool, nullptr);
        }
        vk::ImageView createImageView(vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags)
        {
            vk::ImageViewCreateInfo viewInfo = {};
            viewInfo.image = image;
            viewInfo.viewType = vk::ImageViewType::e2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspectFlags;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            vk::ImageView imageView;
            if (context->device.createImageView(&viewInfo, nullptr, &imageView) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to create texture image view");
            }

            return imageView;
        }

    public:
        Engine() = default;
        ~Engine() = default;

        void Init(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface, int width, int height, SDL_Window *window);
        void Quit();
        void Tick(bool &shouldClose);

    private:
        int width;
        int height;
        std::unique_ptr<StaticMesh> staticMesh;
        std::unique_ptr<Image> image;
    };
}