#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "solver.h"

// 8 directions possibles (dx, dy)
int directions[8][2] = {
    {-1, -1}, {-1, 0}, {-1, 1},
    {0, -1},           {0, 1},
    {1, -1},  {1, 0},  {1, 1}
};
// Vérifie si la position est dans la grille
int est_valide(int ligne, int colonne, int lignes, int colonnes) {
    return (ligne >= 0 && ligne < lignes && colonne >= 0 && colonne < colonnes);
}

// Cherche un mot dans une direction donnée
int chercher_mot_direction(int lignes, int colonnes, char grille[lignes][colonnes],
                           char *mot, int ligne, int colonne, int dx, int dy,
                           int *end_ligne, int *end_colonne) {
    int longueur = strlen(mot);

    for (int i = 0; i < longueur; i++) {
        int nl = ligne + i * dx;
        int nc = colonne + i * dy;
        if (!est_valide(nl, nc, lignes, colonnes))
            return 0;
        if (grille[nl][nc] != mot[i])
            return 0;
    }

    *end_ligne = ligne + (longueur - 1) * dx;
    *end_colonne = colonne + (longueur - 1) * dy;
    return 1;
}

// Fonction principale : cherche tous les mots d’une liste dans la grille
void solveur(int lignes, int colonnes, char grille[lignes][colonnes],
             char *liste_mots[], int nb_mots, MotTrouve resultats[]) {

    for (int i = 0; i < nb_mots; i++) {
        char *mot = liste_mots[i];
        int trouve = 0;

        for (int ligne = 0; ligne < lignes && !trouve; ligne++) {
            for (int colonne = 0; colonne < colonnes && !trouve; colonne++) {
                for (int d = 0; d < 8 && !trouve; d++) {
                    int dx = directions[d][0];
                    int dy = directions[d][1];
                    int end_ligne, end_colonne;

                    // Sens normal
                    if (chercher_mot_direction(lignes, colonnes, grille, mot,
                                               ligne, colonne, dx, dy,
                                               &end_ligne, &end_colonne)) {
                        strcpy(resultats[i].mot, mot);
                        resultats[i].start_ligne = ligne;
                        resultats[i].start_colonne = colonne;
                        resultats[i].end_ligne = end_ligne;
                        resultats[i].end_colonne = end_colonne;
                        resultats[i].trouve = 1;
                        trouve = 1;
                        break;
                    }

                    // Sens inverse
                    char inverse[100];
                    int len = strlen(mot);
                    for (int k = 0; k < len; k++)
                        inverse[k] = mot[len - 1 - k];
                    inverse[len] = '\0';

                    if (chercher_mot_direction(lignes, colonnes, grille, inverse,
                                               ligne, colonne, dx, dy,
                                               &end_ligne, &end_colonne)) {
                        strcpy(resultats[i].mot, mot);
                        resultats[i].start_ligne = end_ligne;   // inversé
                        resultats[i].start_colonne = end_colonne;
                        resultats[i].end_ligne = ligne;
                        resultats[i].end_colonne = colonne;
                        resultats[i].trouve = 1;
                        trouve = 1;
                        break;
                    }
                }
            }
        }

        if (!trouve) {
            strcpy(resultats[i].mot, mot);
            resultats[i].trouve = 0;
        }
    }
}

// Affiche les coordonnées d’un mot trouvé
void afficher_coordonnees(MotTrouve *m, char *mot_recherche) {
    if (m->trouve)
        printf("(%d,%d)(%d,%d)\n",
               m->start_ligne, m->start_colonne,
               m->end_ligne, m->end_colonne);
    else
        printf("%s : mot introuvable\n", mot_recherche);
}

// Exemple d’utilisation
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage : ./solver MOT\n");
        return 1;
    }

    // === Grille de test ===
    char grille[5][5] = {
        {'C','A','P','S','S'},
        {'A','R','U','H','I'},
        {'R','A','A','S','P'},
        {'T','B','E','E','N'},
        {'I','E','A','K','S'}
    };

    // === Liste de mots connus ===
    char *liste_mots[] = {"SHABI", "RAT", "TOP", "AREA", "TAR"};
    int nb_mots = sizeof(liste_mots) / sizeof(liste_mots[0]);

    MotTrouve resultats[MAX_MOTS];

    // Recherche de tous les mots
    solveur(5, 5, grille, liste_mots, nb_mots, resultats);

    // Affiche seulement celui demandé
    char *mot_recherche = argv[1];
    int trouve = 0;
    for (int i = 0; i < nb_mots; i++) {
        if (strcmp(resultats[i].mot, mot_recherche) == 0) {
            afficher_coordonnees(&resultats[i], mot_recherche);
            trouve = 1;
            break;
        }
    }

    if (!trouve)
        printf("%s n’est pas dans la liste des mots connus.\n", mot_recherche);

    return 0;
}

