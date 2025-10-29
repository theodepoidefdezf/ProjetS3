#ifndef SOLVER_H
#define SOLVER_H

#define MAX_LIGNES 1000
#define MAX_COLONNES 1000
#define MAX_MOT 256

typedef struct {
    char mot[MAX_MOT];
    int trouve;
    int start_ligne;
    int start_colonne;
    int end_ligne;
    int end_colonne;
} MotTrouve;

int est_valide(int cur_ligne, int cur_colonne, int nb_lignes, int nb_colonnes);

int chercher_mot_direction(
    int nb_lignes,
    int nb_colonnes,
    char tableau[MAX_LIGNES][MAX_COLONNES],
    const char *mot,
    int cur_ligne,
    int cur_colonne,
    int d_ligne,
    int d_colonne,
    int *end_ligne,
    int *end_colonne
);

int lire_grille(
    const char *nom_fichier,
    char tableau[MAX_LIGNES][MAX_COLONNES],
    int *nb_lignes,
    int *nb_colonnes
);

void chercher_dans_grille(
    int nb_lignes,
    int nb_colonnes,
    char tableau[MAX_LIGNES][MAX_COLONNES],
    const char *mot_origine,
    MotTrouve *sortie
);

#endif



