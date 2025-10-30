#include "image.h"
#include <err.h>
#include <SDL2/SDL_image.h>


// Chargement et création d’images
SDL_Surface *image_load(const char *chemin)
{
    SDL_Surface *image = IMG_Load(chemin);
    if (!image)
        errx(3, "Impossible de charger %s : %s", chemin, IMG_GetError());
    return image;
}

SDL_Surface *image_new(int hauteur, int largeur)
{
    return SDL_CreateRGBSurface(0, largeur, hauteur, 32, 0, 0, 0, 0);
}

static Uint8* image_get_pixel_ref(SDL_Surface *image, int ligne, int colonne)
{
    int octets_par_pixel = image->format->BytesPerPixel;
    Uint8 *pixels = image->pixels;
    return pixels + ligne * image->pitch + colonne * octets_par_pixel;
}

Uint32 image_get_pixel(SDL_Surface *image, int ligne, int colonne)
{
    Uint8 *p = image_get_pixel_ref(image, ligne, colonne);
    switch (image->format->BytesPerPixel)
    {
        case 1:
            return *p;
        case 2:
            return *(Uint16*)p;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;
        case 4:
            return *(Uint32*)p;
        default:
            return 0;
    }
}

void image_set_pixel(SDL_Surface *image, int ligne, int colonne, Uint32 pixel)
{
    Uint8 *p = image_get_pixel_ref(image, ligne, colonne);
    switch (image->format->BytesPerPixel)
    {
        case 1:
            *p = pixel;
            break;
        case 2:
            *(Uint16*)p = pixel;
            break;
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
        case 4:
            *(Uint32*)p = pixel;
            break;
    }
}

bool is_white_pixel(SDL_Surface *image, int ligne, int colonne)
{
    Uint8 rouge, vert, bleu;
    Uint32 pixel = image_get_pixel(image, ligne, colonne);
    SDL_GetRGB(pixel, image->format, &rouge, &vert, &bleu);
    return rouge == 255 && vert == 255 && bleu == 255;
}


bool is_blank_line(SDL_Surface *texte, int ligne)
{
    for (int colonne = 0; colonne < texte->w; colonne++)
        if (!is_white_pixel(texte, ligne, colonne))
            return false;
    return true;
}

bool is_blank_column(SDL_Surface *ligne, int colonne)
{
    for (int hauteur = 0; hauteur < ligne->h; hauteur++)
        if (!is_white_pixel(ligne, hauteur, colonne))
            return false;
    return true;
}

void draw_line(SDL_Surface *image, Uint32 pixel, int ligne, int colonne_debut, int colonne_fin)
{
    for (int colonne = colonne_debut; colonne <= colonne_fin; colonne++)
        image_set_pixel(image, ligne, colonne, pixel);
}

void draw_column(SDL_Surface *image, Uint32 pixel, int colonne, int ligne_debut, int ligne_fin)
{
    for (int ligne = ligne_debut; ligne <= ligne_fin; ligne++)
        image_set_pixel(image, ligne, colonne, pixel);
}



