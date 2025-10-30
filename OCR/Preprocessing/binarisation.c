#include <stdlib.h>
#include <SDL2/SDL.h>
#include "binarisation.h"

// --- Fonctions internes pour accéder aux pixels ---
static Uint8* image_get_pixel_ref(SDL_Surface *image, int h, int w)
{
    int bpp = image->format->BytesPerPixel;
    Uint8 *pixels = image->pixels;
    return pixels + h * image->pitch + w * bpp;
}

static Uint32 image_get_pixel(SDL_Surface *image, int h, int w)
{
    Uint8 *p = image_get_pixel_ref(image, h, w);
    switch (image->format->BytesPerPixel)
    {
        case 1: return *p;
        case 2: return *(Uint16 *)p;
        case 3:
            return (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                ? p[0] << 16 | p[1] << 8 | p[2]
                : p[0] | p[1] << 8 | p[2] << 16;
        case 4: return *(Uint32 *)p;
        default: return 0;
    }
}

static void image_set_pixel(SDL_Surface *image, int h, int w, Uint32 pixel)
{
    Uint8 *p = image_get_pixel_ref(image, h, w);
    switch (image->format->BytesPerPixel)
    {
        case 1: *p = pixel; break;
        case 2: *(Uint16 *)p = pixel; break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            }
            else
            {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;
        case 4: *(Uint32 *)p = pixel; break;
        default: break;
    }
}

static int *compute_histogram(SDL_Surface *surface)
{
    int *histogramme = calloc(256, sizeof(int));
    if (!histogramme)
        return NULL;

    for (int ligne = 0; ligne < surface->h; ligne++)
    {
        for (int colonne = 0; colonne < surface->w; colonne++)
        {
            Uint8 rouge, vert, bleu;
            Uint32 pixel = image_get_pixel(surface, ligne, colonne);
            SDL_GetRGB(pixel, surface->format, &rouge, &vert, &bleu);
            histogramme[rouge]++;
        }
    }
    return histogramme;
}

static int compute_otsu_threshold(SDL_Surface *surface)
{
    int meilleur_seuil = 0;
    double variance_max = 0.0;
    int somme_totale = 0;
    int somme_arriere_plan = 0;
    int poids_arriere_plan = 0;
    int total_pixels = surface->h * surface->w;

    int *histogramme = compute_histogram(surface);
    if (!histogramme)
        return 127;

    for (int i = 0; i < 256; i++)
        somme_totale += i * histogramme[i];

    for (int i = 0; i < 256; i++)
    {
        poids_arriere_plan += histogramme[i];
        int poids_avant_plan = total_pixels - poids_arriere_plan;

        if (poids_arriere_plan == 0 || poids_avant_plan == 0)
            continue;

        somme_arriere_plan += i * histogramme[i];
        int somme_avant_plan = somme_totale - somme_arriere_plan;

        double poidsB = poids_arriere_plan;
        double poidsF = poids_avant_plan;
        double moyenneB = somme_arriere_plan / poidsB;
        double moyenneF = somme_avant_plan / poidsF;
        double difference = moyenneB - moyenneF;

        double variance = poidsB * poidsF * difference * difference;

        if (variance > variance_max)
        {
            variance_max = variance;
            meilleur_seuil = i;
        }
    }

    free(histogramme);
    return meilleur_seuil;
}

void image_binarize(SDL_Surface *surface)
{
    int seuil = compute_otsu_threshold(surface);

    for (int ligne = 0; ligne < surface->h; ligne++)
    {
        for (int colonne = 0; colonne < surface->w; colonne++)
        {
            Uint8 rouge, vert, bleu;
            Uint32 pixel = image_get_pixel(surface, ligne, colonne);
            SDL_GetRGB(pixel, surface->format, &rouge, &vert, &bleu);

            Uint32 nouveau_pixel =
                (rouge > seuil)
                    ? SDL_MapRGB(surface->format, 255, 255, 255)
                    : SDL_MapRGB(surface->format, 0, 0, 0);

            image_set_pixel(surface, ligne, colonne, nouveau_pixel);
        }
    }
}
