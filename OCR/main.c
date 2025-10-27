#include <stdio.h>
#include <stdlib.h>

/* * INCLUDES DE TOUS LES MODULES
 */
#include "UTILS/structs.h"
#include "Preprocessing/image_loader.h"  // (Nom à confirmer avec Shabi)
#include "Preprocessing/preprocessing.h" // (Nom à confirmer avec Shabi)
#include "detection/grid_detection.h"    // <-- TOI
#include "detection/letter_extraction.h" // <-- TOI
#include "ocr/recognition.h"             // (Nom à confirmer avec Théo)
#include "Solver/solver.h"               // (Nom à confirmer)


/*
 * ==========================================================
 * FONCTIONS DES AUTRES MODULES (NOMS A CONFIRMER)
 * ==========================================================
 */

// --- Module: Preprocessing (Shabi) ---
// Image load_image(const char *path);
// Image binarize(Image img);
// void free_image(Image img);
// Image copy_image(Image img); // Important pour ne pas détruire l'originale

// --- Module: OCR (Théo) ---
// WordList recognize_letters(Letters letters_to_check, Network net);

// --- Module: Solver ---
// Grid solve_grid(Image grid_image, WordList words);

// --- Module: Cleanup ---
void free_letters(Letters letters) {
    for (int i = 0; i < letters.count; i++) {
        // Supposant que free_image libère img.data
        // free_image(letters.letters[i]);
        free(letters.letters[i].data); // Libération manuelle
    }
    free(letters.letters);
}


/*
 * ==========================================================
 * LE CHEF D'ORCHESTRE
 * ==========================================================
 */
int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <chemin_vers_image>\n", argv[0]);
        return 1;
    }

    printf("--- Lancement du programme OCR sur %s ---\n", argv[1]);

    /*
     * ÉTAPE 1 : PREPROCESSING (Shabi)
     * (Les noms de fonctions sont inventés)
     */
    printf("[1/5] Chargement et binarisation...\n");
    // Image img_originale = load_image(argv[1]);
    // Image img_binarisee = binarize(img_originale);
    // free_image(img_originale); 

    // --- SIMULATION EN ATTENDANT LE CODE DE SHABI ---
    // A REMPLACER PAR LES VRAIS APPELS
    Image img_binarisee;
    img_binarisee.width = 100; // exemple
    img_binarisee.height = 100; // exemple
    img_binarisee.data = (unsigned char *)calloc(100 * 100, sizeof(unsigned char));
    printf("-> (SIMULATION) Image 100x100 créée.\n");
    // --- FIN SIMULATION ---


    /*
     * ÉTAPE 2 : DETECTION (Toi)
     */
    printf("[2/5] Détection des zones (Grille et Mots)...\n");
    
    // On passe une copie pour ne pas détruire l'originale
    // Image copie_pour_detection = copy_image(img_binarisee); 
    
    // --- SIMULATION (on passe l'originale en attendant copy_image) ---
    Image copie_pour_detection = img_binarisee;
    // --- FIN SIMULATION ---

    // TÂCHE 1 (Toi)
    Image img_grille = isolate_grid_image(copie_pour_detection);
    Image img_mots = isolate_word_list_image(copie_pour_detection);
    

    /*
     * ÉTAPE 3 : EXTRACTION (Toi)
     */
    printf("[3/5] Extraction des lettres...\n");
    
    // TÂCHE 2 (Toi)
    Letters lettres_des_mots = extract_letters(img_mots);
    // TÂCHE 3 (Toi)
    Letters lettres_de_la_grille = extract_grid_letters(img_grille);


    /*
     * ÉTAPE 4 : RECONNAISSANCE (Théo)
     * (Les noms de fonctions sont inventés)
     */
    printf("[4/5] Reconnaissance des caractères (Théo)...\n");
    // Network net = load_network("chemin/vers/reseau.dat");
    // WordList mots_reconnus = recognize_letters(lettres_des_mots, net);
    // (Faut aussi reconnaître les lettres de la grille si nécessaire)


    /*
     * ÉTAPE 5 : SOLVER
     * (Les noms de fonctions sont inventés)
     */
    printf("[5/5] Résolution...\n");
    // Grid grille_resolue = solve_grid(img_grille, mots_reconnus);
    

    /*
     * ÉTAPE 6 : CLEANUP
     */
    printf("--- Terminé. Nettoyage de la mémoire. ---\n");
    free(img_binarisee.data); // A remplacer par free_image(img_binarisee);
    free(img_grille.data);    // A remplacer par free_image(img_grille);
    free(img_mots.data);      // A remplacer par free_image(img_mots);
    free_letters(lettres_des_mots);
    free_letters(lettres_de_la_grille);
    // ... (libérer mots_reconnus et grille_resolue aussi)

    return 0;
}