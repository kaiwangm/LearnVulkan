#pragma once

#include "vulkan/vulkan.hpp"

namespace engine
{
    class Swapchain final
    {
    public:
        vk::SwapchainKHR swapchain;

        Swapchain(int width, int height);
        ~Swapchain();

        struct SwapchainInfo
        {
            vk::Extent2D imageExtent;
            uint32_t imageCount;
            vk::SurfaceFormatKHR format;
            vk::SurfaceTransformFlagBitsKHR transform;
            vk::PresentModeKHR present;
        };

        SwapchainInfo swapchainInfo;
        std::vector<vk::Image> images;
        std::vector<vk::ImageView> imageViews;
        std::vector<vk::Framebuffer> framebuffers;

        void querySwapchainInfo(int width, int height);
        void getImages();
        void createImageViews();
        void createFramebuffers(int width, int height);
    };
}
