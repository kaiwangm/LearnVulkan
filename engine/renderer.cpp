#include <limits>
#include "renderer.hpp"
#include "context.hpp"
#include "render_process.hpp"
#include "swapchain.hpp"


namespace engine
{
    Renderer::Renderer(const engine::Context *context, const RenderProcess *renderProcess, const Swapchain *swapchain)
    {
        this->context = context;
        this->renderProcess = renderProcess;
        this->swapchain = swapchain;

        initCmdPool();
        allocateCmdBuffer();
        createSems();
        createFence();
    }

    Renderer::~Renderer()
    {
        auto &device = context->device;
        device.freeCommandBuffers(cmdPool, cmdBuffer);
        device.destroyCommandPool(cmdPool);
        device.destroySemaphore(imageAvailableSemaphore);
        device.destroySemaphore(imageDrawFinishedSemaphore);
        device.destroyFence(cmdAvailableFence);
    }

    void Renderer::Render()
    {
        const auto &device = context->device;

        auto result = device.acquireNextImageKHR(swapchain->swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore);

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
        context->graphicsQueue.submit(submitInfo, cmdAvailableFence);

        vk::PresentInfoKHR presentInfo;
        presentInfo.setImageIndices(imageIndex)
            .setSwapchains(swapchain->swapchain)
            .setWaitSemaphores(imageDrawFinishedSemaphore);

        if (context->presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to present swapchain image!");
        }

        if (context->device.waitForFences(cmdAvailableFence, true, UINT64_MAX) != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to wait for fence!");
        }

        context->device.resetFences(cmdAvailableFence);
    }

    void Renderer::createSems()
    {
        vk::SemaphoreCreateInfo semInfo;
        imageAvailableSemaphore = context->device.createSemaphore(semInfo);
        imageDrawFinishedSemaphore = context->device.createSemaphore(semInfo);
    }

    void Renderer::createFence()
    {
        vk::FenceCreateInfo fenceInfo;
        cmdAvailableFence = context->device.createFence(fenceInfo);
    }

    void Renderer::initCmdPool()
    {
        vk::CommandPoolCreateInfo cmdPoolInfo;
        cmdPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

        cmdPool = context->device.createCommandPool(cmdPoolInfo);
    }

    void Renderer::allocateCmdBuffer()
    {
        vk::CommandBufferAllocateInfo cmdBufferInfo;
        cmdBufferInfo.setCommandPool(cmdPool)
            .setCommandBufferCount(1)
            .setLevel(vk::CommandBufferLevel::ePrimary);

        cmdBuffer = context->device.allocateCommandBuffers(cmdBufferInfo)[0];
    }
}