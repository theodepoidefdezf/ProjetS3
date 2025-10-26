#include <stdio.h>
#include <stdlib.h>
#include "../utils/structs.h"
#include "neural_network.h"
#include "recognition.h"

#define INPUT_SIZE 50
#define NUM_CLASSES 26 // A-Z

extern Tensor *create_tensor(int r, int c, int d);
extern void free_tensor(Tensor *t);

Tensor *image_to_tensor_input(Image img) {
    if (img.rows != INPUT_SIZE || img.cols != INPUT_SIZE || img.depth != 1) {
        fprintf(stderr, "Erreur: Taille d'image attendue: %dx%dx1. Reçue: %dx%dx%d\n", 
                INPUT_SIZE, INPUT_SIZE, img.rows, img.cols, img.depth);
        return NULL;
    }

    Tensor *input_tensor = create_tensor(img.rows, img.cols, 1);
    int size = img.rows * img.cols;

    // Copie et conversion des int (0/1) en float (0.0/1.0)
    for (int i = 0; i < size; i++) {
        input_tensor->data[i] = (float)img.data[i];
    }
    
    return input_tensor;
}

char decode_argmax(Tensor *probs) {
    float max_prob = 0.0f;
    int max_index = -1;
    
    // L'Argmax (indice de la probabilité maximale)
    for (int i = 0; i < probs->cols; i++) {
        if (probs->data[i] > max_prob) {
            max_prob = probs->data[i];
            max_index = i;
        }
    }
    
    // Index (0 à 25) -> Caractère ('A' à 'Z')
    if (max_index >= 0 && max_index < NUM_CLASSES) {
        return (char)('A' + max_index);
    }
    
    return '?';
}


char recognize_letter(NeuralNetwork *net, Image input_image) {
    printf("Début de la reconnaissance...\n");

    // 1. Conversion Image (int 50x50x1) -> Tenseur (float)
    Tensor *input_tensor = image_to_tensor_input(input_image);
    if (!input_tensor) return '?';

    // 2. Passage avant (Prédiction)
    Tensor *probs = forward_pass(net, input_tensor);

    // 3. Décodage (Argmax)
    char result_char = decode_argmax(probs);
    
    // 4. Nettoyage
    free_tensor(input_tensor);
    free_tensor(probs);
    
    return result_char;
}