#include "engine.hpp"
#include <string>

#include "SDL.h"

namespace engine
{
    static void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }

    void Engine::SetupVulkanWindow(ImGui_ImplVulkanH_Window *wd, VkSurfaceKHR surface, int width, int height)
    {
        wd->Surface = surface;

        // Check for WSI support
        VkBool32 res;
        vkGetPhysicalDeviceSurfaceSupportKHR(context->phyDevice, context->queueFamilyIndices.graphicsQueue.value(), wd->Surface, &res);
        if (res != VK_TRUE)
        {
            fprintf(stderr, "Error no WSI support on physical device 0\n");
            exit(-1);
        }

        // Select Surface Format
        const VkFormat requestSurfaceImageFormat[] = {VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM};
        const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(context->phyDevice, wd->Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);

        // Select Present Mode
#ifdef IMGUI_UNLIMITED_FRAME_RATE
        VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR};
#else
        VkPresentModeKHR present_modes[] = {VK_PRESENT_MODE_FIFO_KHR};
#endif
        wd->PresentMode = ImGui_ImplVulkanH_SelectPresentMode(context->phyDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
        // printf("[vulkan] Selected PresentMode = %d\n", wd->PresentMode);

        // Create SwapChain, RenderPass, Framebuffer, etc.
        ImGui_ImplVulkanH_CreateOrResizeWindow(context->instance, context->phyDevice, context->device, wd, context->queueFamilyIndices.graphicsQueue.value(), nullptr, width, height, 2);
    }

    void Engine::CleanupVulkanWindow()
    {
        ImGui_ImplVulkanH_DestroyWindow(context->instance, context->device, &g_MainWindowData, nullptr);
    }

    void Engine::Init(const std::vector<const char *> &extensions, CreateSurfaceFunction createSurface, int width, int height, SDL_Window *window)
    {
        this->width = width;
        this->height = height;

        staticMesh = std::make_unique<StaticMesh>("assets/models/viking_room/viking_room.obj");
        image = std::make_unique<Image>("assets/models/viking_room/viking_room.png");

        // Create context
        context = std::make_unique<Context>(extensions, createSurface);
        InitImGui(window, width, height);

        CreateObjects();
        CreateTextureImage();
        CreateUniformBuffers();
        CreateDepthResources();

        swapchain = std::make_unique<Swapchain>(context.get(), width, height, depthImageView);

        // Create shader
        shader = std::make_unique<Shader>(context.get(), "assets/shaders/shader.vert.spv", "assets/shaders/shader.frag.spv");

        // Create render process
        renderProcess = std::make_unique<RenderProcess>(context.get());
        renderProcess->InitRenderPass(swapchain.get(), findDepthFormat());
        renderProcess->InitLayout();

        // Create swapchain
        // swapchain->createFramebuffers(renderProcess.get(), width, height, depthImageView);

        // Create pipeline
        renderProcess->InitPipeline(shader.get(), width, height);

        // Create renderer
        // renderer = std::make_unique<Renderer>(context.get(), renderProcess.get(), swapchain.get());

        
    }

    void Engine::InitImGui(SDL_Window *window, int width, int height)
    {
        this->window = window;

        // Init ImGui
        ImGui_ImplVulkanH_Window *wd = &g_MainWindowData;
        SetupVulkanWindow(wd, context->surface, width, height);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForVulkan(window);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = context->instance;
        init_info.PhysicalDevice = context->phyDevice;
        init_info.Device = context->device;
        init_info.QueueFamily = context->queueFamilyIndices.graphicsQueue.value();
        init_info.Queue = context->graphicsQueue;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = context->descriptorPool;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = wd->ImageCount;
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);

        {
            // Use any command queue
            VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
            VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;

            vkResetCommandPool(context->device, command_pool, 0);
            VkCommandBufferBeginInfo begin_info = {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(command_buffer, &begin_info);

            ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

            VkSubmitInfo end_info = {};
            end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            end_info.commandBufferCount = 1;
            end_info.pCommandBuffers = &command_buffer;
            vkEndCommandBuffer(command_buffer);
            vkQueueSubmit(context->graphicsQueue, 1, &end_info, VK_NULL_HANDLE);

            vkDeviceWaitIdle(context->device);
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }

    void Engine::RenderGui(bool &shouldClose)
    {
        static bool show_demo_window = false;
        static bool show_another_window = false;
        static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        // Resize swap chain?
        if (g_SwapChainRebuild)
        {
            SDL_GetWindowSize(window, &width, &height);
            if (width > 0 && height > 0)
            {
                ImGui_ImplVulkan_SetMinImageCount(2);
                ImGui_ImplVulkanH_CreateOrResizeWindow(context->instance, context->phyDevice, context->device, &g_MainWindowData, context->queueFamilyIndices.graphicsQueue.value(), nullptr, width, height, 2);
                g_MainWindowData.FrameIndex = 0;
                g_SwapChainRebuild = false;

                // reset view port
                renderProcess.reset();
                renderProcess = std::make_unique<RenderProcess>(context.get());
                renderProcess->InitRenderPass(swapchain.get(), findDepthFormat());
                renderProcess->InitLayout();
                renderProcess->InitPipeline(shader.get(), width, height);
            }
        }

        // Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui_ImplVulkanH_Window *wd = &g_MainWindowData;
        ImGuiIO &io = ImGui::GetIO();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("Vulkan First Triangle");              // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            // ImGui::SliderFloat("float", &f, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

            if (ImGui::Button("Exit"))
                shouldClose = true;

            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        ImDrawData *draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
            wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
            wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
            wd->ClearValue.color.float32[3] = clear_color.w;
            FrameRender(wd, draw_data);
            FramePresent(wd);
        }
    }

    void Engine::FrameRender(ImGui_ImplVulkanH_Window *wd, ImDrawData *draw_data)
    {
        VkResult err;

        VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
        VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        err = vkAcquireNextImageKHR(context->device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            g_SwapChainRebuild = true;
            return;
        }
        check_vk_result(err);

        UpdateUniformBuffer(wd->FrameIndex);

        ImGui_ImplVulkanH_Frame *fd = &wd->Frames[wd->FrameIndex];
        {
            err = vkWaitForFences(context->device, 1, &fd->Fence, VK_TRUE, UINT64_MAX); // wait indefinitely instead of periodically checking
            check_vk_result(err);

            err = vkResetFences(context->device, 1, &fd->Fence);
            check_vk_result(err);
        }
        {
            err = vkResetCommandPool(context->device, fd->CommandPool, 0);
            check_vk_result(err);
            VkCommandBufferBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
            check_vk_result(err);
        }
        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = wd->RenderPass;
            info.framebuffer = fd->Framebuffer;
            info.renderArea.extent.width = wd->Width;
            info.renderArea.extent.height = wd->Height;
            info.clearValueCount = 1;
            info.pClearValues = &wd->ClearValue;
            vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        vkCmdBindPipeline(fd->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderProcess->pipeline);
        vkCmdBindDescriptorSets(fd->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderProcess->layout, 0, 1, (VkDescriptorSet *)context->descriptorSets.data(), 0, nullptr);
        VkBuffer vertexBuffers[] = {(VkBuffer)vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(fd->CommandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(fd->CommandBuffer, (VkBuffer)indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(fd->CommandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        // Record dear imgui primitives into command buffer
        ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

        // Submit command buffer
        vkCmdEndRenderPass(fd->CommandBuffer);


        // {
        //     VkRenderPassBeginInfo info = {};
        //     info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        //     info.renderPass = renderProcess->renderPass;
        //     info.framebuffer = fd->Framebuffer;
        //     info.renderArea.extent.width = wd->Width;
        //     info.renderArea.extent.height = wd->Height;
        //     std::array<VkClearValue, 2> clearValues = {};
        //     clearValues[0].color = wd->ClearValue.color;
        //     clearValues[1].depthStencil = {1.0f, 0};
        //     info.clearValueCount = 2;
        //     info.pClearValues = clearValues.data();
        //     vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        // }

        // vkCmdBindPipeline(fd->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderProcess->pipeline);
        // vkCmdBindDescriptorSets(fd->CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderProcess->layout, 0, 1, (VkDescriptorSet *)context->descriptorSets.data(), 0, nullptr);
        // VkBuffer vertexBuffers[] = {(VkBuffer)vertexBuffer};
        // VkDeviceSize offsets[] = {0};
        // vkCmdBindVertexBuffers(fd->CommandBuffer, 0, 1, vertexBuffers, offsets);
        // vkCmdBindIndexBuffer(fd->CommandBuffer, (VkBuffer)indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        // vkCmdDrawIndexed(fd->CommandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

        // // Submit command buffer
        // vkCmdEndRenderPass(fd->CommandBuffer);

        
        {
            VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &image_acquired_semaphore;
            info.pWaitDstStageMask = &wait_stage;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &fd->CommandBuffer;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &render_complete_semaphore;

            err = vkEndCommandBuffer(fd->CommandBuffer);
            check_vk_result(err);
            err = vkQueueSubmit(context->graphicsQueue, 1, &info, fd->Fence);
            check_vk_result(err);
        }
    }

    void Engine::FramePresent(ImGui_ImplVulkanH_Window *wd)
    {
        if (g_SwapChainRebuild)
            return;
        VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        VkPresentInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &render_complete_semaphore;
        info.swapchainCount = 1;
        info.pSwapchains = &wd->Swapchain;
        info.pImageIndices = &wd->FrameIndex;
        VkResult err = vkQueuePresentKHR(context->graphicsQueue, &info);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            g_SwapChainRebuild = true;
            return;
        }
        check_vk_result(err);
        wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->ImageCount; // Now we can use the next set of semaphores
    }

    void Engine::Quit()
    {
        context->device.waitIdle();

        DestroyTextureImage();
        DestroyUniformBuffers();
        DestroyObjects();
        DestroyDepthResources();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        CleanupVulkanWindow();

        renderer.reset();
        renderProcess.reset();
        shader.reset();
        swapchain.reset();
        context.reset();
    }

    void Engine::Tick(bool &shouldClose)
    {
        RenderGui(shouldClose);
    }

    void Engine::CreateObjects()
    {
        // vertices = {
        //     {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        //     {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        //     {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}};

        // cube
        // vertices = {
        //     {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        //     {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        //     {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        //     {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        //     {{-0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        //     {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        //     {{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        //     {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        // };

        vertices = staticMesh->get_one_vertices();

        vk::DeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;

        createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                     stagingBuffer, stagingBufferMemory);

        void *verticesData;
        if (context->device.mapMemory(stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlags(), &verticesData) != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to map vertex buffer memory");
        }
        memcpy(verticesData, vertices.data(), (size_t)bufferSize);
        context->device.unmapMemory(stagingBufferMemory);

        createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
                     vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer, vertexBufferMemory);

        copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        context->device.destroyBuffer(stagingBuffer);
        context->device.freeMemory(stagingBufferMemory);

        CreateIndexBuffer();
    }

    void Engine::DestroyObjects()
    {
        DestroyIndexBuffer();
        context->device.destroyBuffer(vertexBuffer);
        context->device.freeMemory(vertexBufferMemory);
    }

    void Engine::CreateIndexBuffer()
    {
        // indices = {0, 1, 2};

        // cube
        // indices = {
        //     2, 1, 0,
        //     2, 0, 3,
        //     1, 5, 4,
        //     1, 4, 0,
        //     7, 4, 5,
        //     7, 5, 6,
        //     3, 0, 4,
        //     3, 4, 7,
        //     6, 5, 1,
        //     6, 1, 2,
        //     3, 7, 6,
        //     3, 6, 2,
        // };

        indices = staticMesh->get_one_indices();

        vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;

        createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                     stagingBuffer, stagingBufferMemory);

        void *indexData;
        if (context->device.mapMemory(stagingBufferMemory, 0, bufferSize, vk::MemoryMapFlags(), &indexData) != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to map index buffer memory");
        }
        memcpy(indexData, indices.data(), (size_t)bufferSize);
        context->device.unmapMemory(stagingBufferMemory);

        createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
                     vk::MemoryPropertyFlagBits::eDeviceLocal,
                     indexBuffer, indexBufferMemory);
        copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        context->device.destroyBuffer(stagingBuffer);
        context->device.freeMemory(stagingBufferMemory);
    }

    void Engine::DestroyIndexBuffer()
    {
        context->device.destroyBuffer(indexBuffer);
        context->device.freeMemory(indexBufferMemory);
    }

    void Engine::CreateUniformBuffers()
    {
        vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

        auto *wd = &g_MainWindowData;

        uniformBuffers.resize(wd->ImageCount);
        uniformBuffersMemory.resize(wd->ImageCount);

        for (size_t i = 0; i < wd->ImageCount; i++)
        {
            vk::BufferCreateInfo bufferInfo;
            bufferInfo.size = bufferSize;
            bufferInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
            bufferInfo.sharingMode = vk::SharingMode::eExclusive;

            if (context->device.createBuffer(&bufferInfo, nullptr, &uniformBuffers[i]) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to create uniform buffer");
            }

            vk::MemoryRequirements memRequirements;
            context->device.getBufferMemoryRequirements(uniformBuffers[i], &memRequirements);

            vk::MemoryAllocateInfo allocInfo;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

            if (context->device.allocateMemory(&allocInfo, nullptr, &uniformBuffersMemory[i]) != vk::Result::eSuccess)
            {
                throw std::runtime_error("Failed to allocate uniform buffer memory");
            }

            context->device.bindBufferMemory(uniformBuffers[i], uniformBuffersMemory[i], 0);
        }

        context->createDescriptorSets(uniformBuffers, wd->ImageCount, textureImageView, textureSampler);
    }

    void Engine::DestroyUniformBuffers()
    {
        auto *wd = &g_MainWindowData;

        for (size_t i = 0; i < wd->ImageCount; i++)
        {
            context->device.destroyBuffer(uniformBuffers[i]);
            context->device.freeMemory(uniformBuffersMemory[i]);
        }
    }

    void Engine::UpdateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo = {};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        // ubo.model = glm::mat4(1.0f);
        // ubo.view = glm::lookAt(glm::vec3(1.5f, 1.5f, 1.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.view = glm::lookAt(glm::vec3(0.0f, 1.8f, 1.8f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.proj = glm::perspective(glm::radians(60.0f), width / (float)height, 0.1f, 1000.0f);
        

        void *uboData;
        if (context->device.mapMemory(uniformBuffersMemory[currentImage], 0, sizeof(ubo), vk::MemoryMapFlags(), &uboData) != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to map uniform buffer memory");
        }
        memcpy(uboData, &ubo, sizeof(ubo));
        context->device.unmapMemory(uniformBuffersMemory[currentImage]);
    }

    void Engine::CreateDepthResources()
    {
        vk::Format depthFormat = findDepthFormat();
        createImage(width, height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
        depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
        transitionImageLayout(depthImage, depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    }


    void Engine::DestroyDepthResources()
    {

    }

    void Engine::CreateTextureImage()
    {
        vk::Buffer stagingBuffer;
        vk::DeviceMemory stagingBufferMemory;

        vk::DeviceSize textureSize = image->get_device_size();

        createBuffer(textureSize, vk::BufferUsageFlagBits::eTransferSrc,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                     stagingBuffer, stagingBufferMemory);

        void *textureData;
        if (context->device.mapMemory(stagingBufferMemory, 0, textureSize, vk::MemoryMapFlags(), &textureData) != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to map texture buffer memory");
        }
        memcpy(textureData, image->get_pixels(), (size_t)textureSize);
        context->device.unmapMemory(stagingBufferMemory);

        createImage(image->get_width(), image->get_height(), vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
                    vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                    vk::MemoryPropertyFlagBits::eDeviceLocal, textureImage, textureImageMemory);

        transitionImageLayout(textureImage,
                              vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined,
                              vk::ImageLayout::eTransferDstOptimal);
        copyBufferToImage(stagingBuffer, textureImage, image->get_width(), image->get_height());
        transitionImageLayout(textureImage,
                              vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal,
                              vk::ImageLayout::eShaderReadOnlyOptimal);

        context->device.destroyBuffer(stagingBuffer);
        context->device.freeMemory(stagingBufferMemory);

        // create image view
        textureImageView = createImageView(textureImage, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor);

        // create sampler
        vk::SamplerCreateInfo samplerInfo;
        samplerInfo.magFilter = vk::Filter::eLinear;
        samplerInfo.minFilter = vk::Filter::eLinear;
        samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
        samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 16;
        samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = vk::CompareOp::eAlways;
        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (context->device.createSampler(&samplerInfo, nullptr, &textureSampler) != vk::Result::eSuccess)
        {
            throw std::runtime_error("Failed to create texture sampler");
        }
    }

    void Engine::DestroyTextureImage()
    {
        context->device.destroySampler(textureSampler);
        context->device.destroyImageView(textureImageView);
        context->device.destroyImage(textureImage);
        context->device.freeMemory(textureImageMemory);
    }
}
