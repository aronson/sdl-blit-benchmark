#include <SDL3/SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

float calculateFPS(Uint32 frameStart, Uint32 frameEnd) {
    float frameTime = (float)(frameEnd - frameStart);
    if (frameTime > 0) {
        return 1000.0f / frameTime;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    const int HEIGHT = 800;
    const int WIDTH = 600;
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("White Noise",
                                          HEIGHT, WIDTH, 0);

    SDL_Surface* screenSurface = SDL_GetWindowSurface(window);

    srand(time(NULL));

    int quit = 0;
    SDL_Event event;

    Uint32 frameStart, frameEnd;
    float fps;
    int frameCounter = 0;
    Uint32 titleUpdateTimer = SDL_GetTicks();

    while (!quit) {

        frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = 1;
            }
        }


        for (int i = 0; i < 25000; ++i) {
            int x = rand() % HEIGHT - 17;
            int y = rand() % WIDTH - 17;
            int alpha = rand() % 256;

            SDL_Surface* whiteRect = SDL_CreateSurface(16, 16, SDL_PIXELFORMAT_BGRA8888);
            SDL_FillSurfaceRect(whiteRect, NULL, SDL_MapRGBA(whiteRect->format, 255, 255, 255, alpha));

            SDL_Rect dstrect;
            dstrect.x = x;
            dstrect.y = y;
            SDL_BlitSurface(whiteRect, NULL, screenSurface, &dstrect);

            SDL_DestroySurface(whiteRect);
        }

        SDL_UpdateWindowSurface(window);
        frameEnd = SDL_GetTicks();
        fps = calculateFPS(frameStart, frameEnd);
        frameCounter++;

        if (SDL_GetTicks() - titleUpdateTimer > 500) {
            char title[64];
            snprintf(title, sizeof(title), "White Noise - FPS: %.2f", fps);
            SDL_SetWindowTitle(window, title);
            printf("FPS: %.2f\n", fps);
            titleUpdateTimer = SDL_GetTicks();
            frameCounter = 0;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
