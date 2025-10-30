#include "../Utils/image.h"
#include "preprocessing.h"

static const int DECALAGES_VOISINS[8][2] = {
    {-1, -1}, {-1, 0}, {-1, 1},
    {0, -1},           {0, 1},
    {1, -1},  {1, 0},  {1, 1}
};

SDL_Surface *image_noise_reduction(SDL_Surface *surface)
{
    SDL_Surface *image_nettoyee = image_new(surface->h, surface->w);

    for (int ligne = 0; ligne < surface->h; ligne++)
    {
        for (int colonne = 0; colonne < surface->w; colonne++)
        {
            int voisins_blancs = 0;

            for (int voisin = 0; voisin < 8; voisin++)
            {
                int voisin_ligne = ligne + DECALAGES_VOISINS[voisin][0];
                int voisin_colonne = colonne + DECALAGES_VOISINS[voisin][1];

                if (voisin_ligne >= 0 && voisin_ligne < surface->h &&
                    voisin_colonne >= 0 && voisin_colonne < surface->w)
                {
                    voisins_blancs += is_white_pixel(surface, voisin_ligne, voisin_colonne);
                }
            }

            Uint32 pixel_blanc = SDL_MapRGB(surface->format, 255, 255, 255);
            Uint32 pixel_noir = SDL_MapRGB(surface->format, 0, 0, 0);

            if (voisins_blancs > 4)
                image_set_pixel(image_nettoyee, ligne, colonne, pixel_blanc);
            else
                image_set_pixel(image_nettoyee, ligne, colonne, pixel_noir);
        }
    }

    return image_nettoyee;
}
