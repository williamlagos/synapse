#include<stdio.h>
#include<stdlib.h>
#include<SDL2/SDL.h>
#include<SDL2/SDL_image.h>

#define LOOP 1
#define IMG_PATH "/boot/oemsplash.png"

/* Transitory code, to be transformed into bash or python */

void centralize_img(SDL_Texture* img, SDL_Rect* texr, SDL_Window* win)
{
    int w, h, win_w, win_h;
    SDL_QueryTexture(img, NULL, NULL, &w, &h);
    SDL_GetWindowSize(win, &win_w, &win_h);
    texr->w = w; texr->h = h;
    texr->x = (win_w - w) / 2;
    texr->y = (win_h - h) / 2;
}

void start(int max, char** buffer)
{
    SDL_Window* win = NULL;
    SDL_Texture* img = NULL;
    SDL_Renderer* renderer = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) exit(EXIT_FAILURE);

    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN, &win, &renderer);
    img = IMG_LoadTexture(renderer, IMG_PATH);

    // Position the splash image to center, without resizing
    /* SDL_Rect rectangle;
    centralize_img(img,&rectangle,win); */

    while(LOOP) {
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) break;
            else if (e.type == SDL_KEYUP &&
                     e.key.keysym.sym == SDLK_ESCAPE) break;
        }
        SDL_RenderClear(renderer);
        // SDL_RenderCopy(renderer, img, NULL, &rectangle);
        SDL_RenderCopy(renderer, img, NULL, NULL);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(img);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

int main(int argc, char** argv)
{
    start(argc,argv);
    return EXIT_SUCCESS;
}
