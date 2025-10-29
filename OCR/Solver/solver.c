#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "solver.h"

int directions[8][2] = {
    {-1, -1}, {-1, 0}, {-1, 1},
    { 0, -1},          { 0, 1},
    { 1, -1}, { 1, 0}, { 1, 1}
};

int est_valide(int ligne, int colonne, int nb_lignes, int nb_colonnes)
{
    return (ligne >= 0 && ligne < nb_lignes && colonne >= 0 && colonne < nb_colonnes);
}

int chercher_mot_direction(
    int nb_lignes,
    int nb_colonnes,
    char tableau[MAX_LIGNES][MAX_COLONNES],
    const char *mot,
    int ligne,
    int colonne,
    int d_ligne,
    int d_colonne,
    int *end_ligne,
    int *end_colonne
)
{
    int longueur = strlen(mot);

    for (int i = 0; i < longueur; i++)
    {
        int cur_ligne = ligne + i * d_ligne;
        int cur_colonne = colonne + i * d_colonne;

        if (!est_valide(cur_ligne, cur_colonne, nb_lignes, nb_colonnes) || tableau[cur_ligne][cur_colonne] != mot[i])
        {
            return 0;
        }
    }

    *end_ligne = ligne + (longueur - 1) * d_ligne;
    *end_colonne = colonne + (longueur - 1) * d_colonne;

    return 1;
}

int lire_grille(
    const char *nom_fichier,
    char tableau[MAX_LIGNES][MAX_COLONNES],
    int *nb_lignes,
    int *nb_colonnes
)
{
    FILE *f = fopen(nom_fichier, "r");
    if (!f)
    {
        return 0;
    }

    char buffer[2048];
    int cpt_ligne = 0;
    int max_col = 0;

    while (fgets(buffer, sizeof(buffer), f) && cpt_ligne < MAX_LIGNES)
    {
        int len = strlen(buffer);

        if (len && buffer[len - 1] == '\n')
        {
            buffer[--len] = '\0';
        }

        if (len && buffer[len - 1] == '\r')
        {
            buffer[--len] = '\0';
        }

        char buffer_nettoye[MAX_COLONNES];
        int index_col = 0;

        for (int i = 0; i < len; i++)
        {
            if (buffer[i] != ' ' && buffer[i] != '\t')
            {
                buffer_nettoye[index_col++] = toupper((unsigned char) buffer[i]);
            }
        }

        if (index_col == 0)
        {
            continue;
        }

        if (index_col > max_col)
        {
            max_col = index_col;
        }

        for (int c = 0; c < index_col; c++)
        {
            tableau[cpt_ligne][c] = buffer_nettoye[c];
        }

        for (int c = index_col; c < MAX_COLONNES; c++)
        {
            tableau[cpt_ligne][c] = ' ';
        }

        cpt_ligne++;
    }

    fclose(f);

    if (cpt_ligne == 0)
    {
        return 0;
    }

    *nb_lignes = cpt_ligne;
    *nb_colonnes = max_col;

    return 1;
}

void chercher_dans_grille(
    int nb_lignes,
    int nb_colonnes,
    char tableau[MAX_LIGNES][MAX_COLONNES],
    const char *mot_origine,
    MotTrouve *sortie
)
{
    sortie->trouve = 0;

    strncpy(sortie->mot, mot_origine, MAX_MOT - 1);
    sortie->mot[MAX_MOT - 1] = '\0';

    char mot[MAX_MOT];
    strncpy(mot, mot_origine, MAX_MOT - 1);
    mot[MAX_MOT - 1] = '\0';

    int len = strlen(mot);

    char inverse[MAX_MOT];
    for (int i = 0; i < len; i++)
    {
        inverse[i] = mot[len - 1 - i];
    }
    inverse[len] = '\0';

    for (int ligne = 0; ligne < nb_lignes && !sortie->trouve; ligne++)
    {
        for (int colonne = 0; colonne < nb_colonnes && !sortie->trouve; colonne++)
        {
            for (int d = 0; d < 8 && !sortie->trouve; d++)
            {
                int d_ligne = directions[d][0];
                int d_colonne = directions[d][1];
                int end_ligne, end_colonne;

                if (chercher_mot_direction(nb_lignes, nb_colonnes, tableau, mot,
                                           ligne, colonne, d_ligne, d_colonne, &end_ligne, &end_colonne))
                {
                    sortie->start_ligne = ligne;
                    sortie->start_colonne = colonne;
                    sortie->end_ligne = end_ligne;
                    sortie->end_colonne = end_colonne;
                    sortie->trouve = 1;
                    break;
                }

                if (chercher_mot_direction(nb_lignes, nb_colonnes, tableau, inverse,
                                           ligne, colonne, d_ligne, d_colonne, &end_ligne, &end_colonne))
                {
                    sortie->start_ligne = end_ligne;
                    sortie->start_colonne = end_colonne;
                    sortie->end_ligne = ligne;
                    sortie->end_colonne = colonne;
                    sortie->trouve = 1;
                    break;
                }
            }
        }
    }
}



