#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// --- Paramètres du Réseau ---
#define NB_ENTREES 2
#define NB_NEURONES_CACHES 2
#define NB_SORTIES 1

// --- Paramètres d'Entraînement ---
#define TAUX_APPRENTISSAGE 0.5
#define NB_EPOCHS 10000

// --- Structure des Poids et Biais ---
// Couche Cachée : [entrée][neurone_cache]
double W1[NB_ENTREES][NB_NEURONES_CACHES];
double B1[NB_NEURONES_CACHES];

// Couche de Sortie : [neurone_cache][sortie]
double W2[NB_NEURONES_CACHES][NB_SORTIES];
double B2[NB_SORTIES];

// --- Données d'Activation (pour la propagation avant) ---
double H[NB_NEURONES_CACHES]; // Activations de la couche cachée
double O[NB_SORTIES];         // Activation de la couche de sortie

double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

double sigmoid_derivative(double x) {
    return x * (1.0 - x);
}

double rand_double() {
    return (double)rand() / (double)RAND_MAX * 2.0 - 1.0;
}

void initialiser_reseau() {
    srand(time(NULL));

    // Initialisation des poids et biais de la couche cachée
    for (int i = 0; i < NB_ENTREES; i++) {
        for (int j = 0; j < NB_NEURONES_CACHES; j++) {
            W1[i][j] = rand_double();
        }
    }
    for (int j = 0; j < NB_NEURONES_CACHES; j++) {
        B1[j] = rand_double();
    }

    // Initialisation des poids et biais de la couche de sortie
    for (int i = 0; i < NB_NEURONES_CACHES; i++) {
        for (int j = 0; j < NB_SORTIES; j++) {
            W2[i][j] = rand_double();
        }
    }
    for (int j = 0; j < NB_SORTIES; j++) {
        B2[j] = rand_double();
    }
}

void forward_propagation(double *entrees) {
    // Couche Cachée
    for (int j = 0; j < NB_NEURONES_CACHES; j++) {
        double somme = 0.0;
        for (int i = 0; i < NB_ENTREES; i++) {
            somme += entrees[i] * W1[i][j];
        }
        somme += B1[j];
        H[j] = sigmoid(somme);
    }

    // Couche de Sortie
    for (int j = 0; j < NB_SORTIES; j++) {
        double somme = 0.0;
        for (int i = 0; i < NB_NEURONES_CACHES; i++) {
            somme += H[i] * W2[i][j];
        }
        somme += B2[j];
        O[j] = sigmoid(somme);
    }
}

void backpropagation(double *entrees, double *attendus) {
    
    // Calcul de l'Erreur de Sortie 
    double delta2[NB_SORTIES];
    for (int j = 0; j < NB_SORTIES; j++) {
        // Erreur = (Attendu - Prédit) * Dérivée(Prédit)
        double erreur = attendus[j] - O[j];
        delta2[j] = erreur * sigmoid_derivative(O[j]);
    }

    // Mise à jour de W2 et B2 (Couche de Sortie)
    for (int i = 0; i < NB_NEURONES_CACHES; i++) {
        for (int j = 0; j < NB_SORTIES; j++) {
            // Mise à jour du poids W2
            W2[i][j] += TAUX_APPRENTISSAGE * delta2[j] * H[i];
        }
    }
    for (int j = 0; j < NB_SORTIES; j++) {
        // Mise à jour du biais B2
        B2[j] += TAUX_APPRENTISSAGE * delta2[j];
    }

    // Calcul de l'Erreur Cachée
    double delta1[NB_NEURONES_CACHES];
    for (int i = 0; i < NB_NEURONES_CACHES; i++) {
        double somme_gradient_poids = 0.0;
        // Propagation de l'erreur de la couche de sortie à la couche cachée
        for (int j = 0; j < NB_SORTIES; j++) {
            somme_gradient_poids += delta2[j] * W2[i][j];
        }
        // Erreur cachée = Somme_propagation * Dérivée(Activation_cachée)
        delta1[i] = somme_gradient_poids * sigmoid_derivative(H[i]);
    }

    // Mise à jour de W1 et B1 (Couche Cachée)
    for (int i = 0; i < NB_ENTREES; i++) {
        for (int j = 0; j < NB_NEURONES_CACHES; j++) {
            // Mise à jour du poids W1
            W1[i][j] += TAUX_APPRENTISSAGE * delta1[j] * entrees[i];
        }
    }
    for (int j = 0; j < NB_NEURONES_CACHES; j++) {
        // Mise à jour du biais B1
        B1[j] += TAUX_APPRENTISSAGE * delta1[j];
    }
}

void entrainer() {
    // Jeu de données d'entraînement XOR : Entrée[2] et Sortie Attendue[1]
    double TRAINING_DATA[4][NB_ENTREES] = {{0.0, 0.0}, {0.0, 1.0}, {1.0, 0.0}, {1.0, 1.0}};
    double TARGET_DATA[4][NB_SORTIES] = {{1.0}, {0.0}, {0.0}, {1.0}};

    for (int epoch = 0; epoch < NB_EPOCHS; epoch++) {
        double erreur_totale = 0.0;
        
        for (int i = 0; i < 4; i++) {
            double *entrees = TRAINING_DATA[i];
            double *attendus = TARGET_DATA[i];

            // Prédiction
            forward_propagation(entrees);

            // Calcul et Accumulation de l'Erreur
            erreur_totale += 0.5 * pow(attendus[0] - O[0], 2);

            // Ajustement des Poids
            backpropagation(entrees, attendus);
        }

        if (epoch % 1000 == 0) {
            printf("Epoch %d, Erreur Moyenne: %.6f\n", epoch, erreur_totale / 4.0);
        }
    }
}

void tester() {
    printf("\n--- Test du Modèle Entraîné ---\n");
    
    // Test des 4 cas XOR
    double test_cases[4][NB_ENTREES] = {{0.0, 0.0}, {0.0, 1.0}, {1.0, 0.0}, {1.0, 1.0}};
    int expected[4] = {1, 0, 0, 1};

    for (int i = 0; i < 4; i++) {
        forward_propagation(test_cases[i]);
        
        // La sortie est une probabilité (entre 0 et 1), on la seuille à 0.5
        int resultat_predit = (O[0] > 0.5) ? 1 : 0;

        printf("Entree (%.0f, %.0f) -> Prédit: %.4f (Résultat Binaire: %d) | Attendu: %d ",
               test_cases[i][0], test_cases[i][1], O[0], resultat_predit, expected[i]);
        
        if (resultat_predit == expected[i]) {
            printf("Succès\n");
        } else {
            printf("Échec\n");
        }
    }
}

int main() {
    
    printf("--------\n");
    
    initialiser_reseau();

    entrainer();

    tester();

    return 0;
}
