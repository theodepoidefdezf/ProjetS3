#ifndef LETTER_EXTRACTION_H
#define LETTER_EXTRACTION_H

#include "../utils/structs.h"

/**
 * @brief Extrait chaque lettre d'une image contenant une liste de mots.
 *
 * @param word_list_image L'image (struct Image) contenant SEULEMENT la liste de mots.
 * @return Une structure 'Letters' contenant les images de chaque lettre.
 */
Letters extract_letters(Image word_list_image);

#endif // LETTER_EXTRACTION_H
