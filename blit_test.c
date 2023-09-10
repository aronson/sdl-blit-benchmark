#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdio.h>

uint64_t rotl(uint64_t x, int k) { return (x << k) | (x >> (-k & 63)); }
uint64_t next(uint64_t state[2]) {
    uint64_t s0 = state[0], s1 = state[1];
    uint64_t result = rotl((s0 + s1) * 9, 29) + s0;
    state[0] = s0 ^ rotl(s1, 29);
    state[1] = s0 ^ s1 << 9;
    return result;
}
static uint64_t rngState[2] = {1, 2};
uint32_t get_random_uint32() {
    return (uint32_t)next(rngState);
}

int testRandomToRandomSVGAMultipleIterations();

SDL_Surface* getNextRandomSurface(SDL_PixelFormatEnum format) {
    # define BUF_LENGTH (15 * 15)
    Uint32 pixels[BUF_LENGTH];

    for (int i = 0; i < BUF_LENGTH; i++) {
        pixels[i] = get_random_uint32();
    }
    return SDL_CreateSurfaceFrom(pixels, 15, 15, 15 * 4, format);
}

long hashSurfacePixels(SDL_Surface* surface) {
    const int length = surface->w * surface->h;
    long hash = 0;
    for (int i = 0; i < length; i++){
        Uint32 pixel = ((Uint32*)(surface->pixels))[i];
        hash = (hash + (324723947 + pixel) ^93485734985);
    }
    return hash;
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    (testRandomToRandomSVGAMultipleIterations());

    return 0;
}

int testRandomToRandomSVGAMultipleIterations() {
    const int width = 800;
    const int height = 600;
    SDL_Surface* destSurface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA8888);

    for (int i = 0; i < 250000; i++) {
        SDL_Surface *sourceSurface = getNextRandomSurface(SDL_PIXELFORMAT_RGBA8888);

        SDL_Rect destRect;
        int location = (int)get_random_uint32();
        destRect.x = location % (width - 15 - 1);
        destRect.y = location % (height - 15 - 1);

        SDL_BlitSurface(sourceSurface, NULL, destSurface, &destRect);

        SDL_DestroySurface(sourceSurface);
    }
    long hash = hashSurfacePixels(destSurface);
    printf("Random to Random VGA blit: %lx\n", hash);
    SDL_SaveBMP(destSurface, "output.bmp");

    SDL_DestroySurface(destSurface);
    return hash == 0x4118a61cd181f;
}
