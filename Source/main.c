#include "..\Include\PNGDecoder.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>

static int graphics_init(SDL_Window **window, SDL_Renderer **renderer, SDL_Texture **texture)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    *window = SDL_CreateWindow("SDL is active!", 100, 100, 1000, 1000, 0);

    if (!*window)
    {
        SDL_Log("Unable to create window: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!*renderer)
    {
        SDL_Log("Unable to create renderer : %s", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return -1;
    }

    int width;
    int height;
    int channels;

    unsigned char *pixels = PNG_decode("Source/cane.png", &width, &height, &channels);
    if (!pixels)
    {
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
        SDL_Log("Unable to open image");
        SDL_Quit();
        return -1;
    }
    SDL_Log("Image width : %d height: %d channels: %d", width, height, channels);
    *texture = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, width, height);
    if (!*texture)
    {
        free(pixels);
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
        SDL_Log("Unable to create texture : %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    SDL_SetTextureAlphaMod(*texture, 255);
    SDL_SetTextureBlendMode(*texture, SDL_BLENDMODE_BLEND);
    SDL_UpdateTexture(*texture, NULL, pixels, width * 4);
    free(pixels);

    return 0;
}

int main()
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    int width = 0;
    int height = 0;
    int channels = 0;
    if (graphics_init(&window, &renderer, &texture))
    {
        return -1;
    }

    int running = 1;
    int x = 100;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_Rect target_rect = {0,0, 977, 724};
        SDL_RenderCopy(renderer, texture, NULL, &target_rect);
        SDL_RenderPresent(renderer);
    }

    SDL_Quit();
    return 0;
}