#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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
void resetRngStream() {
    rngState[0] = 1;
    rngState[1] = 2;
}

int testRandomToRandomSVGA();
int testRandomToRandomSVGAMultipleIterations();

Uint32 *getNextRandomBuffer(const int width, const int height) {
    Uint32* buf = SDL_malloc(sizeof(Uint32) * width * height);
    for (int i = 0; i < width * height; i++) {
        buf[i] = get_random_uint32();
    }
    return buf;
}

SDL_Surface* getRandomBlitChunk(Uint32 *pixels, SDL_PixelFormatEnum format) {
    return SDL_CreateSurfaceFrom(pixels, 15, 15, 15 * 4, format);
}

SDL_Surface* getRandomSVGASurface(Uint32 *pixels, SDL_PixelFormatEnum format) {
    return SDL_CreateSurfaceFrom(pixels, 800, 600, 800 * 4, format);
}

Uint32 FNVHash(Uint32* buf, unsigned int length) {
    const Uint32 fnv_prime = 0x811C9DC5;
    Uint32 hash = 0;

    for (int i = 0; i < length; buf++, i++)
    {
        hash *= fnv_prime;
        hash ^= (*buf);
    }

    return hash;
}

Uint32 hashSurfacePixels(SDL_Surface * surface) {
    uint64_t buffer_size = surface->w * surface->h;
    return FNVHash(surface->pixels, buffer_size);
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    (testRandomToRandomSVGA());
    (testRandomToRandomSVGAMultipleIterations());

    return 0;
}

int testRandomToRandomSVGA() {
    const int width = 800;
    const int height = 600;
    const Uint32 good_hash = 0xfc3eb115;
    // Reset RNG
    resetRngStream();
    // Allocate random buffers
    Uint32 *dest_pixels = getNextRandomBuffer(width, height);
    Uint32 *src_pixels = getNextRandomBuffer(width, height);
    // Create surfaces of different pixel formats
    SDL_Surface* dest_surface = getRandomSVGASurface(dest_pixels, SDL_PIXELFORMAT_BGRA8888);
    SDL_Surface* src_surface = getRandomSVGASurface(src_pixels, SDL_PIXELFORMAT_RGBA8888);
    // Blit surfaces
    SDL_Rect dest_rect;
    SDL_BlitSurface(src_surface, NULL, dest_surface, &dest_rect);
    // Check result
    Uint32 hash = hashSurfacePixels(dest_surface);
    printf("Random to Random SVGA blit single iteration: 0x%x... ", hash);
    // Clean up
    SDL_DestroySurface(dest_surface);
    SDL_DestroySurface(src_surface);
    SDL_free(dest_pixels);
    SDL_free(src_pixels);
    const int result = hash == good_hash;
    if (result) {
        printf("passed!\n");
    } else {
        printf("failed!\n");
    }
    return result;
}

int testRandomToRandomSVGAMultipleIterations() {
    const int width = 800;
    const int height = 600;
    const Uint32 x86_simd_hash = 0x2626be78;
    const Uint32 scalar_hash = 0xfb2a8ee8;
    // Reset RNG
    resetRngStream();
    // Create blank source surface
    SDL_Surface* destSurface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ABGR8888);

    // Perform 250k random blits into random areas of the blank surface
    for (int i = 0; i < 250000; i++) {
        Uint32 *buf = getNextRandomBuffer(15, 15);
        SDL_Surface *sourceSurface = getRandomBlitChunk(buf, SDL_PIXELFORMAT_RGBA8888);

        SDL_Rect dest_rect;
        int location = (int)get_random_uint32();
        dest_rect.x = location % (width - 15 - 1);
        dest_rect.y = location % (height - 15 - 1);

        SDL_BlitSurface(sourceSurface, NULL, destSurface, &dest_rect);

        SDL_DestroySurface(sourceSurface);
        SDL_free(buf);
    }
    // Check result
    Uint32 hash = hashSurfacePixels(destSurface);
    printf("Random to Random SVGA blit 250k iterations: 0x%x... ", hash);
    // Clean up
    SDL_DestroySurface(destSurface);
    const int result = hash == x86_simd_hash || hash == scalar_hash;
    if (result) {
        printf("passed!\n");
    } else {
        printf("failed!\n");
    }
    return result;
}
