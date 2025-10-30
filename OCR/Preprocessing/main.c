#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "preprocessing.h"
#include "rotation.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <image_path>\n", argv[0]);
        return 1;
    }

    const char *image_path = argv[1];

    // Initialisation de SDL et SDL_image
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        fprintf(stderr, "Erreur SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
    {
        fprintf(stderr, "Erreur IMG_Init: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    printf("ROTATION MANUELLE\n");
    manual_rotation(image_path);

    printf("\nPRÉTRAITEMENT COMPLET\n");
    preprocessing("../output/image_manual_rotation.bmp");

    IMG_Quit();
    SDL_Quit();

    printf("\nProgramme terminé avec succès.\n");
    return 0;
}
