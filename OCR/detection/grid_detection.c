#include "grid_detection.h"
#include <stdlib.h>
#include <stdio.h>

#define TAILLE_STANDARD 50 // Taille de sortie pour les lettres
#define MAX_LETTERS_ESTIMATE 200 // Estimation

// --- Structure interne ---
typedef struct {
    int x_start, x_end, y_start, y_end;
} BoundingBox;

// --- Fonctions HELPER (locales) ---

// Helper 1: Crée une nouvelle image à partir d'une 'BoundingBox'
Image create_sub_image(Image img, BoundingBox box) {
    Image sub_img;
    sub_img.width = box.x_end - box.x_start + 1;
    sub_img.height = box.y_end - box.y_start + 1;
    sub_img.data = (unsigned char *)calloc(sub_img.width * sub_img.height, sizeof(unsigned char));

    if (sub_img.data == NULL) {
        fprintf(stderr, "Erreur d'allocation pour la sous-image.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < sub_img.height; i++) {
        for (int j = 0; j < sub_img.width; j++) {
            int source_y = box.y_start + i;
            int source_x = box.x_start + j;
            int source_idx = source_y * img.width + source_x;
            int dest_idx = i * sub_img.width + j;
            
            sub_img.data[dest_idx] = img.data[source_idx];
        }
    }
    return sub_img;
}

// Helper 2: Trouve le premier 'blob' de pixels dans une image
BoundingBox find_first_blob(Image img) {
    BoundingBox box = {0, 0, 0, 0};
    int H = img.height;
    int L = img.width;
    int found_y_start = 0;

    // 1. Trouver le Y_START
    for (int i = 0; i < H; i++) {
        int somme_ligne = 0;
        for (int j = 0; j < L; j++) somme_ligne += img.data[i * L + j];
        if (somme_ligne > 0) {
            box.y_start = i;
            found_y_start = 1;
            break;
        }
    }
    if (!found_y_start) return box; // Image vide

    // 2. Trouver le Y_END
    for (int i = box.y_start; i < H; i++) {
        int somme_ligne = 0;
        for (int j = 0; j < L; j++) somme_ligne += img.data[i * L + j];
        if (somme_ligne == 0) {
            box.y_end = i - 1;
            break;
        }
        if (i == H - 1) box.y_end = i;
    }
    
    // 3. Trouver le X_START (dans la zone Y)
    for (int j = 0; j < L; j++) {
        int somme_col = 0;
        for (int i = box.y_start; i <= box.y_end; i++) somme_col += img.data[i * L + j];
        if (somme_col > 0) {
            box.x_start = j;
            break;
        }
    }

    // 4. Trouver le X_END (dans la zone Y)
    for (int j = box.x_start; j < L; j++) {
        int somme_col = 0;
        for (int i = box.y_start; i <= box.y_end; i++) somme_col += img.data[i * L + j];
        if (somme_col == 0) {
            box.x_end = j - 1;
            break;
        }
        if (j == L - 1) box.x_end = j;
    }
    return box;
}

// Helper 3: Standardise une lettre (copié de letter_extraction.c)
Image standardiser_lettre(Image input_image, BoundingBox box) {
    Image letter_img;
    letter_img.width = TAILLE_STANDARD;
    letter_img.height = TAILLE_STANDARD;
    letter_img.data = (unsigned char *)calloc(TAILLE_STANDARD * TAILLE_STANDARD, sizeof(unsigned char));

    int largeur_caractere = box.x_end - box.x_start + 1;
    int hauteur_caractere = box.y_end - box.y_start + 1;
    int marge_x = (TAILLE_STANDARD - largeur_caractere) / 2;
    int marge_y = (TAILLE_STANDARD - hauteur_caractere) / 2;

    for (int i = 0; i < hauteur_caractere; i++) {
        for (int j = 0; j < largeur_caractere; j++) {
            int source_y = box.y_start + i;
            int source_x = box.x_start + j;
            int source_idx = source_y * input_image.width + source_x;
            int dest_y = marge_y + i;
            int dest_x = marge_x + j;
            int dest_idx = dest_y * TAILLE_STANDARD + dest_x;

            if (dest_y >= 0 && dest_y < TAILLE_STANDARD && dest_x >= 0 && dest_x < TAILLE_STANDARD) {
                letter_img.data[dest_idx] = input_image.data[source_idx];
            }
        }
    }
    return letter_img;
}


/* --- IMPLÉMENTATION DES FONCTIONS DU .H --- */

/*
 * TÂCHE 1 : ISOLER LA GRILLE
 */
Image isolate_grid_image(Image img) {
    printf("Recherche de la zone 'Grille'...\n");
    BoundingBox grid_box = find_first_blob(img);
    printf("Grille trouvée : (x: %d->%d, y: %d->%d)\n", grid_box.x_start, grid_box.x_end, grid_box.y_start, grid_box.y_end);
    
    // On crée la sous-image AVANT d'effacer
    Image grid_sub_image = create_sub_image(img, grid_box);

    // On efface la zone trouvée de l'image 'img' pour la prochaine recherche
    for(int i = grid_box.y_start; i <= grid_box.y_end; i++) {
        for(int j = grid_box.x_start; j <= grid_box.x_end; j++) {
            img.data[i * img.width + j] = 0; // Met à 0 (fond)
        }
    }
    
    return grid_sub_image;
}

/*
 * TÂCHE 1 : ISOLER LES MOTS
 */
Image isolate_word_list_image(Image img) {
    printf("Recherche de la zone 'Mots'...\n");
    // Puisque isolate_grid_image a "effacé" la grille de l'image 'img',
    // find_first_blob trouvera le bloc suivant (les mots).
    BoundingBox words_box = find_first_blob(img);
    printf("Mots trouvés : (x: %d->%d, y: %d->%d)\n", words_box.x_start, words_box.x_end, words_box.y_start, words_box.y_end);
    
    return create_sub_image(img, words_box);
}


/*
 * TÂCHE 3 : EXTRAIRE LETTRES DE LA GRILLE
 * (C'est la même logique que letter_extraction.c)
 */
Letters extract_grid_letters(Image grid_image) {
    printf("--- Début de l'extraction des lettres de la GRILLE ---\n");
    Letters result;
    result.letters = (Image *)malloc(sizeof(Image) * MAX_LETTERS_ESTIMATE);
    result.count = 0;
    if (result.letters == NULL) exit(EXIT_FAILURE);

    int H = grid_image.height;
    int L = grid_image.width;
    int y_start = 0;
    int en_cours_de_ligne = 0;

    // Étape A : Parcourir les lignes
    for (int i = 0; i < H; i++) {
        int somme_pixels_fonces = 0;
        for (int j = 0; j < L; j++) somme_pixels_fonces += grid_image.data[i * L + j];

        if (somme_pixels_fonces > 0 && en_cours_de_ligne == 0) {
            y_start = i;
            en_cours_de_ligne = 1;
        }
        else if ((somme_pixels_fonces == 0 || i == H - 1) && en_cours_de_ligne == 1) {
            int y_end = (somme_pixels_fonces == 0) ? i - 1 : i;
            
            // Étape B : Parcourir les colonnes
            int x_start = 0;
            int en_cours_de_caractere = 0;
            
            for (int j = 0; j < L; j++) {
                int somme_pixels_colonne = 0;
                for (int k = y_start; k <= y_end; k++) somme_pixels_colonne += grid_image.data[k * L + j];

                if (somme_pixels_colonne > 0 && en_cours_de_caractere == 0) {
                    x_start = j;
                    en_cours_de_caractere = 1;
                }
                else if ((somme_pixels_colonne == 0 || j == L - 1) && en_cours_de_caractere == 1) {
                    int x_end = (somme_pixels_colonne == 0) ? j - 1 : j;
                    
                    // Étape C : Caractère trouvé !
                    BoundingBox box = {x_start, x_end, y_start, y_end};
                    
                    // On ne garde que les "vraies" lettres, pas les lignes de la grille
                    // On suppose qu'une lettre fait plus de 5x5 pixels (à ajuster)
                    if (box.x_end - box.x_start > 5 && box.y_end - box.y_start > 5) {
                         Image nouvelle_lettre = standardiser_lettre(grid_image, box);
                        
                        if (result.count < MAX_LETTERS_ESTIMATE) {
                            result.letters[result.count] = nouvelle_lettre;
                            result.count++;
                        }
                    }
                    en_cours_de_caractere = 0;
                }
            }
            en_cours_de_ligne = 0;
        }
    }
    printf("-> Extraction GRILLE terminee. %d lettres trouvees.\n", result.count);
    return result;
}
