#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "rotation.h"
#include "../Utils/image.h"


static double en_radians(double degres)
{
    return degres * M_PI / 180.0;
}

static SDL_Surface *image_nouvelle_surface(int hauteur, int largeur)
{
    return SDL_CreateRGBSurfaceWithFormat(0, largeur, hauteur, 32, SDL_PIXELFORMAT_RGBA32);
}

static SDL_Surface *image_rotate(SDL_Surface *image, double angle)
{
    angle = en_radians(-angle);
    double cosinus = cos(angle);
    double sinus = sin(angle);

    int nouvelle_hauteur = abs((int)(-image->w * sinus)) + abs((int)(image->h * cosinus));
    int nouvelle_largeur = abs((int)(image->w * cosinus)) + abs((int)(image->h * sinus));

    SDL_Surface *image_sortie = image_new(nouvelle_hauteur, nouvelle_largeur);

    for (int ligne = 0; ligne < nouvelle_hauteur; ligne++)
    {
        for (int colonne = 0; colonne < nouvelle_largeur; colonne++)
        {
            int nouvelle_ligne = (int)((ligne - nouvelle_hauteur / 2) * cosinus - (colonne - nouvelle_largeur / 2) * sinus);
            int nouvelle_colonne = (int)((colonne - nouvelle_largeur / 2) * cosinus + (ligne - nouvelle_hauteur / 2) * sinus);

            nouvelle_ligne += image->h / 2;
            nouvelle_colonne += image->w / 2;

            if (nouvelle_ligne >= 0 && nouvelle_ligne < image->h &&
                nouvelle_colonne >= 0 && nouvelle_colonne < image->w)
            {
                image_set_pixel(image_sortie, ligne, colonne, image_get_pixel(image, nouvelle_ligne, nouvelle_colonne));
            }
            else
            {
                image_set_pixel(image_sortie, ligne, colonne, SDL_MapRGB(image_sortie->format, 255, 255, 255));
            }
        }
    }

    return image_sortie;
}




// Rotation automatique 


static int compute_raycast_sum(SDL_Surface *image, int hauteur, double angle)
{
    angle = en_radians(angle);
    int rayon = fabs(cos(angle) * image->w);
    int largeur_depart = (image->w - rayon) / 2;

    int somme = 0;
    for (int largeur = largeur_depart; largeur < largeur_depart + rayon; largeur++)
    {
        int nouvelle_hauteur = hauteur + tan(angle) * largeur;
        if (nouvelle_hauteur >= 0 && nouvelle_hauteur < image->h && !is_white_pixel(image, nouvelle_hauteur, largeur))
            somme++;
    }
    return somme;
}

static double compute_variance(SDL_Surface *image, double angle)
{
    int hauteur_depart = image->h / 8;
    int hauteur_longueur = (7 * image->h) / 8;
    double coefficient = image->w / 15.0;

    double somme = 0.0, somme_carre = 0.0;
    for (int hauteur = hauteur_depart; hauteur < hauteur_depart + hauteur_longueur; hauteur += 4)
    {
        int valeur = compute_raycast_sum(image, hauteur, angle);
        somme += valeur - coefficient;
        somme_carre += (valeur - coefficient) * (valeur - coefficient);
    }
    return (somme_carre - (somme * somme) / hauteur_longueur) / (hauteur_longueur - 1);
}



static double find_skew_angle(SDL_Surface *image,
                              double borne_inferieure, double borne_superieure,
                              double precision)
{
    double angle_inclinaison = 0.0;
    double variance_max = 0.0;

    for (double angle = borne_inferieure; angle <= borne_superieure; angle += precision)
    {
        double variance = compute_variance(image, angle);
        if (variance > variance_max)
        {
            variance_max = variance;
            angle_inclinaison = angle;
        }
    }

    return angle_inclinaison;
}

SDL_Surface *image_deskew(SDL_Surface *image)
{
    double angle_inclinaison = find_skew_angle(image, -15.0, +15.0, 1.0);
    angle_inclinaison = find_skew_angle(image, angle_inclinaison - 3.0, angle_inclinaison + 3.0, 0.1);

    SDL_Surface *image_corrigee = image_rotate(image, angle_inclinaison);
    printf("Inclinaison détectée automatiquement : %.2f°\n", angle_inclinaison);

    SDL_SaveBMP(image_corrigee, "../output/image_auto_rotation.bmp");
    printf("Image sauvegardée : ../output/image_auto_rotation.bmp\n");

    return image_corrigee;
}







// Rotation manuelle par saisie d'un angle

void manual_rotation(const char *chemin_image)
{
    SDL_Surface *image = image_load(chemin_image);
    if (!image)
    {
        fprintf(stderr, "Erreur : impossible de charger l’image %s\n", chemin_image);
        return;
    }

    double angle;
    printf("Entrez l’angle de rotation (en degrés, positif = sens horaire) : ");
    if (scanf("%lf", &angle) != 1)
    {
        fprintf(stderr, "Entrée invalide.\n");
        SDL_FreeSurface(image);
        return;
    }

    SDL_Surface *image_tournee = image_rotate(image, angle);
    SDL_SaveBMP(image_tournee, "../output/image_rotation_manuelle.bmp");

    printf("Rotation manuelle appliquée : %.2f°\n", angle);
    printf("Image sauvegardée : ../output/image_rotation_manuelle.bmp\n");

    SDL_FreeSurface(image_tournee);
    SDL_FreeSurface(image);
}
