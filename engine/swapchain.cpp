#include "swapchain.hpp"
#include "context.hpp"

namespace engine
{
    Swapchain::Swapchain(int w, int h)
    {
        querySwapchainInfo(w, h);

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
            .setPresentMode(swapchainInfo.present);

        auto& queueIndicecs = Context::GetInstance().queueFamilyIndices;
        if (queueIndicecs.graphicsQueue.value() == queueIndicecs.presentQueue.value())
        {
            swapchainCreateInfo.setQueueFamilyIndices(queueIndicecs.graphicsQueue.value())
                            .setImageSharingMode(vk::SharingMode::eExclusive);
        }
        else
        {
            std::array indices = {queueIndicecs.graphicsQueue.value(), queueIndicecs.presentQueue.value()};
            swapchainCreateInfo.setQueueFamilyIndices(indices)
                            .setImageSharingMode(vk::SharingMode::eConcurrent);
        }

    }

    Swapchain::~Swapchain()
    {
        Context::GetInstance().device.destroySwapchainKHR(swapchain);
    }

    void Swapchain::querySwapchainInfo(int w, int h)
    {
        auto &phyDevice = Context::GetInstance().phyDevice;
        auto &surface = Context::GetInstance().surface;
        auto formats = phyDevice.getSurfaceFormatsKHR(surface);
        swapchainInfo.format = formats[0].format;
        for (const auto &format : formats)
        {
            if (format.format == vk::Format::eR8G8B8A8Srgb &&
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                swapchainInfo.format = format.format;
                break;
            }
        }

        auto capabilities = phyDevice.getSurfaceCapabilitiesKHR(surface);
        swapchainInfo.imageCount = std::clamp<uint32_t>(2, capabilities.minImageCount, capabilities.maxImageCount);

        swapchainInfo.imageExtent.width = std::clamp<uint32_t>(w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        swapchainInfo.imageExtent.height = std::clamp<uint32_t>(h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

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
}