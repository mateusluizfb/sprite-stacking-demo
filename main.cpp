#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h>
#include <iterator>
#include <stdio.h>
#include <string>
#include <iostream>

int main(int arhc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Sprite Stacking test", 1200, 800, SDL_WINDOW_OPENGL);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    SDL_Event event;
    bool exit = false;

    // Sprite Renderding

    SDL_Texture* texture = IMG_LoadTexture(renderer, "./spritesheet.png");

    float width;
    float height;
    SDL_GetTextureSize(texture, &width, &height);

    std::cout << width << std::endl;
    std::cout << height << std::endl;

    float posX = 0;
    float posY = 0;
    float w = 146;
    float h = 146;

    SDL_FRect scRect = {
        posX,
        posY,
        w,
        h
    };

    SDL_FRect dsRect = {
        posX,
        posY,
        w,
        h
    };

    // Main Loop
    while(true) {
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                exit = true;
            }
        }

        // Update

        // Render
        SDL_RenderTextureRotated(
            renderer,
            texture,
            &scRect,
            &dsRect,
            0.0,
            nullptr,
            SDL_FLIP_NONE
        );

        if (exit) {
            break;
        }

        SDL_RenderPresent(renderer);
    }
}
