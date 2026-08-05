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
#include <cmath>
#include <vector>
#include <stdio.h>
#include <iostream>

template <typename T>
void print(const T& arg) {
    std::cout << arg << std::endl;
}

struct Sprite {
    float pos_x;
    float pos_y;
    float w;
    float h;
    float y_offset;
    float pitch_angle;
};

int main(int arhc, char* argv[]) {
    print("Starting");

    SDL_Init(SDL_INIT_VIDEO);

    int window_width = 1200;
    int window_height = 800;

    SDL_Window* window = SDL_CreateWindow("Sprite Stacking test", window_width, window_height, SDL_WINDOW_OPENGL);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    SDL_Event event;
    bool exit = false;

    // Sprite Rendering
    SDL_Texture* texture = IMG_LoadTexture(renderer, "./spritesheet.png");

    int sprite_count = 5;
    float sprite_width = 32;
    float sprite_height = 32;
    float current_layer_gap = 0;
    float pitch_angle = 0.785f;
    float rotation_angle = 0.0f;
    float layer_gap = 5.0;

    // Create sprites 
    std::vector<Sprite> sprites = {};

    for (int i = 0; i < sprite_count; i++) {
        Sprite sprite = {
            0 + (float) (i * sprite_width),
            0,
            sprite_width,
            sprite_height,
            current_layer_gap,
            pitch_angle
        };
        sprites.push_back(sprite); 

        current_layer_gap += layer_gap;
    }

    // Main Loop
    while(true) {
        // Update
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                exit = true;
            }

            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_UP) {
                    for (int i = 0; i < sprite_count; i++) {
                        sprites[i].pitch_angle += 0.1;
                    }
                }

                if (event.key.key == SDLK_DOWN) {
                    for (int i = 0; i < sprite_count; i++) {
                        sprites[i].pitch_angle -= 0.1;
                    }
                }

                if (event.key.key == SDLK_LEFT) {
                    for (int i = 0; i < sprite_count; i++) {
                        rotation_angle -= 1.0f;
                    }
                }

                if (event.key.key == SDLK_RIGHT) {
                    for (int i = 0; i < sprite_count; i++) {
                        rotation_angle += 1.0f;
                    }
                }
            }
        }

        // Render
        SDL_RenderClear(renderer);
 
        for (int i = 0; i < sprite_count; i++) {
            Sprite sprite = sprites[i];

            SDL_FRect scRect = {
                sprite.pos_x,
                sprite.pos_y,
                sprite.w,
                sprite.h
            };

            float dsPosX = (window_width / 2.0) - (sprite.w / 2);
            float dsPosY = (window_height / 2.0) - (sprite.h / 2);

            SDL_FRect dsRect = {
                dsPosX,
                dsPosY - sprite.y_offset,
                sprite.w * 4,
                sprite.h * sinf(sprite.pitch_angle) * 4
            };

            SDL_RenderTextureRotated(
                renderer,
                texture,
                &scRect,
                &dsRect,
                rotation_angle,
                nullptr,
                SDL_FLIP_NONE
            );
        }

        if (exit) {
            break;
        }

        SDL_RenderPresent(renderer);
    }
}
