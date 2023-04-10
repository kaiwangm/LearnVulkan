#include "SDL.h"
#include "SDL_vulkan.h"
#include <iostream>
#include <vector>

#include "engine/engine.hpp"

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *window = SDL_CreateWindow(
        "LearnVulkan",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

    if (window == nullptr)
    {
        std::cout << "Failed to create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    bool shouldClose = false;
    SDL_Event event;

    engine::Init();

    while (!shouldClose)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                shouldClose = true;
            }
        }
    }

    engine::Quit();

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}