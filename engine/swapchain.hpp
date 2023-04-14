#pragma once

#include "vulkan/vulkan.hpp"
#include "context.hpp"
#include "render_process.hpp"

namespace engine
{
    class Swapchain final
    {
    public:
        vk::SwapchainKHR swapchain;

        Swapchain(const engine::Context *context, int width, int height);
        ~Swapchain();

        struct SwapchainInfo
        {
            vk::SurfaceCapabilitiesKHR capabilities;
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
        void createFramebuffers(const RenderProcess *renderProcess, int width, int height);

    private:
        const engine::Context *context;
    };
}
