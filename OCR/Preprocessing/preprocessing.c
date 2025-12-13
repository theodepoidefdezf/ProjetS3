#include <stdio.h>
#include <SDL2/SDL.h>
#include <sys/stat.h>
#include <errno.h>
#include "preprocessing.h"
#include "../Utils/image.h"
#include "binarisation.h"
#include "color_modif.h"
#include "rotation.h"
#include "cleaner.h"

static const char *PATH_IMG_GRAYSCALE        = "../output/image_grayscale.bmp";
static const char *PATH_IMG_BINARIZE         = "../output/image_binarize.bmp";
static const char *PATH_IMG_AUTO_ROTATION    = "../output/image_auto_rotation.bmp";
static const char *PATH_IMG_NOISE_REDUC_AUTO = "../output/image_noise_reduc_auto.bmp";
static const char *PATH_IMG_NOISE_REDUC_MAN  = "../output/image_noise_reduc_manual.bmp";

static void ensure_output_folder(void)
{
    if (mkdir("../output", 0777) && errno != EEXIST)
        fprintf(stderr, "Impossible de créer le dossier ../output\n");
}

static void take(SDL_Surface *surface, const char *path)
{
    if (!surface)
    {
        fprintf(stderr, "Surface NULL non sauvegardée : %s\n", path);
        return;
    }

    SDL_Surface *bmp_surface = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGB24, 0);
    if (!bmp_surface)
    {
        fprintf(stderr, "Conversion format échouée avant sauvegarde : %s\n", SDL_GetError());
        return;
    }

    if (SDL_SaveBMP(bmp_surface, path) != 0)
        fprintf(stderr, "Erreur sauvegarde BMP (%s): %s\n", path, SDL_GetError());
    else
        printf("Image sauvegardée : %s\n", path);

    SDL_FreeSurface(bmp_surface);
}

void preprocessing(const char *image_path)
{
    ensure_output_folder();

    SDL_Surface *src = image_load(image_path);
    if (!src)
    {
        fprintf(stderr, "Impossible de charger : %s\n", image_path);
        return;
    }

    printf("Prétraitement de : %s\n", image_path);

    // 1. Grayscale
    SDL_Surface *grayscale = SDL_ConvertSurface(src, src->format, 0);
    conversion(grayscale);
    take(grayscale, PATH_IMG_GRAYSCALE);

    // 2. Binarisation
    SDL_Surface *binarized = SDL_ConvertSurface(grayscale, grayscale->format, 0);
    conversion_bina(binarized);
    take(binarized, PATH_IMG_BINARIZE);

    // 3. Rotation automatique
    SDL_Surface *auto_rotated = correction_inclinaison(binarized);
    take(auto_rotated, PATH_IMG_AUTO_ROTATION);

    // 4. Réduction du bruit sur l'image auto-rotée
    SDL_Surface *noise_auto = reduire_bruit(auto_rotated);
    take(noise_auto, PATH_IMG_NOISE_REDUC_AUTO);

    // 5. Réduction du bruit sur l'image binarisée (sans rotation)
    SDL_Surface *noise_manual = reduire_bruit(binarized);
    take(noise_manual, PATH_IMG_NOISE_REDUC_MAN);

    printf("\nToutes les images ont été générées dans ../output :\n");
    printf(" - %s\n", PATH_IMG_GRAYSCALE);
    printf(" - %s\n", PATH_IMG_BINARIZE);
    printf(" - %s\n", PATH_IMG_AUTO_ROTATION);
    printf(" - %s\n", PATH_IMG_NOISE_REDUC_AUTO);
    printf(" - %s\n", PATH_IMG_NOISE_REDUC_MAN);

    SDL_FreeSurface(src);
    SDL_FreeSurface(grayscale);
    SDL_FreeSurface(binarized);
    SDL_FreeSurface(auto_rotated);
    SDL_FreeSurface(noise_auto);
    SDL_FreeSurface(noise_manual);
}