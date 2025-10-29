#include "letter_extraction.h"
#include <stdlib.h> // Pour malloc
#include <stdio.h>  // Pour les printf de debug

// Définir la taille standard de sortie pour Théo
#define TAILLE_STANDARD 50
// Estimer un nombre max de lettres pour la première allocation
#define MAX_LETTERS_ESTIMATE 200

/**
 * @brief Crée une nouvelle image 50x50 et y copie la lettre extraite.
 *
 * C'est la fonction qui crée la matrice standardisée pour Théo.
 * Elle alloue la mémoire pour la nouvelle image.
 *
 * @param input_image L'image source (liste de mots).
 * @param x_start Coordonnée x de début de la lettre.
 * @param x_end Coordonnée x de fin de la lettre.
 * @param y_start Coordonnée y de début de la lettre.
 * @param y_end Coordonnée y de fin de la lettre.
 * @return Une nouvelle structure 'Image' (50x50) allouée.
 */
Image standardiser_et_creer_image(Image input_image, int x_start, int x_end, int y_start, int y_end)
{
    // 1. Créer la nouvelle image de sortie
    Image letter_img;
    letter_img.width = TAILLE_STANDARD;
    letter_img.height = TAILLE_STANDARD;
    
    // 2. Allouer la mémoire pour ses données (un tableau 1D de 50*50
    letter_img.data = (unsigned char *)calloc(TAILLE_STANDARD * TAILLE_STANDARD, sizeof(unsigned char));

    if (letter_img.data == NULL) {
        // Gérer l'échec d'allocation
        fprintf(stderr, "Erreur d'allocation pour une nouvelle image de lettre.\n");
        exit(EXIT_FAILURE);
    }

    // 3. Calculer la taille de la lettre et les marges pour la centrer
    int largeur_caractere = x_end - x_start + 1;
    int hauteur_caractere = y_end - y_start + 1;
    int marge_x = (TAILLE_STANDARD - largeur_caractere) / 2;
    int marge_y = (TAILLE_STANDARD - hauteur_caractere) / 2;

    // 4. Copier les pixels de l'image source vers l'image de destination
    for (int i = 0; i < hauteur_caractere; i++) {
        for (int j = 0; j < largeur_caractere; j++) {
            
            // Coordonnée source (sur l'image d'entrée)
            int source_y = y_start + i;
            int source_x = x_start + j;
            // Indice 1D de la source
            int source_idx = source_y * input_image.width + source_x;

            // Coordonnée destination (sur l'image 50x50)
            int dest_y = marge_y + i;
            int dest_x = marge_x + j;
            // Indice 1D de la destination
            int dest_idx = dest_y * TAILLE_STANDARD + dest_x;

            // Copier le pixel (après vérification des limites)
            if (dest_y >= 0 && dest_y < TAILLE_STANDARD && dest_x >= 0 && dest_x < TAILLE_STANDARD) {
                letter_img.data[dest_idx] = input_image.data[source_idx];
            }
        }
    }

    return letter_img;
}


/**
 * Fonction principale (déclarée dans le .h)
 */
Letters extract_letters(Image input_image)
{
    printf("--- Début de l'extraction des lettres ---\n");

    // 1. Initialiser la structure 'Letters' qu'on va renvoyer
    Letters result;
    // Allouer un tableau pour stocker les 'Image' des lettres
    result.letters = (Image *)malloc(sizeof(Image) * MAX_LETTERS_ESTIMATE);
    result.count = 0;
    
    if (result.letters == NULL) {
        fprintf(stderr, "Erreur d'allocation pour le tableau de lettres.\n");
        exit(EXIT_FAILURE);
    }

    int H = input_image.height;
    int L = input_image.width;

    // --- LOGIQUE DE PROJECTION (adaptée au tableau 1D) ---

    int y_start = 0;
    int en_cours_de_ligne = 0;

    // Étape A : Parcourir les lignes (Projection Horizontale)
    for (int i = 0; i < H; i++) {
        int somme_pixels_fonces = 0;
        for (int j = 0; j < L; j++) {
            somme_pixels_fonces += input_image.data[i * L + j]; // Formule 1D
        }

        if (somme_pixels_fonces > 0 && en_cours_de_ligne == 0) {
            y_start = i;
            en_cours_de_ligne = 1;
        }
        else if ((somme_pixels_fonces == 0 || i == H - 1) && en_cours_de_ligne == 1) {
            int y_end = (somme_pixels_fonces == 0) ? i - 1 : i;
            
            // Étape B : Ligne trouvée, parcourir les colonnes (Projection Verticale)
            int x_start = 0;
            int en_cours_de_caractere = 0;
            
            for (int j = 0; j < L; j++) {
                int somme_pixels_colonne = 0;
                for (int k = y_start; k <= y_end; k++) {
                    somme_pixels_colonne += input_image.data[k * L + j]; // Formule 1D
                }

                if (somme_pixels_colonne > 0 && en_cours_de_caractere == 0) {
                    x_start = j;
                    en_cours_de_caractere = 1;
                }
                else if ((somme_pixels_colonne == 0 || j == L - 1) && en_cours_de_caractere == 1) {
                    int x_end = (somme_pixels_colonne == 0) ? j - 1 : j;
                    
                    // Étape C : Caractère trouvé !
                    printf("Caractere trouve : (x: %d->%d, y: %d->%d)\n", x_start, x_end, y_start, y_end);

                    // Créer l'image 50x50 standardisée
                    Image nouvelle_lettre = standardiser_et_creer_image(input_image, x_start, x_end, y_start, y_end);
                    
                    // L'ajouter au résultat (en vérifiant qu'on ne déborde pas)
                    if (result.count < MAX_LETTERS_ESTIMATE) {
                        result.letters[result.count] = nouvelle_lettre;
                        result.count++;
                    }

                    en_cours_de_caractere = 0;
                }
            }
            en_cours_de_ligne = 0;
        }
    }

    printf("--- Extraction terminee. %d lettres trouvees.---\n", result.count);
    return result;
}
