#ifndef STRUCTS_H
#define STRUCTS_H

// --- 1. Structure pour l'Image Binarisée (Entrée) ---
typedef struct {
    int rows; 
    int cols; 
    int depth; // 1 pour binarisé/niveaux de gris
    int *data; // Matrice de pixels (0 ou 1)
} Image;

// --- 2. Structure pour le Tenseur (Données du CNN) ---
typedef struct {
    int rows;
    int cols;
    int depth; // Nombre de canaux/filtres
    float *data;
} Tensor;

// --- 3. Structures des Couches du CNN ---

// Couche de Convolution
typedef struct {
    int num_filters;
    int kernel_size; // Taille du filtre (ex: 5x5)
    int stride;      // Pas (ex: 1)
    int padding;     // Remplissage (ex: 2 pour garder la taille)
    Tensor *filters;
    Tensor *biases;
    // ... (Gradients pour l'entraînement)
} ConvLayer;

// Couche Entièrement Connectée (Fully Connected - Dense)
typedef struct {
    int input_size;
    int output_size;
    Tensor *weights;
    Tensor *biases;
    // ... (Gradients pour l'entraînement)
} FCLayer;

// Architecture du Réseau Neuronal Convolutif (CNN)
typedef struct {
    // --- Phase d'Extraction de Caractéristiques (50x50x1) ---
    ConvLayer conv1; // Sortie: 50x50x16
    
    int pool1_size;  // Ex: 2x2
    int pool1_stride; // Sortie: 25x25x16
    
    ConvLayer conv2; // Sortie: 25x25x32
    
    int pool2_size;  // Ex: 5x5
    int pool2_stride; // Sortie: 5x5x32

    // --- Phase de Classification ---
    FCLayer fc1;      // Couche cachée (Input: 800)
    FCLayer fc_out;   // Couche de sortie (Output: 26 classes)
    
    Tensor *conv1_output;
    Tensor *pool1_output;
    Tensor *conv2_output;
    Tensor *pool2_output;
    Tensor *flattened_output;
    Tensor *fc1_output;

} NeuralNetwork;

// Structure de la Grille (pour l'extraction dans le module detection)
typedef struct {
    int size;          
    char **letters;    
} Grid;

#endif // STRUCTS_H