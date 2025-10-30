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

static const char *PATH_IMG_GRAYSCALE          = "../output/image_grayscale.bmp";
static const char *PATH_IMG_BINARIZE           = "../output/image_binarize.bmp";
static const char *PATH_IMG_AUTO_ROTATION      = "../output/image_auto_rotation.bmp";
static const char *PATH_IMG_NOISE_REDUC_AUTO   = "../output/image_noise_reduc_auto.bmp";
static const char *PATH_IMG_NOISE_REDUC_MANUAL = "../output/image_noise_reduc_manual.bmp";

static void ensure_output_folder(void)
{
    if (mkdir("../output", 0777) && errno != EEXIST)
        fprintf(stderr, "Impossible de créer le dossier ../output\n");
}

static void save_surface_compat(SDL_Surface *surface, const char *path)
{
    if (!surface)
    {
        fprintf(stderr, "Tentative de sauvegarde d'une surface NULL : %s\n", path);
        return;
    }

    SDL_Surface *compatible = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGB24, 0);
    if (!compatible)
    {
        fprintf(stderr, "Erreur conversion format avant sauvegarde : %s\n", SDL_GetError());
        return;
    }

    if (SDL_SaveBMP(compatible, path) != 0)
        fprintf(stderr, "Erreur sauvegarde BMP (%s) : %s\n", path, SDL_GetError());
    else
        printf("Image sauvegardée : %s (%dx%d)\n", path, compatible->w, compatible->h);

    SDL_FreeSurface(compatible);
}

// Fonction principale de prétraitement
void preprocessing(const char *image_path)
{
    ensure_output_folder();

    // Charger l’image
    SDL_Surface *image = image_load(image_path);
    if (!image)
    {
        fprintf(stderr, "Impossible de charger l’image : %s\n", image_path);
        return;
    }

    printf("Image chargée : %s (%dx%d, %dbpp)\n",
           image_path, image->w, image->h, image->format->BitsPerPixel);

    // Étape 1 : Niveaux de gris
    image_grayscale(image);
    save_surface_compat(image, PATH_IMG_GRAYSCALE);

    // Étape 2 : Binarisation
    image_binarize(image);
    save_surface_compat(image, PATH_IMG_BINARIZE);

    // Étape 3 : Correction automatique de l’inclinaison
    SDL_Surface *auto_rotated = image_deskew(image);
    if (!auto_rotated)
    {
        fprintf(stderr, "Erreur lors du deskew.\n");
        SDL_FreeSurface(image);
        return;
    }
    save_surface_compat(auto_rotated, PATH_IMG_AUTO_ROTATION);

    // Étape 4 : Réduction du bruit
    SDL_Surface *final_auto   = image_noise_reduction(auto_rotated);
    SDL_Surface *final_manual = image_noise_reduction(image);

    if (final_auto)
        save_surface_compat(final_auto, PATH_IMG_NOISE_REDUC_AUTO);
    else
        fprintf(stderr, "Échec réduction bruit (auto)\n");

    if (final_manual)
        save_surface_compat(final_manual, PATH_IMG_NOISE_REDUC_MANUAL);
    else
        fprintf(stderr, "Échec réduction bruit (manuel)\n");

    printf("Réduction du bruit terminée.\n");
    printf("   - Auto   : %s\n", PATH_IMG_NOISE_REDUC_AUTO);
    printf("   - Manuel : %s\n", PATH_IMG_NOISE_REDUC_MANUAL);

    SDL_FreeSurface(image);
    SDL_FreeSurface(auto_rotated);
    if (final_auto) SDL_FreeSurface(final_auto);
    if (final_manual) SDL_FreeSurface(final_manual);

    printf("Prétraitement terminé avec succès !\n");
    printf("Résultats disponibles dans le dossier ../output/\n");
}
