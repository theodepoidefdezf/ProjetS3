#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "solver.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <fichier_grille> <mot>\n", argv[0]);
        return 1;
    }

    const char *nom_fichier = argv[1];
    char mot_raw[MAX_MOT];
    int i;

    for (i = 0; i < MAX_MOT - 1 && argv[2][i]; i++)
    {
        mot_raw[i] = toupper((unsigned char) argv[2][i]);
    }
    mot_raw[i] = '\0';

    static char grille[MAX_LIGNES][MAX_COLONNES];
    int lignes = 0;
    int colonnes = 0;

    if (!lire_grille(nom_fichier, grille, &lignes, &colonnes))
    {
        fprintf(stderr, "Erreur: impossible de lire la grille depuis '%s'\n", nom_fichier);
        return 1;
    }

    MotTrouve resultat;
    chercher_dans_grille(lignes, colonnes, grille, mot_raw, &resultat);

    if (resultat.trouve)
    {
        printf("(%d,%d)(%d,%d)\n",
               resultat.start_colonne, resultat.start_ligne,
               resultat.end_colonne, resultat.end_ligne);
    }
    else
    {
        printf("Not Found\n");
    }

    return 0;
}

