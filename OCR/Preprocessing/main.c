#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "preprocessing.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <image_path>\n", argv[0]);
        return 1;
    }

    const char *image_path = argv[1];

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Erreur SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
    {
        fprintf(stderr, "Erreur IMG_Init: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    printf("Lancement du prétraitement automatique...\n");
    preprocessing(image_path);

    IMG_Quit();
    SDL_Quit();
    printf("\nProgramme terminé avec succès.\n");

    return 0;
}