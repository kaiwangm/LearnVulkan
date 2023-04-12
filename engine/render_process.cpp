#include "render_process.hpp"
#include "context.hpp"

namespace engine
{
    void RenderProcess::InitPipeline(const Shader &shader, int width, int height)
    {
        vk::GraphicsPipelineCreateInfo pipelineInfo;

        // 1. vertex input
        vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
        pipelineInfo.setPVertexInputState(&vertexInputInfo);

        // 2. vertex input assembly
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
        inputAssembly.setPrimitiveRestartEnable(VK_FALSE)
            .setTopology(vk::PrimitiveTopology::eTriangleList);
        pipelineInfo.setPInputAssemblyState(&inputAssembly);

        // 3. shader
        auto stages = shader.GetStage();
        pipelineInfo.setStages(stages);

        // 4. viewport
        vk::PipelineViewportStateCreateInfo viewportInfo;
        vk::Viewport viewport(0, 0, width, height, 0, 1);
        viewportInfo.setViewports(viewport);
        vk::Rect2D scissor({0, 0}, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
        viewportInfo.setScissors(scissor);
        pipelineInfo.setPViewportState(&viewportInfo);

        // 5. Rasterization
        vk::PipelineRasterizationStateCreateInfo rasterizerInfo;
        rasterizerInfo.setRasterizerDiscardEnable(VK_FALSE)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setLineWidth(1.0f);
        pipelineInfo.setPRasterizationState(&rasterizerInfo);

        // 6. multisampling
        vk::PipelineMultisampleStateCreateInfo multisamplingInfo;
        multisamplingInfo.setSampleShadingEnable(VK_FALSE)
            .setRasterizationSamples(vk::SampleCountFlagBits::e1);
        pipelineInfo.setPMultisampleState(&multisamplingInfo);

        // 7. test - stencil, depth

        // 8. color blending
        vk::PipelineColorBlendStateCreateInfo colorBlendingInfo;
        vk::PipelineColorBlendAttachmentState colorBlendAttachment;
        colorBlendAttachment.setBlendEnable(VK_FALSE)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR |
                               vk::ColorComponentFlagBits::eG |
                               vk::ColorComponentFlagBits::eB |
                               vk::ColorComponentFlagBits::eA);

        colorBlendingInfo.setLogicOpEnable(VK_FALSE)
            .setAttachments(colorBlendAttachment);
        pipelineInfo.setPColorBlendState(&colorBlendingInfo);

        // 9. renderPass and layout
        pipelineInfo.setRenderPass(renderPass)
            .setLayout(layout);

        // result
        auto result = Context::GetInstance().device.createGraphicsPipeline(nullptr, pipelineInfo);
        if (result.result != vk::Result::eSuccess)
        {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        pipeline = result.value;
    }

    RenderProcess::~RenderProcess()
    {
        auto& device = Context::GetInstance().device;
        device.destroyRenderPass(renderPass);
        device.destroyPipelineLayout(layout);
        device.destroyPipeline(pipeline);
    }

    void RenderProcess::InitLayout()
    {
        vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
        layout = Context::GetInstance().device.createPipelineLayout(pipelineLayoutInfo);
    }

    void RenderProcess::InitRenderPass()
    {
        vk::RenderPassCreateInfo renderPassInfo;
        vk::AttachmentDescription attachDesc;
        attachDesc.setFormat(Context::GetInstance().swapchain->swapchainInfo.format.format)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setSamples(vk::SampleCountFlagBits::e1);
        renderPassInfo.setAttachments(attachDesc);

        vk::AttachmentReference attachRef;
        attachRef.setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        vk::SubpassDescription subpass;
        subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachments(attachRef);
        renderPassInfo.setSubpasses(subpass);

        vk::SubpassDependency dependency;
        dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        renderPassInfo.setDependencies(dependency);

        renderPass = Context::GetInstance().device.createRenderPass(renderPassInfo);  
    }
}