#include "renderer.hpp"
#include "context.hpp"
#include <limits>

namespace engine
{
    Renderer::Renderer()
    {
        initCmdPool();
        allocateCmdBuffer();
        createSems();
        createFence();
    }

    Renderer::~Renderer()
    {
        auto &device = Context::GetInstance().device;
        device.freeCommandBuffers(cmdPool, cmdBuffer);
        device.destroyCommandPool(cmdPool);
        device.destroySemaphore(imageAvailableSemaphore);
        device.destroySemaphore(imageDrawFinishedSemaphore);
        device.destroyFence(cmdAvailableFence);
    }

    void Renderer::Render()
    {
        auto &device = Context::GetInstance().device;
        auto &renderProcess = Context::GetInstance().renderProcess;
        auto &swapchain = Context::GetInstance().swapchain;

        auto result = device.acquireNextImageKHR(Context::GetInstance().swapchain->swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore);

        if (result.result != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to acquire swapchain image!");
        }

        auto imageIndex = result.value;

        cmdBuffer.reset();

        vk::CommandBufferBeginInfo beginInfo;
        beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        cmdBuffer.begin(beginInfo);
        {
            vk::RenderPassBeginInfo renderPassBeginInfo;
            vk::Rect2D renderArea;
            vk::ClearValue clearValue;
            clearValue.setColor(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}));
            renderArea.setOffset({0, 0})
                .setExtent(swapchain->swapchainInfo.imageExtent);

            renderPassBeginInfo.setRenderPass(renderProcess->renderPass)
                .setRenderArea(renderArea)
                .setFramebuffer(swapchain->framebuffers[imageIndex])
                .setClearValues(clearValue);

            cmdBuffer.beginRenderPass(renderPassBeginInfo, {});
            {
                cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, renderProcess->pipeline);
                cmdBuffer.draw(3, 1, 0, 0);
            }
            cmdBuffer.endRenderPass();
        }
        cmdBuffer.end();

        vk::SubmitInfo submitInfo;
        vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        submitInfo.setCommandBuffers(cmdBuffer)
            .setWaitSemaphores(imageAvailableSemaphore)
            .setWaitDstStageMask(flags)
            .setSignalSemaphores(imageDrawFinishedSemaphore);
        Context::GetInstance().graphicsQueue.submit(submitInfo, cmdAvailableFence);

        vk::PresentInfoKHR presentInfo;
        presentInfo.setImageIndices(imageIndex)
            .setSwapchains(swapchain->swapchain)
            .setWaitSemaphores(imageDrawFinishedSemaphore);

        if (Context::GetInstance().presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to present swapchain image!");
        }

        if (Context::GetInstance().device.waitForFences(cmdAvailableFence, true, UINT64_MAX) != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to wait for fence!");
        }

        Context::GetInstance().device.resetFences(cmdAvailableFence);
    }

    void Renderer::createSems()
    {
        vk::SemaphoreCreateInfo semInfo;
        imageAvailableSemaphore = Context::GetInstance().device.createSemaphore(semInfo);
        imageDrawFinishedSemaphore = Context::GetInstance().device.createSemaphore(semInfo);
    }

    void Renderer::createFence()
    {
        vk::FenceCreateInfo fenceInfo;
        cmdAvailableFence = Context::GetInstance().device.createFence(fenceInfo);
    }

    void Renderer::initCmdPool()
    {
        vk::CommandPoolCreateInfo cmdPoolInfo;
        cmdPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

        cmdPool = Context::GetInstance().device.createCommandPool(cmdPoolInfo);
    }

    void Renderer::allocateCmdBuffer()
    {
        vk::CommandBufferAllocateInfo cmdBufferInfo;
        cmdBufferInfo.setCommandPool(cmdPool)
            .setCommandBufferCount(1)
            .setLevel(vk::CommandBufferLevel::ePrimary);

        cmdBuffer = Context::GetInstance().device.allocateCommandBuffers(cmdBufferInfo)[0];
    }
}