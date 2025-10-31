#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "../Utils/image.h"
#include "rotation.h"

static double degres_vers_radians(double degres)
{
    return degres * M_PI / 180.0;
}

static SDL_Surface *creer_surface_vide(int hauteur, int largeur)
{
    return SDL_CreateRGBSurfaceWithFormat(0, largeur, hauteur, 32, SDL_PIXELFORMAT_RGBA32);
}

static SDL_Surface *faire_rotation(SDL_Surface *image, double angle)
{
    angle = degres_vers_radians(-angle);
    double cosinus = cos(angle);
    double sinus = sin(angle);

    int nouvelle_hauteur = fabs(-image->w * sinus) + fabs(image->h * cosinus);
    int nouvelle_largeur = fabs(image->w * cosinus) + fabs(image->h * sinus);

    SDL_Surface *resultat = image_new(nouvelle_hauteur, nouvelle_largeur);

    for (int y = 0; y < nouvelle_hauteur; y++)
    {
        for (int x = 0; x < nouvelle_largeur; x++)
        {
            int ny = (int)((y - nouvelle_hauteur / 2) * cosinus - (x - nouvelle_largeur / 2) * sinus);
            int nx = (int)((x - nouvelle_largeur / 2) * cosinus + (y - nouvelle_hauteur / 2) * sinus);

            ny += image->h / 2;
            nx += image->w / 2;

            if (ny >= 0 && ny < image->h && nx >= 0 && nx < image->w)
                image_set_pixel(resultat, y, x, image_get_pixel(image, ny, nx));
            else
                image_set_pixel(resultat, y, x, SDL_MapRGB(resultat->format, 255, 255, 255));
        }
    }
    return resultat;
}


static int somme_projection(SDL_Surface *image, int h, double angle)
{
    angle = degres_vers_radians(angle);
    int rayon = fabs(cos(angle) * image->w);
    int w_depart = (image->w - rayon) / 2;

    int somme = 0;
    for (int w = w_depart; w < w_depart + rayon; w++)
    {
        int nh = h + tan(angle) * w;
        if (nh >= 0 && nh < image->h && !is_white_pixel(image, nh, w))
            somme++;
    }
    return somme;
}

static double variance_projection(SDL_Surface *image, double angle)
{
    int h_debut = image->h / 8;
    int h_long = (7 * image->h) / 8;
    double facteur = image->w / 15.0;

    double somme = 0.0, somme_carre = 0.0;
    for (int h = h_debut; h < h_debut + h_long; h += 4)
    {
        int s = somme_projection(image, h, angle);
        somme += s - facteur;
        somme_carre += (s - facteur) * (s - facteur);
    }
    return (somme_carre - (somme * somme) / h_long) / (h_long - 1);
}

static double trouver_angle_inclinaison(SDL_Surface *image,
                                        double borne_inf, double borne_sup,
                                        double precision)
{
    double meilleur_angle = 0.0;
    double variance_max = 0.0;

    for (double angle = borne_inf; angle <= borne_sup; angle += precision)
    {
        double variance = variance_projection(image, angle);
        if (variance > variance_max)
        {
            variance_max = variance;
            meilleur_angle = angle;
        }
    }

    return meilleur_angle;
}

SDL_Surface *correction_inclinaison(SDL_Surface *image)
{
    double angle = trouver_angle_inclinaison(image, -15.0, +15.0, 1.0);
    angle = trouver_angle_inclinaison(image, angle - 3.0, angle + 3.0, 0.1);

    SDL_Surface *redressee = faire_rotation(image, angle);
    printf("Inclinaison détectée automatiquement : %.2f°\n", angle);

    SDL_SaveBMP(redressee, "../output/image_rotation_auto.bmp");
    printf("Image corrigée sauvegardée : ../output/image_rotation_auto.bmp\n");

    return redressee;
}


void manual_rotation(const char *chemin_image)
{
    SDL_Surface *image = image_load(chemin_image);
    if (!image)
    {
        fprintf(stderr, "Erreur : impossible de charger l’image %s\n", chemin_image);
        return;
    }

    double angle;
    printf("Entrez l’angle de rotation (°, positif = sens horaire) : ");
    if (scanf("%lf", &angle) != 1)
    {
        fprintf(stderr, "Entrée invalide.\n");
        SDL_FreeSurface(image);
        return;
    }

    SDL_Surface *tournee = faire_rotation(image, angle);
    SDL_SaveBMP(tournee, "../output/image_rotation_manuelle.bmp");

    printf("Rotation manuelle appliquée : %.2f°\n", angle);
    printf("Image sauvegardée : ../output/image_rotation_manuelle.bmp\n");

    SDL_FreeSurface(tournee);
    SDL_FreeSurface(image);
}
