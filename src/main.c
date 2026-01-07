#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

static inline uint32_t pack_rbga8(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    return ((uint32_t) r) | ((uint32_t) g << 8) | ((uint32_t) b << 16) | ((uint32_t) a << 24);
}

int main(int argc, char** argv)
{
    (void) argc; (void) argv;

    if(SDL_Init(SDL_INIT_VIDEO)!= 0)
    {
        fprintf(stderr, "SDL failed to init");
        return 1;
    }

    const int W = 960, H = 540;

    SDL_Window* win = SDL_CreateWindow("JianJian Render", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, 0);
    if(!win)
    {
        fprintf(stderr, "create window failed");
        return 1;
    }

    SDL_Renderer* render = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if(!render)
    {
        fprintf(stderr, "create render failed");
        return 1;
    }
    
    //use a render texture as the screen
    SDL_Texture* tex = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, W, H);
    if(!tex)
    {
        fprintf(stderr, "create tex failed");
        return 1;
    }

    uint32_t* color = (uint32_t*) SDL_malloc((size_t)W * (size_t)H * sizeof(uint32_t));
    if(!color)
    {
        fprintf(stderr, "create color failed");
        return 1;
    }

    bool running = true;
    uint32_t t0 = SDL_GetTicks();

    while(running)
    {
        SDL_Event e;

        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT) running = false;
            if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        uint32_t t = SDL_GetTicks() - t0;

        for(int j = 0; j < H; j++)
        {
            for(int i = 0; i < W; i++)
            {
                uint8_t r = (uint8_t)((i + (t / 10)) & 255);
                uint8_t g = (uint8_t)((i + (t / 15)) & 255);
                uint8_t b = (uint8_t)((i^j) & 255);
                color[j * W + i] = pack_rbga8(r, g, b, 255);
            }
        }

        SDL_UpdateTexture(tex, NULL, color, W * (int)sizeof(uint32_t));
        SDL_RenderClear(render);
        SDL_RenderCopy(render, tex, NULL, NULL);
        SDL_RenderPresent(render);
    }

    SDL_free(color);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}