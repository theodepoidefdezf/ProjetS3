include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "solver.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <fichier_grille> <fichier_mots>\n", argv[0]);
        return 1;
    }

    const char *fichier_grille = argv[1];
    const char *fichier_mots = argv[2];

    static char grille[MAX_LIGNES][MAX_COLONNES];
    int lignes = 0;
    int colonnes = 0;

    if (!lire_grille(fichier_grille, grille, &lignes, &colonnes))
    {
        fprintf(stderr, "Erreur: impossible de lire la grille depuis '%s'\n",
                fichier_grille);
        return 1;
    }

    FILE *fm = fopen(fichier_mots, "r");
    if (!fm)
    {
        fprintf(stderr, "Erreur: impossible d'ouvrir le fichier mots '%s'\n",
                fichier_mots);
        return 1;
    }

    FILE *fc = fopen("coordonnees", "w");
    if (!fc)
    {
        fprintf(stderr, "Erreur: impossible de créer le fichier coordonnees.txt\n");
        fclose(fm);
        return 1;
    }

    char buffer[MAX_MOT];

    while (fgets(buffer, sizeof(buffer), fm))
    {
        int len = strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r'))
            buffer[--len] = '\0';

        if (len == 0)
            continue;

        char mot[MAX_MOT];
        int i;
        for (i = 0; i < MAX_MOT - 1 && buffer[i]; i++)
            mot[i] = toupper((unsigned char)buffer[i]);
        mot[i] = '\0';

        MotTrouve resultat;
        chercher_dans_grille(lignes, colonnes, grille, mot, &resultat);

        if (resultat.trouve)
        {
            printf("%s: (%d,%d)(%d,%d)\n",
                   mot,
                   resultat.start_colonne, resultat.start_ligne,
                   resultat.end_colonne, resultat.end_ligne);

            fprintf(fc, "%s: (%d,%d)(%d,%d)\n",
                    mot,
                    resultat.start_colonne, resultat.start_ligne,
                    resultat.end_colonne, resultat.end_ligne);
        }
        else
        {
            printf("%s: Not Found\n", mot);
            fprintf(fc, "%s: Not Found\n", mot);
        }
    }

    fclose(fm);
    fclose(fc);

    return 0;
}
