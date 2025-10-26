#ifndef TRAINING_H
#define TRAINING_H

#include "../utils/structs.h"

/**
 * @brief Calcule la perte de l'entropie croisée (Cross-Entropy Loss).
 * @param predicted Tenseur de probabilités prédites (sortie Softmax).
 * @param expected Tenseur de la vérité terrain (One-Hot Encoding).
 * @return La valeur de la perte.
 */
float cross_entropy_loss(Tensor *predicted, Tensor *expected);

/**
 * @brief Effectue la rétropropagation pour calculer les gradients.
 * Calcule dL/dW et dL/dB pour toutes les couches et les stocke dans d_weights/d_filters.
 * @param net Le réseau de neurones.
 * @param input L'image d'entrée (Tensor) utilisée pour le forward pass.
 * @param expected_output La vérité terrain.
 */
void backward_pass(NeuralNetwork *net, Tensor *input, Tensor *expected_output);

/**
 * @brief Met à jour les poids et les biais du réseau.
 * Applique la descente de gradient : W = W - learning_rate * dW.
 * @param net Le réseau de neurones.
 * @param learning_rate Le taux d'apprentissage.
 */
void update_weights(NeuralNetwork *net, float learning_rate);

/**
 * @brief Exécute une seule étape d'entraînement (Forward -> Loss -> Backward -> Update).
 * @param net Le réseau de neurones.
 * @param input Tenseur d'entrée (image).
 * @param expected_output Tenseur attendu (label).
 * @param learning_rate Taux d'apprentissage.
 */
void train_network_step(NeuralNetwork *net, Tensor *input, Tensor *expected_output, float learning_rate);

#endif // TRAINING_H