#include "SDL.h"
#include "SDL_vulkan.h"
#include <iostream>
#include <vector>

#include "engine/engine.hpp"

int main(int argc, char **argv)
{
    // show work dir
    char *base_path = SDL_GetBasePath();
    if (base_path)
    {
        std::cout << "Working directory: " << base_path << std::endl;
        SDL_free(base_path);
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        throw std::runtime_error("Failed to initialize SDL");
    }

    unsigned int width = 1200;
    unsigned int height = 800;


    SDL_Window *window = SDL_CreateWindow(
        "LearnVulkan",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

    if (window == nullptr)
    {
        std::cout << "Failed to create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    bool shouldClose = false;
    SDL_Event event;

    unsigned int extensionCount = 0;
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
    std::vector<const char *> extensions(extensionCount);
    SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data());

    engine::Init(
        extensions,
        [&](vk::Instance instance)
        {
            VkSurfaceKHR surface;
            if (!SDL_Vulkan_CreateSurface(window, instance, &surface))
            {
                throw std::runtime_error("Failed to create surface");
            }
            return surface;
        },
        width, height);

    auto &renderer = engine::GetRenderer();

    while (!shouldClose)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                shouldClose = true;
            }

            renderer.Render();
        }
    }

    engine::Quit();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}