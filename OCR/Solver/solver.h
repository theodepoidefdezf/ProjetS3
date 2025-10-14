#ifndef SOLVER_H
#define SOLVER_H

#define MAX_LIGNES 50
#define MAX_COLONNES 50
#define MAX_MOTS 100

typedef struct {
    char mot[100];
    int start_ligne;
    int start_colonne;
    int end_ligne;
    int end_colonne;
    int trouve;
} MotTrouve;

int est_valide(int ligne, int colonne, int lignes, int colonnes);

void solveur(int lignes, int colonnes, char grille[lignes][colonnes],
             char *liste_mots[], int nb_mots, MotTrouve resultats[]);

void afficher_coordonnees(MotTrouve *m, char *mot_recherche);

#endif

