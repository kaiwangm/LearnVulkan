#include "swapchain.hpp"
#include "context.hpp"

namespace engine
{
    Swapchain::Swapchain(int width, int height)
    {
        querySwapchainInfo(width, height);

        vk::SwapchainCreateInfoKHR swapchainCreateInfo;
        swapchainCreateInfo.setClipped(true)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setSurface(Context::GetInstance().surface)
            .setImageColorSpace(swapchainInfo.format.colorSpace)
            .setImageFormat(swapchainInfo.format.format)
            .setImageExtent(swapchainInfo.imageExtent)
            .setMinImageCount(swapchainInfo.imageCount)
            .setPreTransform(swapchainInfo.transform)
            .setPresentMode(swapchainInfo.present);

        auto &queueIndicecs = Context::GetInstance().queueFamilyIndices;
        if (queueIndicecs.graphicsQueue.value() == queueIndicecs.presentQueue.value())
        {
            swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eExclusive);
        }
        else
        {
            std::array indices = {queueIndicecs.graphicsQueue.value(), queueIndicecs.presentQueue.value()};
            swapchainCreateInfo.setQueueFamilyIndices(indices)
                .setImageSharingMode(vk::SharingMode::eConcurrent);
        }

        swapchain = Context::GetInstance().device.createSwapchainKHR(swapchainCreateInfo);

        getImages();
        createImageViews();
    }

    Swapchain::~Swapchain()
    {
        for (auto &framebuffer : framebuffers)
        {
            Context::GetInstance().device.destroyFramebuffer(framebuffer);
        }
        for (auto &imageView : imageViews)
        {
            Context::GetInstance().device.destroyImageView(imageView);
        }
        Context::GetInstance().device.destroySwapchainKHR(swapchain);
    }

    void Swapchain::querySwapchainInfo(int width, int height)
    {
        auto &phyDevice = Context::GetInstance().phyDevice;
        auto &surface = Context::GetInstance().surface;
        auto formats = phyDevice.getSurfaceFormatsKHR(surface);
        swapchainInfo.format = formats[0];
        for (const auto &format : formats)
        {
            if (format.format == vk::Format::eR8G8B8A8Sint &&
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                swapchainInfo.format = format.format;
                break;
            }
        }

        auto capabilities = phyDevice.getSurfaceCapabilitiesKHR(surface);
        swapchainInfo.imageCount = std::clamp<uint32_t>(2, capabilities.minImageCount, capabilities.maxImageCount);

        swapchainInfo.imageExtent.width = std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        swapchainInfo.imageExtent.height = std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        swapchainInfo.transform = capabilities.currentTransform;

        auto presentModes = phyDevice.getSurfacePresentModesKHR(surface);
        swapchainInfo.present = vk::PresentModeKHR::eFifo;
        for (const auto &presentMode : presentModes)
        {
            if (presentMode == vk::PresentModeKHR::eMailbox)
            {
                swapchainInfo.present = presentMode;
                break;
            }
        }
    }

    void Swapchain::getImages()
    {
        images = Context::GetInstance().device.getSwapchainImagesKHR(swapchain);
    }

    void Swapchain::createImageViews()
    {
        imageViews.resize(images.size());
        for (size_t i = 0; i < images.size(); i++)
        {
            vk::ImageViewCreateInfo imageViewCreateInfo;
            imageViewCreateInfo.setImage(images[i])
                .setViewType(vk::ImageViewType::e2D)
                .setComponents(vk::ComponentMapping())
                .setFormat(swapchainInfo.format.format)
                .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

            imageViews[i] = Context::GetInstance().device.createImageView(imageViewCreateInfo);
        }
    }

    void Swapchain::createFramebuffers(int width, int height)
    {
        framebuffers.resize(imageViews.size());
        for (size_t i = 0; i < imageViews.size(); i++)
        {
            vk::FramebufferCreateInfo framebufferCreateInfo;
            framebufferCreateInfo.setAttachments(imageViews[i])
                .setWidth(width)
                .setHeight(height)
                .setRenderPass(Context::GetInstance().renderProcess->renderPass)
                .setLayers(1);

            framebuffers[i] = Context::GetInstance().device.createFramebuffer(framebufferCreateInfo);
        }
    }
}