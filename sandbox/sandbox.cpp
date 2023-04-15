#include <iostream>
#include <vector>

#include "SDL.h"
#include "SDL_vulkan.h"

#include "engine/engine.hpp"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"

class Sandbox
{
public:
    Sandbox(unsigned int width, unsigned int height) : workDir(SDL_GetBasePath()), width(width), height(height)
    {
        std::cout << "Working directory: " << workDir << std::endl;

        if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        {
            SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
            throw std::runtime_error("Failed to initialize SDL");
        }

        window = SDL_CreateWindow(
            "LearnVulkan",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

        if (window == nullptr)
        {
            throw std::runtime_error(std::string("Failed to create window: ") + SDL_GetError());
        }

        unsigned int extensionCount = 0;
        SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
        std::vector<const char *> extensions(extensionCount);
        SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data());

        engine.Init(
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
    }
    ~Sandbox()
    {
        engine.Quit();
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void Run()
    {
        bool shouldClose = false;
        SDL_Event event;

        while (!shouldClose)
        {
            while (SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    shouldClose = true;
                }

                engine.Tick();
            }
        }
    }

private:
    std::string workDir;
    unsigned int width;
    unsigned int height;

    SDL_Window *window;
    engine::Engine engine;
};

int main(int argc, char **argv)
{
    Sandbox sandbox(800, 600);
    sandbox.Run();

    return 0;
}