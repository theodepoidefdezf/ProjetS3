/*
 * CNN pour reconnaissance de lettres - Utilise uniquement stdio.h
 * Architecture: Conv -> ReLU -> Pool -> Conv -> ReLU -> Pool -> FC -> FC -> Softmax
 * Images d'entrée: 50x50 pixels en niveaux de gris
 * Sortie: 26 classes (A-Z)
 */

#include <stdio.h>

/* ============================================================
 * CONSTANTES ET CONFIGURATION
 * ============================================================ */

#define IMG_SIZE 50
#define NUM_CLASSES 26

/* Architecture du réseau - VERSION AUGMENTÉE */
#define CONV1_FILTERS 16      /* 8 → 16 */
#define CONV1_SIZE 5
#define POOL1_SIZE 2
#define AFTER_POOL1 23

#define CONV2_FILTERS 32      /* 16 → 32 */
#define CONV2_SIZE 3
#define AFTER_POOL2 10

#define FC1_SIZE 256          /* 128 → 256 */
#define FC2_SIZE NUM_CLASSES

#define FLATTEN_SIZE (CONV2_FILTERS * AFTER_POOL2 * AFTER_POOL2)  /* sera 32*10*10 = 3200 */

/* Hyperparamètres */
#define LEARNING_RATE 0.0005f  /* réduire un peu car réseau plus grand */
#define EPOCHS 30              /* peut nécessiter plus d'époques */
#define SAMPLES_PER_LETTER 1000

/* ============================================================
 * FONCTIONS MATHÉMATIQUES DE BASE
 * ============================================================ */

static unsigned int rand_seed = 42;

float my_rand(void) {
    rand_seed = rand_seed * 1103515245 + 12345;
    return (float)((rand_seed >> 16) & 0x7FFF) / 32767.0f;
}

float my_exp(float x) {
    /* Approximation de e^x par série de Taylor */
    if (x > 20.0f) return 485165195.4f;
    if (x < -20.0f) return 0.0f;
    
    float result = 1.0f;
    float term = 1.0f;
    int i;
    for (i = 1; i <= 20; i++) {
        term *= x / (float)i;
        result += term;
    }
    return result;
}

float my_log(float x) {
    /* Approximation de ln(x) */
    if (x <= 0.0f) return -1000.0f;
    if (x < 0.0001f) return -1000.0f;
    
    /* Normalisation: ln(x) = ln(m * 2^e) = ln(m) + e*ln(2) */
    float result = 0.0f;
    while (x > 2.0f) { x /= 2.0f; result += 0.693147f; }
    while (x < 0.5f) { x *= 2.0f; result -= 0.693147f; }
    
    /* Série de Taylor pour ln(1+y) où y = x-1 */
    float y = x - 1.0f;
    float term = y;
    float sign = 1.0f;
    int i;
    for (i = 1; i <= 30; i++) {
        result += sign * term / (float)i;
        term *= y;
        sign = -sign;
    }
    return result;
}

float my_sqrt(float x) {
    if (x <= 0.0f) return 0.0f;
    float guess = x / 2.0f;
    int i;
    for (i = 0; i < 20; i++) {
        guess = (guess + x / guess) / 2.0f;
    }
    return guess;
}

float relu(float x) {
    return x > 0.0f ? x : 0.0f;
}

float relu_derivative(float x) {
    return x > 0.0f ? 1.0f : 0.0f;
}

/* ============================================================
 * STRUCTURE DU RÉSEAU DE NEURONES
 * ============================================================ */

typedef struct {
    /* Couche Conv1: 8 filtres 5x5 */
    float conv1_weights[CONV1_FILTERS][CONV1_SIZE][CONV1_SIZE];
    float conv1_bias[CONV1_FILTERS];
    
    /* Couche Conv2: 16 filtres 3x3x8 */
    float conv2_weights[CONV2_FILTERS][CONV1_FILTERS][CONV2_SIZE][CONV2_SIZE];
    float conv2_bias[CONV2_FILTERS];
    
    /* Couche FC1: FLATTEN_SIZE -> FC1_SIZE */
    float fc1_weights[FC1_SIZE][FLATTEN_SIZE];
    float fc1_bias[FC1_SIZE];
    
    /* Couche FC2: FC1_SIZE -> NUM_CLASSES */
    float fc2_weights[FC2_SIZE][FC1_SIZE];
    float fc2_bias[FC2_SIZE];
} CNN;

/* Buffers pour la propagation avant et arrière */
typedef struct {
    float input[IMG_SIZE][IMG_SIZE];
    
    /* Sorties Conv1 */
    float conv1_out[CONV1_FILTERS][IMG_SIZE - CONV1_SIZE + 1][IMG_SIZE - CONV1_SIZE + 1];
    float relu1_out[CONV1_FILTERS][IMG_SIZE - CONV1_SIZE + 1][IMG_SIZE - CONV1_SIZE + 1];
    float pool1_out[CONV1_FILTERS][AFTER_POOL1][AFTER_POOL1];
    int pool1_max_i[CONV1_FILTERS][AFTER_POOL1][AFTER_POOL1];
    int pool1_max_j[CONV1_FILTERS][AFTER_POOL1][AFTER_POOL1];
    
    /* Sorties Conv2 */
    float conv2_out[CONV2_FILTERS][AFTER_POOL1 - CONV2_SIZE + 1][AFTER_POOL1 - CONV2_SIZE + 1];
    float relu2_out[CONV2_FILTERS][AFTER_POOL1 - CONV2_SIZE + 1][AFTER_POOL1 - CONV2_SIZE + 1];
    float pool2_out[CONV2_FILTERS][AFTER_POOL2][AFTER_POOL2];
    int pool2_max_i[CONV2_FILTERS][AFTER_POOL2][AFTER_POOL2];
    int pool2_max_j[CONV2_FILTERS][AFTER_POOL2][AFTER_POOL2];
    
    /* Vecteur aplati */
    float flatten[FLATTEN_SIZE];
    
    /* Sorties FC */
    float fc1_out[FC1_SIZE];
    float relu3_out[FC1_SIZE];
    float fc2_out[FC2_SIZE];
    float softmax_out[NUM_CLASSES];
} ForwardCache;

/* Gradients */
typedef struct {
    float conv1_weights[CONV1_FILTERS][CONV1_SIZE][CONV1_SIZE];
    float conv1_bias[CONV1_FILTERS];
    float conv2_weights[CONV2_FILTERS][CONV1_FILTERS][CONV2_SIZE][CONV2_SIZE];
    float conv2_bias[CONV2_FILTERS];
    float fc1_weights[FC1_SIZE][FLATTEN_SIZE];
    float fc1_bias[FC1_SIZE];
    float fc2_weights[FC2_SIZE][FC1_SIZE];
    float fc2_bias[FC2_SIZE];
} Gradients;

/* Variables globales */
static CNN network;
static ForwardCache cache;
static Gradients grads;

/* ============================================================
 * INITIALISATION
 * ============================================================ */

float xavier_init(int fan_in, int fan_out) {
    /* Initialisation Xavier/Glorot */
    float limit = my_sqrt(6.0f / (float)(fan_in + fan_out));
    return (my_rand() * 2.0f - 1.0f) * limit;
}

void init_network(void) {
    int f, c, i, j;
    
    /* Conv1: fan_in = 5*5 = 25, fan_out = 8 */
    for (f = 0; f < CONV1_FILTERS; f++) {
        for (i = 0; i < CONV1_SIZE; i++) {
            for (j = 0; j < CONV1_SIZE; j++) {
                network.conv1_weights[f][i][j] = xavier_init(25, CONV1_FILTERS);
            }
        }
        network.conv1_bias[f] = 0.0f;
    }
    
    /* Conv2: fan_in = 8*3*3 = 72, fan_out = 16 */
    for (f = 0; f < CONV2_FILTERS; f++) {
        for (c = 0; c < CONV1_FILTERS; c++) {
            for (i = 0; i < CONV2_SIZE; i++) {
                for (j = 0; j < CONV2_SIZE; j++) {
                    network.conv2_weights[f][c][i][j] = xavier_init(72, CONV2_FILTERS);
                }
            }
        }
        network.conv2_bias[f] = 0.0f;
    }
    
    /* FC1 */
    for (i = 0; i < FC1_SIZE; i++) {
        for (j = 0; j < FLATTEN_SIZE; j++) {
            network.fc1_weights[i][j] = xavier_init(FLATTEN_SIZE, FC1_SIZE);
        }
        network.fc1_bias[i] = 0.0f;
    }
    
    /* FC2 */
    for (i = 0; i < FC2_SIZE; i++) {
        for (j = 0; j < FC1_SIZE; j++) {
            network.fc2_weights[i][j] = xavier_init(FC1_SIZE, FC2_SIZE);
        }
        network.fc2_bias[i] = 0.0f;
    }
}

/* ============================================================
 * LECTURE D'IMAGE PBM/PGM
 * ============================================================ */

/* Fonction pour sauter les espaces blancs et commentaires */
static void skip_whitespace_and_comments(FILE *fp) {
    int c;
    while (1) {
        c = fgetc(fp);
        if (c == '#') {
            /* Sauter le commentaire jusqu'à la fin de la ligne */
            while ((c = fgetc(fp)) != '\n' && c != EOF);
        } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            /* Continuer à sauter les espaces blancs */
        } else {
            if (c != EOF) {
                ungetc(c, fp);
            }
            break;
        }
    }
}

int read_pbm(const char *filename, float img[IMG_SIZE][IMG_SIZE]) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("Erreur: impossible d'ouvrir %s\n", filename);
        return -1;
    }
    
    /* Lire le magic number */
    char magic[3];
    if (fscanf(fp, "%2s", magic) != 1) {
        printf("Erreur: impossible de lire le magic number\n");
        fclose(fp);
        return -1;
    }
    
    int is_pbm_ascii = (magic[0] == 'P' && magic[1] == '1');   /* PBM ASCII */
    int is_pbm_binary = (magic[0] == 'P' && magic[1] == '4');  /* PBM binaire */
    int is_pgm_ascii = (magic[0] == 'P' && magic[1] == '2');   /* PGM ASCII */
    int is_pgm_binary = (magic[0] == 'P' && magic[1] == '5');  /* PGM binaire */
    
    if (!is_pbm_ascii && !is_pbm_binary && !is_pgm_ascii && !is_pgm_binary) {
        printf("Erreur: format non supporté (attendu P1, P2, P4 ou P5, reçu %s)\n", magic);
        fclose(fp);
        return -1;
    }
    
    /* Lire les dimensions */
    skip_whitespace_and_comments(fp);
    int width, height;
    if (fscanf(fp, "%d", &width) != 1) {
        printf("Erreur: impossible de lire la largeur\n");
        fclose(fp);
        return -1;
    }
    
    skip_whitespace_and_comments(fp);
    if (fscanf(fp, "%d", &height) != 1) {
        printf("Erreur: impossible de lire la hauteur\n");
        fclose(fp);
        return -1;
    }
    
    if (width != IMG_SIZE || height != IMG_SIZE) {
        printf("Erreur: image doit être %dx%d (reçu %dx%d)\n", IMG_SIZE, IMG_SIZE, width, height);
        fclose(fp);
        return -1;
    }
    
    /* Lire maxval pour PGM */
    int maxval = 1;
    if (is_pgm_ascii || is_pgm_binary) {
        skip_whitespace_and_comments(fp);
        if (fscanf(fp, "%d", &maxval) != 1) {
            printf("Erreur: impossible de lire maxval\n");
            fclose(fp);
            return -1;
        }
    }
    
    /* Sauter un seul caractère whitespace après le header pour le format binaire */
    if (is_pbm_binary || is_pgm_binary) {
        fgetc(fp);
    }
    
    int i, j;
    
    if (is_pbm_ascii) {
        /* P1: PBM ASCII - 0 = blanc, 1 = noir */
        for (i = 0; i < IMG_SIZE; i++) {
            for (j = 0; j < IMG_SIZE; j++) {
                int pixel;
                skip_whitespace_and_comments(fp);
                if (fscanf(fp, "%d", &pixel) != 1) {
                    printf("Erreur de lecture pixel (%d, %d)\n", i, j);
                    fclose(fp);
                    return -1;
                }
                /* PBM: 1 = noir (0.0), 0 = blanc (1.0) */
                img[i][j] = (pixel == 0) ? 1.0f : 0.0f;
            }
        }
    } else if (is_pbm_binary) {
        /* P4: PBM binaire - bits packés */
        int bytes_per_row = (IMG_SIZE + 7) / 8;
        unsigned char row_buffer[16]; /* Suffisant pour 50 pixels = 7 octets */
        
        for (i = 0; i < IMG_SIZE; i++) {
            if (fread(row_buffer, 1, bytes_per_row, fp) != (size_t)bytes_per_row) {
                printf("Erreur de lecture ligne %d\n", i);
                fclose(fp);
                return -1;
            }
            for (j = 0; j < IMG_SIZE; j++) {
                int byte_idx = j / 8;
                int bit_idx = 7 - (j % 8);
                int pixel = (row_buffer[byte_idx] >> bit_idx) & 1;
                /* PBM: 1 = noir (0.0), 0 = blanc (1.0) */
                img[i][j] = (pixel == 0) ? 1.0f : 0.0f;
            }
        }
    } else if (is_pgm_ascii) {
        /* P2: PGM ASCII */
        for (i = 0; i < IMG_SIZE; i++) {
            for (j = 0; j < IMG_SIZE; j++) {
                int pixel;
                skip_whitespace_and_comments(fp);
                if (fscanf(fp, "%d", &pixel) != 1) {
                    printf("Erreur de lecture pixel (%d, %d)\n", i, j);
                    fclose(fp);
                    return -1;
                }
                img[i][j] = (float)pixel / (float)maxval;
            }
        }
    } else if (is_pgm_binary) {
        /* P5: PGM binaire */
        unsigned char pixel_buffer[IMG_SIZE];
        for (i = 0; i < IMG_SIZE; i++) {
            if (fread(pixel_buffer, 1, IMG_SIZE, fp) != IMG_SIZE) {
                printf("Erreur de lecture ligne %d\n", i);
                fclose(fp);
                return -1;
            }
            for (j = 0; j < IMG_SIZE; j++) {
                img[i][j] = (float)pixel_buffer[j] / (float)maxval;
            }
        }
    }
    
    fclose(fp);
    return 0;
}

/* ============================================================
 * PROPAGATION AVANT
 * ============================================================ */

void forward(float input[IMG_SIZE][IMG_SIZE]) {
    int f, c, i, j, ki, kj, pi, pj;
    float sum, max_val;
    int max_i, max_j;
    
    /* Copier l'entrée */
    for (i = 0; i < IMG_SIZE; i++) {
        for (j = 0; j < IMG_SIZE; j++) {
            cache.input[i][j] = input[i][j];
        }
    }
    
    /* ========== CONV1 ========== */
    int conv1_out_size = IMG_SIZE - CONV1_SIZE + 1;
    for (f = 0; f < CONV1_FILTERS; f++) {
        for (i = 0; i < conv1_out_size; i++) {
            for (j = 0; j < conv1_out_size; j++) {
                sum = network.conv1_bias[f];
                for (ki = 0; ki < CONV1_SIZE; ki++) {
                    for (kj = 0; kj < CONV1_SIZE; kj++) {
                        sum += cache.input[i + ki][j + kj] * network.conv1_weights[f][ki][kj];
                    }
                }
                cache.conv1_out[f][i][j] = sum;
                cache.relu1_out[f][i][j] = relu(sum);
            }
        }
    }
    
    /* ========== POOL1 (Max Pooling 2x2) ========== */
    for (f = 0; f < CONV1_FILTERS; f++) {
        for (i = 0; i < AFTER_POOL1; i++) {
            for (j = 0; j < AFTER_POOL1; j++) {
                max_val = -1e10f;
                max_i = 0;
                max_j = 0;
                for (pi = 0; pi < POOL1_SIZE; pi++) {
                    for (pj = 0; pj < POOL1_SIZE; pj++) {
                        int ii = i * POOL1_SIZE + pi;
                        int jj = j * POOL1_SIZE + pj;
                        if (ii < conv1_out_size && jj < conv1_out_size) {
                            if (cache.relu1_out[f][ii][jj] > max_val) {
                                max_val = cache.relu1_out[f][ii][jj];
                                max_i = ii;
                                max_j = jj;
                            }
                        }
                    }
                }
                cache.pool1_out[f][i][j] = max_val;
                cache.pool1_max_i[f][i][j] = max_i;
                cache.pool1_max_j[f][i][j] = max_j;
            }
        }
    }
    
    /* ========== CONV2 ========== */
    int conv2_out_size = AFTER_POOL1 - CONV2_SIZE + 1;
    for (f = 0; f < CONV2_FILTERS; f++) {
        for (i = 0; i < conv2_out_size; i++) {
            for (j = 0; j < conv2_out_size; j++) {
                sum = network.conv2_bias[f];
                for (c = 0; c < CONV1_FILTERS; c++) {
                    for (ki = 0; ki < CONV2_SIZE; ki++) {
                        for (kj = 0; kj < CONV2_SIZE; kj++) {
                            sum += cache.pool1_out[c][i + ki][j + kj] * 
                                   network.conv2_weights[f][c][ki][kj];
                        }
                    }
                }
                cache.conv2_out[f][i][j] = sum;
                cache.relu2_out[f][i][j] = relu(sum);
            }
        }
    }
    
    /* ========== POOL2 ========== */
    for (f = 0; f < CONV2_FILTERS; f++) {
        for (i = 0; i < AFTER_POOL2; i++) {
            for (j = 0; j < AFTER_POOL2; j++) {
                max_val = -1e10f;
                max_i = 0;
                max_j = 0;
                for (pi = 0; pi < POOL1_SIZE; pi++) {
                    for (pj = 0; pj < POOL1_SIZE; pj++) {
                        int ii = i * POOL1_SIZE + pi;
                        int jj = j * POOL1_SIZE + pj;
                        if (ii < conv2_out_size && jj < conv2_out_size) {
                            if (cache.relu2_out[f][ii][jj] > max_val) {
                                max_val = cache.relu2_out[f][ii][jj];
                                max_i = ii;
                                max_j = jj;
                            }
                        }
                    }
                }
                cache.pool2_out[f][i][j] = max_val;
                cache.pool2_max_i[f][i][j] = max_i;
                cache.pool2_max_j[f][i][j] = max_j;
            }
        }
    }
    
    /* ========== FLATTEN ========== */
    int idx = 0;
    for (f = 0; f < CONV2_FILTERS; f++) {
        for (i = 0; i < AFTER_POOL2; i++) {
            for (j = 0; j < AFTER_POOL2; j++) {
                cache.flatten[idx++] = cache.pool2_out[f][i][j];
            }
        }
    }
    
    /* ========== FC1 ========== */
    for (i = 0; i < FC1_SIZE; i++) {
        sum = network.fc1_bias[i];
        for (j = 0; j < FLATTEN_SIZE; j++) {
            sum += cache.flatten[j] * network.fc1_weights[i][j];
        }
        cache.fc1_out[i] = sum;
        cache.relu3_out[i] = relu(sum);
    }
    
    /* ========== FC2 ========== */
    for (i = 0; i < FC2_SIZE; i++) {
        sum = network.fc2_bias[i];
        for (j = 0; j < FC1_SIZE; j++) {
            sum += cache.relu3_out[j] * network.fc2_weights[i][j];
        }
        cache.fc2_out[i] = sum;
    }
    
    /* ========== SOFTMAX ========== */
    float max_logit = cache.fc2_out[0];
    for (i = 1; i < NUM_CLASSES; i++) {
        if (cache.fc2_out[i] > max_logit) max_logit = cache.fc2_out[i];
    }
    
    float exp_sum = 0.0f;
    for (i = 0; i < NUM_CLASSES; i++) {
        cache.softmax_out[i] = my_exp(cache.fc2_out[i] - max_logit);
        exp_sum += cache.softmax_out[i];
    }
    for (i = 0; i < NUM_CLASSES; i++) {
        cache.softmax_out[i] /= exp_sum;
        if (cache.softmax_out[i] < 1e-7f) cache.softmax_out[i] = 1e-7f;
    }
}

/* ============================================================
 * PROPAGATION ARRIÈRE
 * ============================================================ */

void zero_gradients(void) {
    int f, c, i, j;
    
    for (f = 0; f < CONV1_FILTERS; f++) {
        for (i = 0; i < CONV1_SIZE; i++) {
            for (j = 0; j < CONV1_SIZE; j++) {
                grads.conv1_weights[f][i][j] = 0.0f;
            }
        }
        grads.conv1_bias[f] = 0.0f;
    }
    
    for (f = 0; f < CONV2_FILTERS; f++) {
        for (c = 0; c < CONV1_FILTERS; c++) {
            for (i = 0; i < CONV2_SIZE; i++) {
                for (j = 0; j < CONV2_SIZE; j++) {
                    grads.conv2_weights[f][c][i][j] = 0.0f;
                }
            }
        }
        grads.conv2_bias[f] = 0.0f;
    }
    
    for (i = 0; i < FC1_SIZE; i++) {
        for (j = 0; j < FLATTEN_SIZE; j++) {
            grads.fc1_weights[i][j] = 0.0f;
        }
        grads.fc1_bias[i] = 0.0f;
    }
    
    for (i = 0; i < FC2_SIZE; i++) {
        for (j = 0; j < FC1_SIZE; j++) {
            grads.fc2_weights[i][j] = 0.0f;
        }
        grads.fc2_bias[i] = 0.0f;
    }
}

void backward(int label) {
    int f, c, i, j, ki, kj;
    
    /* Gradient de la loss (Cross-Entropy + Softmax) */
    float d_fc2_out[FC2_SIZE];
    for (i = 0; i < FC2_SIZE; i++) {
        d_fc2_out[i] = cache.softmax_out[i];
        if (i == label) d_fc2_out[i] -= 1.0f;
    }
    
    /* ========== Gradients FC2 ========== */
    for (i = 0; i < FC2_SIZE; i++) {
        grads.fc2_bias[i] += d_fc2_out[i];
        for (j = 0; j < FC1_SIZE; j++) {
            grads.fc2_weights[i][j] += d_fc2_out[i] * cache.relu3_out[j];
        }
    }
    
    /* Gradient vers relu3_out */
    float d_relu3[FC1_SIZE];
    for (j = 0; j < FC1_SIZE; j++) {
        d_relu3[j] = 0.0f;
        for (i = 0; i < FC2_SIZE; i++) {
            d_relu3[j] += d_fc2_out[i] * network.fc2_weights[i][j];
        }
    }
    
    /* Gradient à travers ReLU */
    float d_fc1_out[FC1_SIZE];
    for (i = 0; i < FC1_SIZE; i++) {
        d_fc1_out[i] = d_relu3[i] * relu_derivative(cache.fc1_out[i]);
    }
    
    /* ========== Gradients FC1 ========== */
    for (i = 0; i < FC1_SIZE; i++) {
        grads.fc1_bias[i] += d_fc1_out[i];
        for (j = 0; j < FLATTEN_SIZE; j++) {
            grads.fc1_weights[i][j] += d_fc1_out[i] * cache.flatten[j];
        }
    }
    
    /* Gradient vers flatten */
    float d_flatten[FLATTEN_SIZE];
    for (j = 0; j < FLATTEN_SIZE; j++) {
        d_flatten[j] = 0.0f;
        for (i = 0; i < FC1_SIZE; i++) {
            d_flatten[j] += d_fc1_out[i] * network.fc1_weights[i][j];
        }
    }
    
    /* ========== Déflatten vers pool2_out ========== */
    float d_pool2[CONV2_FILTERS][AFTER_POOL2][AFTER_POOL2];
    int idx = 0;
    for (f = 0; f < CONV2_FILTERS; f++) {
        for (i = 0; i < AFTER_POOL2; i++) {
            for (j = 0; j < AFTER_POOL2; j++) {
                d_pool2[f][i][j] = d_flatten[idx++];
            }
        }
    }
    
    /* ========== Gradient à travers Pool2 ========== */
    int conv2_out_size = AFTER_POOL1 - CONV2_SIZE + 1;
    float d_relu2[CONV2_FILTERS][AFTER_POOL1 - CONV2_SIZE + 1][AFTER_POOL1 - CONV2_SIZE + 1];
    for (f = 0; f < CONV2_FILTERS; f++) {
        for (i = 0; i < conv2_out_size; i++) {
            for (j = 0; j < conv2_out_size; j++) {
                d_relu2[f][i][j] = 0.0f;
            }
        }
    }
    
    for (f = 0; f < CONV2_FILTERS; f++) {
        for (i = 0; i < AFTER_POOL2; i++) {
            for (j = 0; j < AFTER_POOL2; j++) {
                int mi = cache.pool2_max_i[f][i][j];
                int mj = cache.pool2_max_j[f][i][j];
                d_relu2[f][mi][mj] += d_pool2[f][i][j];
            }
        }
    }
    
    /* Gradient à travers ReLU2 */
    float d_conv2[CONV2_FILTERS][AFTER_POOL1 - CONV2_SIZE + 1][AFTER_POOL1 - CONV2_SIZE + 1];
    for (f = 0; f < CONV2_FILTERS; f++) {
        for (i = 0; i < conv2_out_size; i++) {
            for (j = 0; j < conv2_out_size; j++) {
                d_conv2[f][i][j] = d_relu2[f][i][j] * relu_derivative(cache.conv2_out[f][i][j]);
            }
        }
    }
    
    /* ========== Gradients Conv2 ========== */
    for (f = 0; f < CONV2_FILTERS; f++) {
        for (i = 0; i < conv2_out_size; i++) {
            for (j = 0; j < conv2_out_size; j++) {
                grads.conv2_bias[f] += d_conv2[f][i][j];
                for (c = 0; c < CONV1_FILTERS; c++) {
                    for (ki = 0; ki < CONV2_SIZE; ki++) {
                        for (kj = 0; kj < CONV2_SIZE; kj++) {
                            grads.conv2_weights[f][c][ki][kj] += 
                                d_conv2[f][i][j] * cache.pool1_out[c][i + ki][j + kj];
                        }
                    }
                }
            }
        }
    }
    
    /* Gradient vers pool1_out */
    float d_pool1[CONV1_FILTERS][AFTER_POOL1][AFTER_POOL1];
    for (c = 0; c < CONV1_FILTERS; c++) {
        for (i = 0; i < AFTER_POOL1; i++) {
            for (j = 0; j < AFTER_POOL1; j++) {
                d_pool1[c][i][j] = 0.0f;
            }
        }
    }
    
    for (f = 0; f < CONV2_FILTERS; f++) {
        for (i = 0; i < conv2_out_size; i++) {
            for (j = 0; j < conv2_out_size; j++) {
                for (c = 0; c < CONV1_FILTERS; c++) {
                    for (ki = 0; ki < CONV2_SIZE; ki++) {
                        for (kj = 0; kj < CONV2_SIZE; kj++) {
                            d_pool1[c][i + ki][j + kj] += 
                                d_conv2[f][i][j] * network.conv2_weights[f][c][ki][kj];
                        }
                    }
                }
            }
        }
    }
    
    /* ========== Gradient à travers Pool1 ========== */
    int conv1_out_size = IMG_SIZE - CONV1_SIZE + 1;
    float d_relu1[CONV1_FILTERS][IMG_SIZE - CONV1_SIZE + 1][IMG_SIZE - CONV1_SIZE + 1];
    for (f = 0; f < CONV1_FILTERS; f++) {
        for (i = 0; i < conv1_out_size; i++) {
            for (j = 0; j < conv1_out_size; j++) {
                d_relu1[f][i][j] = 0.0f;
            }
        }
    }
    
    for (f = 0; f < CONV1_FILTERS; f++) {
        for (i = 0; i < AFTER_POOL1; i++) {
            for (j = 0; j < AFTER_POOL1; j++) {
                int mi = cache.pool1_max_i[f][i][j];
                int mj = cache.pool1_max_j[f][i][j];
                d_relu1[f][mi][mj] += d_pool1[f][i][j];
            }
        }
    }
    
    /* Gradient à travers ReLU1 */
    float d_conv1[CONV1_FILTERS][IMG_SIZE - CONV1_SIZE + 1][IMG_SIZE - CONV1_SIZE + 1];
    for (f = 0; f < CONV1_FILTERS; f++) {
        for (i = 0; i < conv1_out_size; i++) {
            for (j = 0; j < conv1_out_size; j++) {
                d_conv1[f][i][j] = d_relu1[f][i][j] * relu_derivative(cache.conv1_out[f][i][j]);
            }
        }
    }
    
    /* ========== Gradients Conv1 ========== */
    for (f = 0; f < CONV1_FILTERS; f++) {
        for (i = 0; i < conv1_out_size; i++) {
            for (j = 0; j < conv1_out_size; j++) {
                grads.conv1_bias[f] += d_conv1[f][i][j];
                for (ki = 0; ki < CONV1_SIZE; ki++) {
                    for (kj = 0; kj < CONV1_SIZE; kj++) {
                        grads.conv1_weights[f][ki][kj] += 
                            d_conv1[f][i][j] * cache.input[i + ki][j + kj];
                    }
                }
            }
        }
    }
}

void update_weights(float lr) {
    int f, c, i, j;
    
    for (f = 0; f < CONV1_FILTERS; f++) {
        for (i = 0; i < CONV1_SIZE; i++) {
            for (j = 0; j < CONV1_SIZE; j++) {
                network.conv1_weights[f][i][j] -= lr * grads.conv1_weights[f][i][j];
            }
        }
        network.conv1_bias[f] -= lr * grads.conv1_bias[f];
    }
    
    for (f = 0; f < CONV2_FILTERS; f++) {
        for (c = 0; c < CONV1_FILTERS; c++) {
            for (i = 0; i < CONV2_SIZE; i++) {
                for (j = 0; j < CONV2_SIZE; j++) {
                    network.conv2_weights[f][c][i][j] -= lr * grads.conv2_weights[f][c][i][j];
                }
            }
        }
        network.conv2_bias[f] -= lr * grads.conv2_bias[f];
    }
    
    for (i = 0; i < FC1_SIZE; i++) {
        for (j = 0; j < FLATTEN_SIZE; j++) {
            network.fc1_weights[i][j] -= lr * grads.fc1_weights[i][j];
        }
        network.fc1_bias[i] -= lr * grads.fc1_bias[i];
    }
    
    for (i = 0; i < FC2_SIZE; i++) {
        for (j = 0; j < FC1_SIZE; j++) {
            network.fc2_weights[i][j] -= lr * grads.fc2_weights[i][j];
        }
        network.fc2_bias[i] -= lr * grads.fc2_bias[i];
    }
}

/* ============================================================
 * SAUVEGARDE ET CHARGEMENT
 * ============================================================ */

int save_network(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("Erreur: impossible de sauvegarder dans %s\n", filename);
        return -1;
    }
    
    int f, c, i, j;
    
    fprintf(fp, "CNN_MODEL_V1\n");
    
    /* Conv1 */
    for (f = 0; f < CONV1_FILTERS; f++) {
        for (i = 0; i < CONV1_SIZE; i++) {
            for (j = 0; j < CONV1_SIZE; j++) {
                fprintf(fp, "%.8f\n", network.conv1_weights[f][i][j]);
            }
        }
        fprintf(fp, "%.8f\n", network.conv1_bias[f]);
    }
    
    /* Conv2 */
    for (f = 0; f < CONV2_FILTERS; f++) {
        for (c = 0; c < CONV1_FILTERS; c++) {
            for (i = 0; i < CONV2_SIZE; i++) {
                for (j = 0; j < CONV2_SIZE; j++) {
                    fprintf(fp, "%.8f\n", network.conv2_weights[f][c][i][j]);
                }
            }
        }
        fprintf(fp, "%.8f\n", network.conv2_bias[f]);
    }
    
    /* FC1 */
    for (i = 0; i < FC1_SIZE; i++) {
        for (j = 0; j < FLATTEN_SIZE; j++) {
            fprintf(fp, "%.8f\n", network.fc1_weights[i][j]);
        }
        fprintf(fp, "%.8f\n", network.fc1_bias[i]);
    }
    
    /* FC2 */
    for (i = 0; i < FC2_SIZE; i++) {
        for (j = 0; j < FC1_SIZE; j++) {
            fprintf(fp, "%.8f\n", network.fc2_weights[i][j]);
        }
        fprintf(fp, "%.8f\n", network.fc2_bias[i]);
    }
    
    fclose(fp);
    printf("Modèle sauvegardé dans %s\n", filename);
    return 0;
}

int load_network(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("Erreur: impossible de charger %s\n", filename);
        return -1;
    }
    
    char header[32];
    if (fscanf(fp, "%31s", header) != 1) {
        fclose(fp);
        return -1;
    }
    
    int f, c, i, j;
    
    /* Conv1 */
    for (f = 0; f < CONV1_FILTERS; f++) {
        for (i = 0; i < CONV1_SIZE; i++) {
            for (j = 0; j < CONV1_SIZE; j++) {
                if (fscanf(fp, "%f", &network.conv1_weights[f][i][j]) != 1) {
                    fclose(fp);
                    return -1;
                }
            }
        }
        if (fscanf(fp, "%f", &network.conv1_bias[f]) != 1) {
            fclose(fp);
            return -1;
        }
    }
    
    /* Conv2 */
    for (f = 0; f < CONV2_FILTERS; f++) {
        for (c = 0; c < CONV1_FILTERS; c++) {
            for (i = 0; i < CONV2_SIZE; i++) {
                for (j = 0; j < CONV2_SIZE; j++) {
                    if (fscanf(fp, "%f", &network.conv2_weights[f][c][i][j]) != 1) {
                        fclose(fp);
                        return -1;
                    }
                }
            }
        }
        if (fscanf(fp, "%f", &network.conv2_bias[f]) != 1) {
            fclose(fp);
            return -1;
        }
    }
    
    /* FC1 */
    for (i = 0; i < FC1_SIZE; i++) {
        for (j = 0; j < FLATTEN_SIZE; j++) {
            if (fscanf(fp, "%f", &network.fc1_weights[i][j]) != 1) {
                fclose(fp);
                return -1;
            }
        }
        if (fscanf(fp, "%f", &network.fc1_bias[i]) != 1) {
            fclose(fp);
            return -1;
        }
    }
    
    /* FC2 */
    for (i = 0; i < FC2_SIZE; i++) {
        for (j = 0; j < FC1_SIZE; j++) {
            if (fscanf(fp, "%f", &network.fc2_weights[i][j]) != 1) {
                fclose(fp);
                return -1;
            }
        }
        if (fscanf(fp, "%f", &network.fc2_bias[i]) != 1) {
            fclose(fp);
            return -1;
        }
    }
    
    fclose(fp);
    printf("Modèle chargé depuis %s\n", filename);
    return 0;
}

/* ============================================================
 * ENTRAÎNEMENT
 * ============================================================ */

void shuffle_indices(int *indices, int n) {
    int i, j, tmp;
    for (i = n - 1; i > 0; i--) {
        j = (int)(my_rand() * (i + 1));
        if (j > i) j = i;
        tmp = indices[i];
        indices[i] = indices[j];
        indices[j] = tmp;
    }
}

void train(const char *data_dir) {
    float img[IMG_SIZE][IMG_SIZE];
    char path[256];
    int epoch, letter, sample, i;
    int total_samples = NUM_CLASSES * SAMPLES_PER_LETTER;
    int indices[NUM_CLASSES * SAMPLES_PER_LETTER];
    
    printf("=== ENTRAÎNEMENT DU CNN ===\n");
    printf("Architecture: Conv(8x5x5) -> Pool(2x2) -> Conv(16x3x3) -> Pool(2x2) -> FC(128) -> FC(26)\n");
    printf("Données: %d lettres x %d échantillons = %d images\n", NUM_CLASSES, SAMPLES_PER_LETTER, total_samples);
    printf("Époques: %d, Learning rate: %.4f\n\n", EPOCHS, LEARNING_RATE);
    
    init_network();
    
    /* Créer les indices pour le mélange */
    for (i = 0; i < total_samples; i++) {
        indices[i] = i;
    }
    
    for (epoch = 0; epoch < EPOCHS; epoch++) {
        float total_loss = 0.0f;
        int correct = 0;
        
        /* Mélanger les données */
        shuffle_indices(indices, total_samples);
        
        for (i = 0; i < total_samples; i++) {
            int idx = indices[i];
            letter = idx / SAMPLES_PER_LETTER;
            sample = idx % SAMPLES_PER_LETTER;
            
            /* Construire le chemin - maintenant avec .pbm */
            sprintf(path, "%s/%c/%c_%03d.pbm", data_dir, 'A' + letter, 'A' + letter, sample);
            
            if (read_pbm(path, img) != 0) {
                continue;
            }
            
            /* Forward */
            forward(img);
            
            /* Calculer la loss */
            float loss = -my_log(cache.softmax_out[letter]);
            total_loss += loss;
            
            /* Vérifier la prédiction */
            int pred = 0;
            float max_prob = cache.softmax_out[0];
            int k;
            for (k = 1; k < NUM_CLASSES; k++) {
                if (cache.softmax_out[k] > max_prob) {
                    max_prob = cache.softmax_out[k];
                    pred = k;
                }
            }
            if (pred == letter) correct++;
            
            /* Backward */
            zero_gradients();
            backward(letter);
            update_weights(LEARNING_RATE);
            
            /* Affichage de progression */
            if ((i + 1) % 200 == 0) {
                printf("\r  Époque %d/%d: %d/%d échantillons traités...", 
                       epoch + 1, EPOCHS, i + 1, total_samples);
                fflush(stdout);
            }
        }
        
        float avg_loss = total_loss / (float)total_samples;
        float accuracy = 100.0f * (float)correct / (float)total_samples;
        printf("\r  Époque %d/%d: Loss = %.4f, Accuracy = %.2f%%                    \n", 
               epoch + 1, EPOCHS, avg_loss, accuracy);
    }
    
    printf("\nEntraînement terminé!\n");
    save_network("model.txt");
}

/* ============================================================
 * PRÉDICTION
 * ============================================================ */

char predict(float img[IMG_SIZE][IMG_SIZE]) {
    forward(img);
    
    int pred = 0;
    float max_prob = cache.softmax_out[0];
    int i;
    for (i = 1; i < NUM_CLASSES; i++) {
        if (cache.softmax_out[i] > max_prob) {
            max_prob = cache.softmax_out[i];
            pred = i;
        }
    }
    
    return 'A' + pred;
}

void test_image(const char *image_path) {
    float img[IMG_SIZE][IMG_SIZE];
    
    if (load_network("model.txt") != 0) {
        printf("Erreur: impossible de charger le modèle. Entraînez d'abord avec l'option 1.\n");
        return;
    }
    
    if (read_pbm(image_path, img) != 0) {
        printf("Erreur: impossible de lire l'image %s\n", image_path);
        return;
    }
    
    char result = predict(img);
    
    printf("\n=== RÉSULTAT DE LA PRÉDICTION ===\n");
    printf("Image: %s\n", image_path);
    printf("Lettre prédite: %c\n\n", result);
    
    printf("Probabilités (top 5):\n");
    
    /* Trouver les 5 meilleures probabilités */
    int sorted[NUM_CLASSES];
    int i, j;
    for (i = 0; i < NUM_CLASSES; i++) sorted[i] = i;
    
    for (i = 0; i < 5; i++) {
        for (j = i + 1; j < NUM_CLASSES; j++) {
            if (cache.softmax_out[sorted[j]] > cache.softmax_out[sorted[i]]) {
                int tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
        printf("  %c: %.2f%%\n", 'A' + sorted[i], cache.softmax_out[sorted[i]] * 100.0f);
    }
}

/* ============================================================
 * MAIN
 * ============================================================ */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage:\n");
        printf("  %s 1          - Entraîner le modèle\n", argv[0]);
        printf("  %s 2          - Tester une image\n", argv[0]);
        return 1;
    }
    
    int mode = argv[1][0] - '0';
    
    if (mode == 1) {
        train("letters_50x50_fonts");
    } else if (mode == 2) {
        char path[256];
        printf("Entrez le chemin de l'image à tester: ");
        if (scanf("%255s", path) == 1) {
            test_image(path);
        } else {
            printf("Erreur de lecture du chemin.\n");
        }
    } else {
        printf("Mode invalide. Utilisez 1 pour entraîner ou 2 pour tester.\n");
        return 1;
    }
    
    return 0;
}
