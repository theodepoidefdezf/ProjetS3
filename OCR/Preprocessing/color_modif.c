#include <stdio.h>
#include <SDL2/SDL.h>
#include "color_modif.h"
#include "../Utils/image.h"

void image_grayscale(SDL_Surface *surface)
{
    for (int ligne = 0; ligne < surface->h; ligne++)
    {
        for (int colonne = 0; colonne < surface->w; colonne++)
        {
            Uint8 rouge, vert, bleu;
            Uint32 pixel = image_get_pixel(surface, ligne, colonne);
            SDL_GetRGB(pixel, surface->format, &rouge, &vert, &bleu);

            Uint8 niveau_gris = 0.2126 * rouge + 0.7152 * vert + 0.0722 * bleu;

            Uint32 pixel_gris = SDL_MapRGB(surface->format, niveau_gris, niveau_gris, niveau_gris);
            image_set_pixel(surface, ligne, colonne, pixel_gris);
        }
    }
}
