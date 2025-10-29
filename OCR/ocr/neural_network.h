#ifndef RESEAU_NEURONES_XOR_H
#define RESEAU_NEURONES_XOR_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NB_ENTREES 2
#define NB_NEURONES_CACHES 2
#define NB_SORTIES 1

#define TAUX_APPRENTISSAGE 0.5
#define NB_EPOCHS 10000

extern double W1[NB_ENTREES][NB_NEURONES_CACHES];
extern double B1[NB_NEURONES_CACHES];

extern double W2[NB_NEURONES_CACHES][NB_SORTIES];
extern double B2[NB_SORTIES];

extern double H[NB_NEURONES_CACHES];
extern double O[NB_SORTIES];

double sigmoid(double x);
double sigmoid_derivative(double x);
double rand_double();
void initialiser_reseau();
void forward_propagation(double *entrees);
void backpropagation(double *entrees, double *attendus);
void entrainer();
void tester();

#endif