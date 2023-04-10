#include "engine/context.hpp"

namespace engine
{
    std::unique_ptr<Context> Context::instance_ = nullptr;
    void Context::Init()
    {
        instance_.reset(new Context());
    }

    void Context::Quit()
    {
        instance_.reset();
    }

    Context &Context::GetInstance()
    {
        return *instance_;
    }

    Context::Context()
    {
        CreateInstance();
        pickupPhysicalDevice();
        queryQueueFamilyIndices();
        createLogicalDevice();
        getQueues();
    }

    Context::~Context()
    {
        device.destroy();
        instance.destroy();
    }

    void Context::CreateInstance()
    {
        vk::InstanceCreateInfo createInfo;
        vk::ApplicationInfo appInfo;
        std::vector<const char *> layers = {
            "VK_LAYER_KHRONOS_validation",
        };

        appInfo.setApiVersion(VK_API_VERSION_1_3)
            .setPApplicationName("LearnVulkan");

        createInfo.setPApplicationInfo(&appInfo)
            .setPEnabledLayerNames(layers);

        instance = vk::createInstance(createInfo);
    }

    void Context::pickupPhysicalDevice()
    {
        auto physicalDevices = instance.enumeratePhysicalDevices();
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
        vk::DeviceCreateInfo createInfo;
        vk::DeviceQueueCreateInfo queueCreateInfo;
        float priorities = 1.0f;
        queueCreateInfo.setQueueFamilyIndex(0)
            .setQueueCount(1)
            .setPQueuePriorities(&priorities);
        createInfo.setQueueCreateInfos(queueCreateInfo);

        device = phyDevice.createDevice(createInfo);
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
                break;
            }
        }
    }

    void Context::getQueues()
    {
        graphicsQueue = device.getQueue(queueFamilyIndices.graphicsQueue.value(), 0);
    }
}