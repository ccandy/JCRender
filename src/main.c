// main.c
#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "renderer_sw/jc_raster.h"


static void clear_rgba8(uint32_t* pixels, int w, int h, uint32_t rgba8)
{
    for (int i = 0; i < w * h; ++i) pixels[i] = rgba8;
}

int main(int argc, char** argv)
{
    (void)argc; (void)argv;

    const int W = 960;
    const int H = 540;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) return 1;

    SDL_Window* win = SDL_CreateWindow(
        "JCRender",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        W, H,
        0);

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* tex = SDL_CreateTexture(
        ren,
        SDL_PIXELFORMAT_RGBA8888,   
        SDL_TEXTUREACCESS_STREAMING,
        W, H);

    uint32_t* framebuffer = (uint32_t*)SDL_malloc((size_t)W * (size_t)H * sizeof(uint32_t));
    if (!framebuffer) return 1;

    bool running = true;
    while (running)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        clear_rgba8(framebuffer, W, H, jc_pack_rgba8(0.08f, 0.08f, 0.10f, 1.0f));

        JC_Vertex2D v0 = { .pos = { 100.0f,  80.0f }, .color = { 1, 0, 0, 1 } };
        JC_Vertex2D v1 = { .pos = { 800.0f, 120.0f }, .color = { 0, 1, 0, 1 } };
        JC_Vertex2D v2 = { .pos = { 300.0f, 460.0f }, .color = { 0, 0, 1, 1 } };

        jc_raster_tri_color_rgba8(framebuffer, W, H, v0, v1, v2);

        SDL_UpdateTexture(tex, NULL, framebuffer, W * (int)sizeof(uint32_t));
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, NULL, NULL);
        SDL_RenderPresent(ren);
    }

    SDL_free(framebuffer);
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
