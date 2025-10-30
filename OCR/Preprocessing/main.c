#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <sys/stat.h>
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
    const char *manual_output = "../output/image_rotation_manuelle.bmp";

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
    printf("Rotation manuelle : entrez un angle de rotation.\n");
    manual_rotation(image_path);
    struct stat buffer;
    if (stat(manual_output, &buffer) != 0)
    {
        fprintf(stderr, "L’image manuelle n’a pas été générée (%s).\n", manual_output);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    printf("\nLancement du prétraitement complet...\n");
    preprocessing(manual_output);
    IMG_Quit();
    SDL_Quit();
    printf("\nProgramme terminé avec succès.\n");
    return 0;
}


