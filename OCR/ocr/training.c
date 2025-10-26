#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../utils/structs.h"
#include "neural_network.h"
#include "training.h"

// Déclarations des fonctions utilitaires/de rétropropagation (doivent exister dans utils.c ou neural_network.c)
extern Tensor *create_tensor(int r, int c, int d);
extern void free_tensor(Tensor *t);
extern void zero_gradients(NeuralNetwork *net); // Met tous les d_weights/d_filters à zéro.

// Fonctions de rétropropagation spécifiques aux couches
extern Tensor *backward_fc(FCLayer *layer, Tensor *delta_in, Tensor *input_activation);
extern Tensor *backward_pool(int pool_size, int stride, Tensor *delta_in, Tensor *forward_output);
extern Tensor *backward_conv(ConvLayer *layer, Tensor *delta_in, Tensor *input_activation);
extern Tensor *backward_flatten(Tensor *delta_in, int prev_rows, int prev_cols, int prev_depth);

// Fonction de la passe avant (déclarée dans neural_network.h)
extern Tensor *forward_pass(NeuralNetwork *net, Tensor *input_tensor);


/**
 * @brief Calcule la perte de l'entropie croisée.
 * L = - sum( y_i * log(p_i) )
 */
float cross_entropy_loss(Tensor *predicted, Tensor *expected) {
    float loss = 0.0f;
    int size = predicted->cols; 
    
    // Vérification de la taille (devrait être 26)
    if (predicted->cols != expected->cols) {
        fprintf(stderr, "Erreur: Les tailles des tenseurs de perte ne correspondent pas.\n");
        return -1.0f;
    }

    for (int i = 0; i < size; i++) {
        if (expected->data[i] > 0.5f) { // Seul le label correct (y_i = 1) contribue à la perte
            // Ajout d'une petite valeur (1e-9) pour éviter log(0)
            loss -= expected->data[i] * logf(predicted->data[i] + 1e-9f); 
        }
    }
    return loss;
}

/**
 * @brief Effectue la rétropropagation pour calculer les gradients.
 */
void backward_pass(NeuralNetwork *net, Tensor *input, Tensor *expected_output) {
    printf("--- Rétropropagation ---\n");
    
    // 1. Calcul du delta initial (erreur dL/dZ) sur la couche de sortie Softmax/FC
    // L'erreur de la Softmax est simplement : Predicted_Probabilities - Expected_OneHot
    Tensor *delta_out = create_tensor(1, expected_output->cols, 1);
    for (int i = 0; i < expected_output->cols; i++) {
        // Delta = Probabilité (p_i) - Label (y_i)
        delta_out->data[i] = net->fc1_output->data[i] - expected_output->data[i];
    }
    
    // Stockage du delta pour la propagation
    Tensor *current_delta = delta_out;


    // --- 2. Rétropropagation à travers la Couche FC_OUT (Classification) ---
    // Calcule dW_fc_out, dB_fc_out et le delta pour la couche précédente (fc1)
    Tensor *delta_fc1 = backward_fc(&net->fc_out, current_delta, net->fc1_output);
    free_tensor(current_delta);
    current_delta = delta_fc1;


    // --- 3. Rétropropagation à travers la Couche FC1 (Cachée) ---
    // Calcule dW_fc1, dB_fc1 et le delta pour la couche précédente (flattened)
    Tensor *delta_flattened = backward_fc(&net->fc1, current_delta, net->flattened_output);
    free_tensor(current_delta);
    current_delta = delta_flattened;


    // --- 4. Rétropropagation à travers l'Aplatissement (Unflatten) ---
    // Redimensionne le vecteur delta (800) en tenseur (5x5x32)
    Tensor *delta_pool2 = backward_flatten(current_delta, 5, 5, 32); 
    free_tensor(current_delta);
    current_delta = delta_pool2;


    // --- 5. Rétropropagation à travers POOL2 ---
    // Le delta est propagé aux régions Max dans le tenseur précédent (conv2)
    Tensor *delta_conv2 = backward_pool(net->pool2_size, net->pool2_stride, 
                                        current_delta, net->conv2_output);
    free_tensor(current_delta);
    current_delta = delta_conv2;


    // --- 6. Rétropropagation à travers CONV2 ---
    // Calcule dF_conv2, dB_conv2, et le delta pour la couche précédente (pool1)
    Tensor *delta_pool1 = backward_conv(&net->conv2, current_delta, net->pool1_output);
    free_tensor(current_delta);
    current_delta = delta_pool1;


    // --- 7. Rétropropagation à travers POOL1 ---
    // Le delta est propagé aux régions Max dans le tenseur précédent (conv1)
    Tensor *delta_conv1 = backward_pool(net->pool1_size, net->pool1_stride, 
                                        current_delta, net->conv1_output);
    free_tensor(current_delta);
    current_delta = delta_conv1;

    
    // --- 8. Rétropropagation à travers CONV1 ---
    // Calcule dF_conv1, dB_conv1. L'erreur ne va pas plus loin que l'entrée.
    Tensor *delta_input = backward_conv(&net->conv1, current_delta, input);
    free_tensor(current_delta);
    free_tensor(delta_input); // Le delta de l'entrée n'est pas utilisé pour la mise à jour
    
    printf("Rétropropagation terminée.\n");
}

/**
 * @brief Met à jour les poids et les biais du réseau (SGD).
 */
void update_weights(NeuralNetwork *net, float learning_rate) {
    printf("Mise à jour des poids (LR=%.5f)...\n", learning_rate);

    // Fonction utilitaire pour mettre à jour un tenseur (poids/biais)
    void update_tensor(Tensor *target, Tensor *gradient) {
        int size = target->rows * target->cols * target->depth;
        for (int i = 0; i < size; i++) {
            // W = W - learning_rate * dW
            target->data[i] -= learning_rate * gradient->data[i];
        }
    }

    // 1. Couche FC_OUT
    update_tensor(net->fc_out.weights, net->fc_out.d_weights);
    update_tensor(net->fc_out.biases, net->fc_out.d_biases);

    // 2. Couche FC1
    update_tensor(net->fc1.weights, net->fc1.d_weights);
    update_tensor(net->fc1.biases, net->fc1.d_biases);

    // 3. Couche CONV2
    update_tensor(net->conv2.filters, net->conv2.d_filters);
    update_tensor(net->conv2.biases, net->conv2.d_biases);

    // 4. Couche CONV1
    update_tensor(net->conv1.filters, net->conv1.d_filters);
    update_tensor(net->conv1.biases, net->conv1.d_biases);
    
    printf("Mise à jour terminée.\n");
}


/**
 * @brief Exécute une seule étape d'entraînement.
 */
void train_network_step(NeuralNetwork *net, Tensor *input, Tensor *expected_output, float learning_rate) {
    // 1. Mise à zéro des gradients accumulés pour cette étape
    // (Crucial si l'on utilise des mini-batchs, mais bonne pratique même pour SGD simple)
    zero_gradients(net); 

    // 2. Passage Avant (Forward Pass)
    Tensor *predicted_probs = forward_pass(net, input);
    
    // 3. Calcul de l'erreur
    float loss = cross_entropy_loss(predicted_probs, expected_output);
    if (loss < 0) {
        free_tensor(predicted_probs);
        return;
    }
    printf("Loss actuelle: %.6f\n", loss);
    
    // 4. Rétropropagation (Backward Pass)
    backward_pass(net, input, expected_output);
    
    // 5. Mise à jour des poids
    update_weights(net, learning_rate);
    
    // 6. Nettoyage
    free_tensor(predicted_probs);
}