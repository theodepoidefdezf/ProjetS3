#include "letter_extraction.h"
#include <stdlib.h>
#include <stdio.h> 

// taille sortie et lettre max
#define TAILLE_STANDARD 50
#define MAX_LETTERS_ESTIMATE 200

Image standardiser_et_creer_image(Image input_image, int x_start, int x_end, int y_start, int y_end)
{
    // création de la nouvelle image
    Image letter_img;
    letter_img.width = TAILLE_STANDARD;
    letter_img.height = TAILLE_STANDARD;
    
    // allocation memore 50*50
    letter_img.data = (unsigned char *)calloc(TAILLE_STANDARD * TAILLE_STANDARD, sizeof(unsigned char));

    if (letter_img.data == NULL) {
        // err alloc
        fprintf(stderr, "Erreur d'allocation pour une nouvelle image de lettre.\n");
        exit(EXIT_FAILURE);
    }

    // Calc taille  lettre & marges for centrer
    int largeur_caractere = x_end - x_start + 1;
    int hauteur_caractere = y_end - y_start + 1;
    int marge_x = (TAILLE_STANDARD - largeur_caractere) / 2;
    int marge_y = (TAILLE_STANDARD - hauteur_caractere) / 2;

    // ctrl c pixels de l'image to  l'image de dest
    for (int i = 0; i < hauteur_caractere; i++) {
        for (int j = 0; j < largeur_caractere; j++) {
            
            // Coordo image d'entréee 
            int source_y = y_start + i;
            int source_x = x_start + j;
            // Indice 1D source
            int source_idx = source_y * input_image.width + source_x;

            // Coordonnée dest  5050
            int dest_y = marge_y + i;
            int dest_x = marge_x + j;
            // Indice 1D dest 
            int dest_idx = dest_y * TAILLE_STANDARD + dest_x;

            // crtl c pixel 
            if (dest_y >= 0 && dest_y < TAILLE_STANDARD && dest_x >= 0 && dest_x < TAILLE_STANDARD) {
                letter_img.data[dest_idx] = input_image.data[source_idx];
            }
        }
    }

    return letter_img;
}

Letters extract_letters(Image input_image)
{
    printf("--- Début de l'extraction des lettres ---\n");

    Letters result;
    // alloc tab img letre 
    result.letters = (Image *)malloc(sizeof(Image) * MAX_LETTERS_ESTIMATE);
    result.count = 0;
    
    if (result.letters == NULL) {
        fprintf(stderr, "Erreur d'allocation pour le tableau de lettres.\n");
        exit(EXIT_FAILURE);
    }

    int H = input_image.height;
    int L = input_image.width;

    int y_start = 0;
    int en_cours_de_ligne = 0;

    // Parcour lignes
    for (int i = 0; i < H; i++) {
        int somme_pixels_fonces = 0;
        for (int j = 0; j < L; j++) {
            somme_pixels_fonces += input_image.data[i * L + j];  
        }

        if (somme_pixels_fonces > 0 && en_cours_de_ligne == 0) {
            y_start = i;
            en_cours_de_ligne = 1;
        }
        else if ((somme_pixels_fonces == 0 || i == H - 1) && en_cours_de_ligne == 1) {
            int y_end = (somme_pixels_fonces == 0) ? i - 1 : i;
            
            // Ligne trouv so parcour des colonnes 
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
                    // char trouve 
                    printf("Caractere trouve : (x: %d->%d, y: %d->%d)\n", x_start, x_end, y_start, y_end);

                    // create img 50x5O
                    Image nouvelle_lettre = standardiser_et_creer_image(input_image, x_start, x_end, y_start, y_end);
                    
                    // add to result 
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
