#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define NUM_LAYERS 32
#define SPACING 2 // Espaçamento em pixels entre as camadas

// Estrutura simplificada para representar uma fatia/camada
typedef struct {
    SDL_Texture* texture;
    int size;
} SpriteLayer;

int main(int argc, char* argv[]) {
    // 1. Inicialização padrão do SDL3
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Erro ao inicializar SDL: %s", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Sprite Stacking 3D - SDL3", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) {
        SDL_Log("Erro ao criar janela: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        SDL_Log("Erro ao criar renderizador: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // 2. Criando texturas procedimentais (Gerando um cone de baixo para cima)
    SpriteLayer layers[NUM_LAYERS];
    for (int i = 0; i < NUM_LAYERS; i++) {
        // A base é maior, o topo é menor
        int radius = (NUM_LAYERS - i) * 2 + 10; 
        int tex_size = radius * 2 + 2;
        
        layers[i].size = tex_size;
        layers[i].texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, tex_size, tex_size);
        
        // Ativa transparência na textura
        SDL_SetTextureBlendMode(layers[i].texture, SDL_BLENDMODE_BLEND);
        
        // Desenha o círculo na textura
        SDL_SetRenderTarget(renderer, layers[i].texture);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // Fundo transparente
        SDL_RenderClear(renderer);
        
        // Altera a cor dependendo da altura para dar profundidade visual
        SDL_SetRenderDrawColor(renderer, 50 + (i * 5), 150 + (i * 3), 250 - (i * 4), 255);
        
        // Desenha um "pixel art" simples ou quadrado/círculo centralizado
        SDL_FRect rect = { (float)tex_size/4.0f, (float)tex_size/4.0f, (float)tex_size/2.0f, (float)tex_size/2.0f };
        SDL_RenderFillRect(renderer, &rect);
    }
    
    // Restaura o alvo de renderização principal para a tela
    SDL_SetRenderTarget(renderer, NULL);

    // 3. Loop principal do jogo
    bool running = true;
    float angle = 0.0f;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        // Atualiza a rotação continuamente
        angle += 1.0f;
        if (angle >= 360.0f) angle -= 360.0f;

        // Limpa a tela com um fundo escuro
        SDL_SetRenderDrawColor(renderer, 20, 20, 25, 255);
        SDL_RenderClear(renderer);

        // Posição central do objeto na tela
        float base_x = SCREEN_WIDTH / 2.0f;
        float base_y = SCREEN_HEIGHT / 2.0f + 100.0f; // Ajustado um pouco para baixo

        // 4. O TRUQUE: Renderiza as camadas de baixo para cima
        for (int i = 0; i < NUM_LAYERS; i++) {
            // Desloca cada camada para cima no eixo Y baseado no índice da fatia
            float render_x = base_x - (layers[i].size / 2.0f);
            float render_y = (base_y - (i * SPACING)) - (layers[i].size / 2.0f);

            SDL_FRect dst_rect = { render_x, render_y, (float)layers[i].size, (float)layers[i].size };
            
            // No SDL3, SDL_RenderTextureRotated cuida da rotação mantendo o ponto central
            SDL_RenderTextureRotated(renderer, layers[i].texture, NULL, &dst_rect, (double)angle, NULL, SDL_FLIP_NONE);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    // 5. Limpeza de memória
    for (int i = 0; i < NUM_LAYERS; i++) {
        SDL_DestroyTexture(layers[i].texture);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
