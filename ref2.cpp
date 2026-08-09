#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cmath>
#include <vector>
#include <iostream>

constexpr int WINDOW_WIDTH = 1200;
constexpr int WINDOW_HEIGHT = 800;
constexpr int LAYER_COUNT = 16;
constexpr int SLICE_SIZE = 32; // 32x32 pixels per slice
constexpr float RENDER_SCALE = 4.0f;
constexpr float LAYER_SPACING = 2.0f;

struct Vec3 {
    float x, y, z;
};

struct VertexUV {
    float u, v;
};

// Mode switch
enum class RenderMode {
    METHOD_1_QUAD_GEOMETRY,
    METHOD_2_VOXEL_SAMPLING
};

// ============================================================================
// 3D TRANSFORM HELPER
// Rotates a local 3D point (x, y, z) by Yaw and Pitch, then returns 2D screen pos
// ============================================================================
SDL_FPoint Project3DTo2D(Vec3 local_pos, float yaw_rad, float pitch_rad, float center_x, float center_y) {
    // 1. Yaw Rotation (around Y-axis)
    float x_yaw = local_pos.x * cosf(yaw_rad) + local_pos.z * sinf(yaw_rad);
    float z_yaw = -local_pos.x * sinf(yaw_rad) + local_pos.z * cosf(yaw_rad);

    // 2. Pitch Rotation (around X-axis)
    float y_pitch = local_pos.y * cosf(pitch_rad) - z_yaw * sinf(pitch_rad);

    // 3. Project to Screen Space (Screen Y increases downwards)
    return SDL_FPoint{
        center_x + x_yaw * RENDER_SCALE,
        center_y - y_pitch * RENDER_SCALE
    };
}

// ============================================================================
// METHOD 1: Quad Projection using SDL_RenderGeometry
// ============================================================================
void RenderMethod1_Geometry(SDL_Renderer* renderer, SDL_Texture* texture, 
                            float yaw_rad, float pitch_rad, float world_x, float world_y) {
    
    float half_size = SLICE_SIZE / 2.0f;

    // Define UV coordinates for a single slice inside our horizontal spritesheet texture
    // Total texture width = SLICE_SIZE * LAYER_COUNT
    float tex_total_width = (float)(SLICE_SIZE * LAYER_COUNT);

    for (int i = 0; i < LAYER_COUNT; ++i) {
        float layer_y = i * LAYER_SPACING;

        // Local 3D corners of the layer quad (Clockwise: Top-Left, Top-Right, Bottom-Right, Bottom-Left)
        Vec3 corners[4] = {
            { -half_size, layer_y, -half_size },
            {  half_size, layer_y, -half_size },
            {  half_size, layer_y,  half_size },
            { -half_size, layer_y,  half_size }
        };

        // Calculate UV coordinates for layer 'i'
        float u0 = (i * SLICE_SIZE) / tex_total_width;
        float u1 = ((i + 1) * SLICE_SIZE) / tex_total_width;
        VertexUV uvs[4] = { {u0, 0.0f}, {u1, 0.0f}, {u1, 1.0f}, {u0, 1.0f} };

        SDL_Vertex vertices[4];
        for (int c = 0; c < 4; ++c) {
            vertices[c].position = Project3DTo2D(corners[c], yaw_rad, pitch_rad, world_x, world_y);
            vertices[c].color = SDL_FColor{ 1.0f, 1.0f, 1.0f, 1.0f }; // White tint (normal texture colors)
            vertices[c].tex_coord = SDL_FPoint{ uvs[c].u, uvs[c].v };
        }

        // Two triangles forming the quad: (0, 1, 2) and (2, 3, 0)
        int indices[6] = { 0, 1, 2, 2, 3, 0 };

        SDL_RenderGeometry(renderer, texture, vertices, 4, indices, 6);
    }
}

// ============================================================================
// METHOD 2: Voxel Point Sampling
// ============================================================================
void RenderMethod2_Voxels(SDL_Renderer* renderer, SDL_Surface* surface, 
                          float yaw_rad, float pitch_rad, float world_x, float world_y) {
    
    float half_size = SLICE_SIZE / 2.0f;
    uint32_t* pixels = (uint32_t*)surface->pixels;

    for (int i = 0; i < LAYER_COUNT; ++i) {
        float layer_y = i * LAYER_SPACING;
        int slice_start_x = i * SLICE_SIZE;

        for (int py = 0; py < SLICE_SIZE; ++py) {
            for (int px = 0; px < SLICE_SIZE; ++px) {
                
                // Sample pixel color from CPU surface
                int img_x = slice_start_x + px;
                uint32_t pixel_color = pixels[py * surface->w + img_x];

                uint8_t r, g, b, a;
                SDL_GetRGBA(pixel_color, SDL_GetPixelFormatDetails(surface->format), NULL, &r, &g, &b, &a);

                // Skip transparent pixels
                if (a < 10) continue;

                // Local 3D coordinate of the voxel point
                Vec3 voxel_pos = {
                    (float)px - half_size,
                    layer_y,
                    (float)py - half_size
                };

                SDL_FPoint screen_pos = Project3DTo2D(voxel_pos, yaw_rad, pitch_rad, world_x, world_y);

                // Render a small scaled dot/rect for each voxel
                SDL_SetRenderDrawColor(renderer, r, g, b, a);
                SDL_FRect voxel_rect = {
                    screen_pos.x,
                    screen_pos.y,
                    RENDER_SCALE,
                    RENDER_SCALE
                };
                SDL_RenderFillRect(renderer, &voxel_rect);
            }
        }
    }
}

// ============================================================================
// HELPER: Procedurally generate a square slice texture for testing
// ============================================================================
SDL_Surface* CreateProceduralSpritesheet() {
    int total_w = SLICE_SIZE * LAYER_COUNT;
    int total_h = SLICE_SIZE;

    SDL_Surface* surface = SDL_CreateSurface(total_w, total_h, SDL_PIXELFORMAT_RGBA8888);
    SDL_LockSurface(surface);

    uint32_t* pixels = (uint32_t*)surface->pixels;

    for (int layer = 0; layer < LAYER_COUNT; ++layer) {
        for (int y = 0; y < SLICE_SIZE; ++y) {
            for (int x = 0; x < SLICE_SIZE; ++x) {
                int px = layer * SLICE_SIZE + x;
                int idx = y * total_w + px;

                // Create a hollow square border with colored inner features
                bool is_border = (x == 0 || x == SLICE_SIZE - 1 || y == 0 || y == SLICE_SIZE - 1);
                bool is_inner_square = (x >= 8 && x <= 23 && y >= 8 && y <= 23);

                if (is_border) {
                    // Dark wooden border
                    pixels[idx] = SDL_MapRGBA(SDL_GetPixelFormatDetails(surface->format), NULL, 120, 70, 30, 255);
                } else if (is_inner_square && layer > 2 && layer < 14) {
                    // Bright color gradient across layers
                    uint8_t red = 50 + layer * 12;
                    uint8_t green = 200 - layer * 8;
                    pixels[idx] = SDL_MapRGBA(SDL_GetPixelFormatDetails(surface->format), NULL, red, green, 220, 255);
                } else if (x > 2 && x < SLICE_SIZE - 3 && y > 2 && y < SLICE_SIZE - 3) {
                    // Gray fill
                    pixels[idx] = SDL_MapRGBA(SDL_GetPixelFormatDetails(surface->format), NULL, 180, 180, 190, 255);
                } else {
                    // Transparent space
                    pixels[idx] = SDL_MapRGBA(SDL_GetPixelFormatDetails(surface->format), NULL, 0, 0, 0, 0);
                }
            }
        }
    }

    SDL_UnlockSurface(surface);
    return surface;
}

// ============================================================================
// MAIN ENTRY POINT
// ============================================================================
int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init Error: %s", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Perspective-Preserving Sprite Stacking", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    // Create procedural surface & texture
    SDL_Surface* surface = CreateProceduralSpritesheet();
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    // Variables
    float yaw_deg = 45.0f;
    float pitch_deg = 35.0f;
    RenderMode current_mode = RenderMode::METHOD_1_QUAD_GEOMETRY;

    bool running = true;
    SDL_Event event;

    std::cout << "--- CONTROLS ---\n";
    std::cout << "LEFT / RIGHT : Rotate Yaw\n";
    std::cout << "UP / DOWN    : Adjust Pitch\n";
    std::cout << "M            : Toggle Method 1 (Geometry) / Method 2 (Voxels)\n";

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                switch (event.key.key) {
                    case SDLK_LEFT:  yaw_deg -= 3.0f; break;
                    case SDLK_RIGHT: yaw_deg += 3.0f; break;
                    case SDLK_UP:    pitch_deg += 2.0f; if (pitch_deg > 85.0f) pitch_deg = 85.0f; break;
                    case SDLK_DOWN:  pitch_deg -= 2.0f; if (pitch_deg < 5.0f) pitch_deg = 5.0f; break;
                    case SDLK_M:
                        current_mode = (current_mode == RenderMode::METHOD_1_QUAD_GEOMETRY) 
                            ? RenderMode::METHOD_2_VOXEL_SAMPLING 
                            : RenderMode::METHOD_1_QUAD_GEOMETRY;
                        std::cout << "Switched to: " 
                                  << (current_mode == RenderMode::METHOD_1_QUAD_GEOMETRY ? "Method 1 (SDL_RenderGeometry)" : "Method 2 (Voxel Point Sampling)") 
                                  << std::endl;
                        break;
                    case SDLK_ESCAPE: running = false; break;
                }
            }
        }

        // Convert angles to radians
        float yaw_rad = yaw_deg * (3.14159265f / 180.0f);
        float pitch_rad = pitch_deg * (3.14159265f / 180.0f);

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 20, 22, 28, 255);
        SDL_RenderClear(renderer);

        float center_x = WINDOW_WIDTH / 2.0f;
        float center_y = WINDOW_HEIGHT / 2.0f + 100.0f;

        // Render selected method
        if (current_mode == RenderMode::METHOD_1_QUAD_GEOMETRY) {
            RenderMethod1_Geometry(renderer, texture, yaw_rad, pitch_rad, center_x, center_y);
        } else {
            RenderMethod2_Voxels(renderer, surface, yaw_rad, pitch_rad, center_x, center_y);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    // Cleanup
    SDL_DestroyTexture(texture);
    SDL_DestroySurface(surface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
