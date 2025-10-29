#ifndef GRID_DETECTION_H
#define GRID_DETECTION_H

#include "../UTILS/structs.h" // On utilise 'Image' et 'Letters'

/*
 * TÂCHE 1 (Shani) : "Couper l'image en deux"
 */

/**
 * @brief Isole la zone de la grille de l'image principale.
 * @param img L'image binarisée complète (qui sera modifiée !)
 * @return Une nouvelle 'Image' ne contenant que la grille.
 * @warning CETTE FONCTION MODIFIE L'IMAGE D'ENTREE 'img' EN EFFACANT LA GRILLE.
 */
Image isolate_grid_image(Image img);

/**
 * @brief Isole la zone de la liste de mots de l'image principale.
 * @param img L'image binarisée (modifiée par isolate_grid_image)
 * @return Une nouvelle 'Image' ne contenant que la liste de mots.
 */
Image isolate_word_list_image(Image img);


/*
 * TÂCHE 3 (Shani) : "Détecter la position des lettres [de la grille]"
 */

/**
 * @brief Extrait les images des lettres déjà présentes dans la grille.
 * @param grid_image L'image ne contenant que la grille.
 * @return Une structure 'Letters' contenant les images 50x50 des lettres de la grille.
 */
Letters extract_grid_letters(Image grid_image);


#endif // GRID_DETECTION_H
