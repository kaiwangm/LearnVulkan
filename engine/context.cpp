#include "context.hpp"

namespace engine
{
    Context::Context(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface)
    {
        CreateInstance(extensions);
        pickupPhysicalDevice();
        surface = createSurface(instance);
        queryQueueFamilyIndices();
        createLogicalDevice();
    }

    Context::~Context()
    {
        instance.destroySurfaceKHR(surface);
        device.destroy();
        instance.destroy();
    }

    void Context::CreateInstance(const std::vector<const char *> &extensions)
    {
        vk::InstanceCreateInfo createInfo;
        vk::ApplicationInfo appInfo;
        appInfo.setApiVersion(VK_API_VERSION_1_3);

        createInfo.setPApplicationInfo(&appInfo);
        instance = vk::createInstance(createInfo);

        std::vector<const char *> layers = {"VK_LAYER_KHRONOS_validation"};

        createInfo.setPEnabledLayerNames(layers)
            .setPEnabledExtensionNames(extensions);

        instance = vk::createInstance(createInfo);
    }

    void Context::pickupPhysicalDevice()
    {
        auto physicalDevices = instance.enumeratePhysicalDevices();
        if (physicalDevices.size() == 0)
        {
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }

        for (auto &device : physicalDevices)
        {
            auto properties = device.getProperties();
            auto features = device.getFeatures();

            if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu && features.geometryShader)
            {
                phyDevice = device;
                break;
            }
        }

        if (!phyDevice)
        {
            phyDevice = physicalDevices[0];
        }

        std::cout << "Physical Device: " << phyDevice.getProperties().deviceName << std::endl;
    }

    void Context::createLogicalDevice()
    {
        std::array<const char *, 1> extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };
        vk::DeviceCreateInfo createInfo;
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        float priorities = 1.0f;
        if (queueFamilyIndices.presentQueue.value() == queueFamilyIndices.graphicsQueue.value())
        {
            vk::DeviceQueueCreateInfo queueCreateInfo;
            queueCreateInfo.setQueueFamilyIndex(queueFamilyIndices.graphicsQueue.value())
                .setQueueCount(1)
                .setPQueuePriorities(&priorities);
            queueCreateInfos.push_back(std::move(queueCreateInfo));
        }
        else
        {
            vk::DeviceQueueCreateInfo queueCreateInfo;
            queueCreateInfo.setQueueFamilyIndex(queueFamilyIndices.graphicsQueue.value())
                .setQueueCount(1)
                .setPQueuePriorities(&priorities);
            queueCreateInfos.push_back(queueCreateInfo);
            queueCreateInfo.setQueueFamilyIndex(queueFamilyIndices.presentQueue.value())
                .setQueueCount(1)
                .setPQueuePriorities(&priorities);
            queueCreateInfos.push_back(queueCreateInfo);
        }

        createInfo.setQueueCreateInfos(queueCreateInfos)
            .setPEnabledExtensionNames(extensions);

        device = phyDevice.createDevice(createInfo);
        if (!device)
        {
            throw std::runtime_error("Failed to create logical device!");
        }

        graphicsQueue = device.getQueue(queueFamilyIndices.graphicsQueue.value(), 0);
        presentQueue = device.getQueue(queueFamilyIndices.presentQueue.value(), 0);
    }

    void Context::queryQueueFamilyIndices()
    {
        auto queueFamilies = phyDevice.getQueueFamilyProperties();
        for (int i = 0; i < queueFamilies.size(); i++)
        {
            const auto &queueFamily = queueFamilies[i];
            if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                queueFamilyIndices.graphicsQueue = i;
            }
            if (phyDevice.getSurfaceSupportKHR(i, surface))
            {
                queueFamilyIndices.presentQueue = i;
            }

            if (queueFamilyIndices)
            {
                break;
            }
        }
    }
}