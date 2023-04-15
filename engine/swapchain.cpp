#include "swapchain.hpp"

namespace engine
{
    Swapchain::Swapchain(const engine::Context *context, int width, int height)
    {
        this->context = context;

        querySwapchainInfo(width, height);

        vk::SwapchainCreateInfoKHR swapchainCreateInfo;
        swapchainCreateInfo.setClipped(true)
            .setImageArrayLayers(1)
            .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
            .setSurface(context->surface)
            .setImageColorSpace(swapchainInfo.format.colorSpace)
            .setImageFormat(swapchainInfo.format.format)
            .setImageExtent(swapchainInfo.imageExtent)
            .setMinImageCount(swapchainInfo.imageCount)
            .setPreTransform(swapchainInfo.transform)
            .setPresentMode(swapchainInfo.present);

        auto &queueIndicecs = context->queueFamilyIndices;
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

        // swapchain = context->device.createSwapchainKHR(swapchainCreateInfo);

        // getImages();
        // createImageViews();
    }

    Swapchain::~Swapchain()
    {
        for (auto &framebuffer : framebuffers)
        {
            context->device.destroyFramebuffer(framebuffer);
        }
        for (auto &imageView : imageViews)
        {
            context->device.destroyImageView(imageView);
        }
        context->device.destroySwapchainKHR(swapchain);
    }

    void Swapchain::querySwapchainInfo(int width, int height)
    {
        auto &phyDevice = context->phyDevice;
        auto &surface = context->surface;

        auto capabilities = phyDevice.getSurfaceCapabilitiesKHR(surface);
        swapchainInfo.capabilities = capabilities;

        auto formats = phyDevice.getSurfaceFormatsKHR(surface);
        swapchainInfo.format = formats[0];

        for (const auto &format : formats)
        {
            if (format.format == vk::Format::eB8G8R8A8Unorm &&
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                swapchainInfo.format = format.format;
                break;
            }
        }

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
        images = context->device.getSwapchainImagesKHR(swapchain);
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

            imageViews[i] = context->device.createImageView(imageViewCreateInfo);
        }
    }

    void Swapchain::createFramebuffers(const RenderProcess *renderProcess, int width, int height)
    {
        framebuffers.resize(imageViews.size());
        for (size_t i = 0; i < imageViews.size(); i++)
        {
            vk::FramebufferCreateInfo framebufferCreateInfo;
            framebufferCreateInfo.setAttachments(imageViews[i])
                .setWidth(width)
                .setHeight(height)
                .setRenderPass(renderProcess->renderPass)
                .setLayers(1);

            framebuffers[i] = context->device.createFramebuffer(framebufferCreateInfo);
        }
    }
}