#include <stdio.h>

#define MAX_PATH 1024
#define MAX_COORDS 100
#define CROSS_SIZE 15
#define LINE_THICKNESS 5
#define HEAP_SIZE 10000000

typedef struct {
    int col_start, row_start;
    int col_end, row_end;
} Coordinate;

typedef struct {
    int width;
    int height;
    unsigned char *data;
} PBMImage;

// Heap statique pour allocation mémoire
static char g_heap[HEAP_SIZE];
static unsigned long g_heap_used = 0;

// Fonctions utilitaires pour remplacer stdlib.h
int my_abs(int x) {
    return x < 0 ? -x : x;
}

void *my_malloc(unsigned long size) {
    if (g_heap_used + size > HEAP_SIZE) {
        return NULL;
    }
    
    void *ptr = &g_heap[g_heap_used];
    g_heap_used += size;
    return ptr;
}

void *my_calloc(unsigned long count, unsigned long size) {
    unsigned long total = count * size;
    unsigned char *ptr = my_malloc(total);
    if (ptr) {
        for (unsigned long i = 0; i < total; i++) {
            ptr[i] = 0;
        }
    }
    return ptr;
}

// Fonctions utilitaires pour remplacer string.h
int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

// Construit un chemin avec un numéro formaté sur 2 chiffres
void build_path(char *dest, const char *base, const char *prefix, int num, const char *suffix) {
    char *p = dest;
    const char *s;
    
    // Copier base
    for (s = base; *s; s++) *p++ = *s;
    *p++ = '/';
    
    // Copier prefix
    for (s = prefix; *s; s++) *p++ = *s;
    
    // Ajouter le numéro sur 2 chiffres
    *p++ = '0' + (num / 10);
    *p++ = '0' + (num % 10);
    
    // Copier suffix
    for (s = suffix; *s; s++) *p++ = *s;
    
    *p = '\0';
}

// Trouve le nombre max en essayant d'ouvrir des fichiers séquentiellement
int find_max_index(const char *base_path, const char *prefix, const char *suffix) {
    char path[MAX_PATH];
    int index = 0;
    
    while (index < 100) {
        build_path(path, base_path, prefix, index, suffix);
        
        FILE *f = fopen(path, "r");
        if (!f) {
            break;
        }
        fclose(f);
        index++;
    }
    
    return index - 1;
}

// Lit une image PBM (P1 format ASCII ou P4 binaire)
PBMImage* read_pbm(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Erreur: impossible d'ouvrir %s\n", filename);
        return NULL;
    }
    
    PBMImage *img = my_malloc(sizeof(PBMImage));
    if (!img) {
        fclose(file);
        return NULL;
    }
    
    char magic[3];
    if (fscanf(file, "%2s", magic) != 1) {
        fclose(file);
        return NULL;
    }
    
    // Ignorer les commentaires
    int c;
    while ((c = fgetc(file)) != EOF) {
        if (c == '#') {
            while ((c = fgetc(file)) != EOF && c != '\n');
        } else if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            ungetc(c, file);
            break;
        }
    }
    
    if (fscanf(file, "%d %d", &img->width, &img->height) != 2) {
        fclose(file);
        return NULL;
    }
    
    img->data = my_calloc(img->width * img->height, sizeof(unsigned char));
    if (!img->data) {
        fclose(file);
        return NULL;
    }
    
    if (my_strcmp(magic, "P1") == 0) {
        // Format ASCII
        for (int i = 0; i < img->width * img->height; i++) {
            int val;
            if (fscanf(file, "%d", &val) == 1) {
                img->data[i] = val ? 1 : 0;
            }
        }
    } else if (my_strcmp(magic, "P4") == 0) {
        // Format binaire - passer le whitespace
        fgetc(file);
        
        int row_bytes = (img->width + 7) / 8;
        
        for (int y = 0; y < img->height; y++) {
            for (int b = 0; b < row_bytes; b++) {
                int byte = fgetc(file);
                if (byte == EOF) break;
                for (int bit = 0; bit < 8 && (b * 8 + bit) < img->width; bit++) {
                    int x = b * 8 + bit;
                    img->data[y * img->width + x] = (byte >> (7 - bit)) & 1;
                }
            }
        }
    }
    
    fclose(file);
    return img;
}

// Écrit une image PBM en format ASCII (P1)
int write_pbm(const char *filename, PBMImage *img) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Erreur: impossible de creer %s\n", filename);
        return 0;
    }
    
    fprintf(file, "P1\n");
    fprintf(file, "%d %d\n", img->width, img->height);
    
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            fprintf(file, "%d ", img->data[y * img->width + x]);
        }
        fprintf(file, "\n");
    }
    
    fclose(file);
    return 1;
}

// Dessine une croix épaisse sur l'image à la position (px, py)
void draw_cross(PBMImage *img, int px, int py, int size) {
    for (int i = -size; i <= size; i++) {
        for (int t = -LINE_THICKNESS; t <= LINE_THICKNESS; t++) {
            // Ligne horizontale
            int x = px + i;
            int y = py + t;
            if (x >= 0 && x < img->width && y >= 0 && y < img->height) {
                img->data[y * img->width + x] = 1;
            }
            // Ligne verticale
            x = px + t;
            y = py + i;
            if (x >= 0 && x < img->width && y >= 0 && y < img->height) {
                img->data[y * img->width + x] = 1;
            }
        }
    }
}

// Dessine un pixel épais
void draw_thick_pixel(PBMImage *img, int px, int py) {
    for (int dx = -LINE_THICKNESS; dx <= LINE_THICKNESS; dx++) {
        for (int dy = -LINE_THICKNESS; dy <= LINE_THICKNESS; dy++) {
            int x = px + dx;
            int y = py + dy;
            if (x >= 0 && x < img->width && y >= 0 && y < img->height) {
                img->data[y * img->width + x] = 1;
            }
        }
    }
}

// Dessine une ligne épaisse entre deux points (algorithme de Bresenham)
void draw_line(PBMImage *img, int x0, int y0, int x1, int y1) {
    int dx = my_abs(x1 - x0);
    int dy = my_abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        draw_thick_pixel(img, x0, y0);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Lit les coordonnées depuis le fichier
int read_coordinates(const char *filename, Coordinate *coords, int max_coords) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erreur: impossible d'ouvrir %s\n", filename);
        return 0;
    }
    
    int count = 0;
    while (count < max_coords &&
           fscanf(file, "%d,%d %d,%d",
                  &coords[count].col_start, &coords[count].row_start,
                  &coords[count].col_end, &coords[count].row_end) == 4) {
        count++;
    }
    
    fclose(file);
    return count;
}

// Concatène deux chaînes
void concat_path(char *dest, const char *s1, const char *s2) {
    char *p = dest;
    while (*s1) *p++ = *s1++;
    while (*s2) *p++ = *s2++;
    *p = '\0';
}

