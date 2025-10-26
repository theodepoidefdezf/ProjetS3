#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../utils/structs.h"
#include "neural_network.h"

#define INPUT_SIZE 50
#define NUM_CLASSES 26 // A-Z

// Déclarations des utilitaires (implémentés dans utils.c)
extern Tensor *create_tensor(int r, int c, int d);
extern void free_tensor(Tensor *t);
extern void init_conv_layer(ConvLayer *layer, int input_d, int num_f, int k_size, int stride, int padding);
extern void init_fc_layer(FCLayer *layer, int input_s, int output_s);
extern void softmax(Tensor *input_raw, Tensor *output_probs);
extern Tensor *forward_conv(ConvLayer *layer, Tensor *input);
extern Tensor *forward_pool(int pool_size, int stride, Tensor *input);
extern Tensor *forward_fc(FCLayer *layer, Tensor *input);
extern Tensor *flatten_tensor(Tensor *input);


NeuralNetwork *create_network() {
    NeuralNetwork *net = (NeuralNetwork *)malloc(sizeof(NeuralNetwork));
    if (!net) exit(EXIT_FAILURE);

    // --- CONV1 : 16 filtres 5x5, stride 1, padding 2 (pour maintenir la taille) ---
    // Sortie: 50x50x16
    init_conv_layer(&net->conv1, 1, 16, 5, 1, 2);
    
    // --- POOL1 : MaxPool 2x2, stride 2 ---
    // Sortie: 25x25x16
    net->pool1_size = 2;
    net->pool1_stride = 2;

    // --- CONV2 : 32 filtres 3x3, stride 1, padding 1 ---
    // Sortie: 25x25x32
    init_conv_layer(&net->conv2, 16, 32, 3, 1, 1);
    
    // --- POOL2 : MaxPool 5x5, stride 5 ---
    // Sortie: 5x5x32
    net->pool2_size = 5;
    net->pool2_stride = 5;

    // Taille après aplatissement : 5 * 5 * 32 = 800
    int flattened_size = 5 * 5 * 32; 

    // --- FC1 (Couche cachée) : 800 -> 128 neurones ---
    init_fc_layer(&net->fc1, flattened_size, 128); 

    // --- FC_OUT (Sortie) : 128 -> 26 classes (A-Z) ---
    init_fc_layer(&net->fc_out, 128, NUM_CLASSES); 

    // Initialisation des pointeurs intermédiaires
    net->conv1_output = NULL;
    // ...
    printf("Réseau neuronal 2-couches pour 50x50 créé. Taille Flattened: %d\n", flattened_size);

    return net;
}

// ... (free_network() implémentation)

Tensor *forward_pass(NeuralNetwork *net, Tensor *input_tensor) {
    // 1. CONV1 -> ReLU
    net->conv1_output = forward_conv(&net->conv1, input_tensor);
    
    // 2. POOL1
    net->pool1_output = forward_pool(net->pool1_size, net->pool1_stride, net->conv1_output);
    free_tensor(net->conv1_output); // Libération mémoire intermédiaire 
    
    // 3. CONV2 -> ReLU
    net->conv2_output = forward_conv(&net->conv2, net->pool1_output);
    free_tensor(net->pool1_output); 

    // 4. POOL2
    net->pool2_output = forward_pool(net->pool2_size, net->pool2_stride, net->conv2_output);
    free_tensor(net->conv2_output); 

    // 5. FLATTEN (5x5x32 -> 800)
    net->flattened_output = flatten_tensor(net->pool2_output);
    free_tensor(net->pool2_output); 

    // 6. FC1 (Couche cachée)
    net->fc1_output = forward_fc(&net->fc1, net->flattened_output);
    
    // 7. FC_OUT (Sortie)
    Tensor *final_output_raw = forward_fc(&net->fc_out, net->fc1_output);
    
    // 8. SOFTMAX pour les probabilités
    Tensor *final_probs = create_tensor(1, NUM_CLASSES, 1);
    softmax(final_output_raw, final_probs); 
    
    // Nettoyage
    free_tensor(final_output_raw);
    
    return final_probs; // Tenseur de probabilités
}