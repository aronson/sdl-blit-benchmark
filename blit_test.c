#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

const uint32_t simd_result[4] = {0x148e9ddb, 0x9d5b220b, 0xeaf4d5a5, 0x26c5dc37};
const uint32_t classic_result[4] = {0xb300df09, 0xb81c0646, 0xd5faeb50, 0x7897fc07};

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

Uint32 *getNextRandomBuffer() {
    Uint32* buf = SDL_malloc(sizeof(Uint32) * 15 * 15);
    for (int i = 0; i < 15 * 15; i++) {
        buf[i] = get_random_uint32();
    }
    return buf;
}

SDL_Surface* getNextRandomSurface(Uint32 *pixels, SDL_PixelFormatEnum format) {
    return SDL_CreateSurfaceFrom(pixels, 15, 15, 15 * 4, format);
}

void murmurhash3_x86_128(const uint32_t *data, size_t len, uint32_t seed, uint32_t *out) {
    const uint32_t c1 = 0x239b961b;
    const uint32_t c2 = 0xab0e9789;
    const uint32_t c3 = 0x38b34ae5;
    const uint32_t c4 = 0xa1e38b93;

    uint32_t h1 = seed;
    uint32_t h2 = seed;
    uint32_t h3 = seed;
    uint32_t h4 = seed;

    for (size_t i = 0; i < len; i += 4) {
        uint32_t k1 = (i < len) ? data[i] : 0;
        uint32_t k2 = (i + 1 < len) ? data[i + 1] : 0;
        uint32_t k3 = (i + 2 < len) ? data[i + 2] : 0;
        uint32_t k4 = (i + 3 < len) ? data[i + 3] : 0;

        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> (32 - 15));
        k1 *= c2;

        h1 ^= k1;
        h1 = (h1 << 19) | (h1 >> (32 - 19));
        h1 += h2;
        h1 = h1 * 5 + 0x561ccd1b;

        k2 *= c2;
        k2 = (k2 << 16) | (k2 >> (32 - 16));
        k2 *= c3;

        h2 ^= k2;
        h2 = (h2 << 17) | (h2 >> (32 - 17));
        h2 += h3;
        h2 = h2 * 5 + 0x0bcaa747;

        k3 *= c3;
        k3 = (k3 << 17) | (k3 >> (32 - 17));
        k3 *= c4;

        h3 ^= k3;
        h3 = (h3 << 15) | (h3 >> (32 - 15));
        h3 += h4;
        h3 = h3 * 5 + 0x96cd1c35;

        k4 *= c4;
        k4 = (k4 << 18) | (k4 >> (32 - 18));
        k4 *= c1;

        h4 ^= k4;
        h4 = (h4 << 13) | (h4 >> (32 - 13));
        h4 += h1;
        h4 = h4 * 5 + 0x32ac3b17;
    }

    h1 ^= len * 4;
    h2 ^= len * 4;
    h3 ^= len * 4;
    h4 ^= len * 4;

    h1 += h2;
    h1 += h3;
    h1 += h4;
    h2 += h1;
    h3 += h1;
    h4 += h1;

    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    h2 ^= h2 >> 16;
    h2 *= 0x85ebca6b;
    h2 ^= h2 >> 13;
    h2 *= 0xc2b2ae35;
    h2 ^= h2 >> 16;

    h3 ^= h3 >> 16;
    h3 *= 0x85ebca6b;
    h3 ^= h3 >> 13;
    h3 *= 0xc2b2ae35;
    h3 ^= h3 >> 16;

    h4 ^= h4 >> 16;
    h4 *= 0x85ebca6b;
    h4 ^= h4 >> 13;
    h4 *= 0xc2b2ae35;
    h4 ^= h4 >> 16;

    out[0] = h1;
    out[1] = h2;
    out[2] = h3;
    out[3] = h4;
}

void hashSurfacePixels(SDL_Surface * surface, uint32_t * out) {
    uint64_t buffer_size = surface->w * surface->h;
    murmurhash3_x86_128(surface->pixels, buffer_size, 0, out);
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    assert(testRandomToRandomSVGAMultipleIterations());

    return 0;
}

int testRandomToRandomSVGAMultipleIterations() {
    const int width = 800;
    const int height = 600;
    SDL_Surface* destSurface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA8888);

    for (int i = 0; i < 250000; i++) {
        Uint32 *buf = getNextRandomBuffer();
        SDL_Surface *sourceSurface = getNextRandomSurface(buf, SDL_PIXELFORMAT_RGBA8888);

        SDL_Rect destRect;
        int location = (int)get_random_uint32();
        destRect.x = location % (width - 15 - 1);
        destRect.y = location % (height - 15 - 1);

        SDL_BlitSurface(sourceSurface, NULL, destSurface, &destRect);

        SDL_DestroySurface(sourceSurface);
        SDL_free(buf);
    }
    uint32_t out[4];
    hashSurfacePixels(destSurface, out);
    printf("Random to Random SVGA blit: %x, %x, %x, %x\n", out[0], out[1], out[2], out[3]);

    SDL_DestroySurface(destSurface);
    const int passes_x86_simd = out[0] == simd_result[0] && out[1] == simd_result[1] &&
            out[2] == simd_result[2] && out[3] == simd_result[3];
    const int passes_x86_classic = out[0] == classic_result[0] && out[1] == classic_result[1] &&
            out[2] == classic_result[2] && out[3] == classic_result[3];
    return passes_x86_simd || passes_x86_classic;
}
