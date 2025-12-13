#ifndef DECOUPE_H
#define DECOUPE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <math.h>
#include <SDL.h>

/* =============================================================================
   STRUCTURES DE DONNÉES
   ============================================================================= */

typedef struct {
    unsigned char *data;  // 0=blanc, 1=noir
    int width;
    int height;
} Image;

typedef struct {
    int x, y;
    int width, height;
} Rectangle;

typedef struct {
    int start;
    int end;
} Segment;

typedef struct {
    Rectangle *cells;
    int num_cells;
    int rows;
    int cols;
} GridCells;

/* =============================================================================
   CONSTANTES
   ============================================================================= */

// Histogrammes
#define HIST_NOISE_TOLERANCE    1

// Gaps pour détection
#define GAP_BLOCKS              25
#define MERGE_GAP_THRESHOLD     60

// Dimensions minimales
#define MIN_BLOCK_WIDTH         70
#define MIN_BLOCK_HEIGHT        70

// Filtrage nettoyage
#define DRAWING_MIN_SURFACE     15000
#define DRAWING_MIN_DENSITY     0.45

// Padding cellules/caractères
#define PADDING_X               2
#define PADDING_Y               2

// Taille de normalisation
#define TARGET_SIZE             50

// Seuil bordures (90% de pixels noirs consécutifs = bordure)
#define BORDER_CONTINUITY_THRESHOLD  0.90

/* =============================================================================
   MODULE 1 : GESTION DES IMAGES (decoupe.c)
   ============================================================================= */

int         create_dir_recursively(const char *path);
Image       load_bmp_to_image(const char *filepath);
int         write_pbm(const Image *img, const char *filepath);
Image       create_sub_image(const Image *source, Rectangle rect);
Image       copy_image(const Image *source);
void        free_image(Image *img);

/* =============================================================================
   MODULE 2 : DÉTECTION GRILLE/LISTE (decoupe.c)
   ============================================================================= */

Rectangle*  find_all_components(const Image *img, int *num_blocks);
Image       clean_image(const Image *original, Rectangle *blocks, int num_blocks);
Rectangle*  detect_grid_and_list(const Image *img, int *num_blocks);
int         is_likely_grid(Rectangle rect);

/* =============================================================================
   MODULE 3 : NETTOYAGE BORDURES (decoupe.c)
   ============================================================================= */

Image       remove_grid_frame(const Image *grid_img);

/* =============================================================================
   MODULE 4 : SEGMENTATION LETTRES (decoupe_lettre.c)
   ============================================================================= */

GridCells   segment_grid_cells(const Image *grid_img);
Rectangle*  segment_word_lines(const Image *list_img, int *num_lines);
Rectangle*  segment_line_characters(const Image *line_img, int *num_chars);
int         save_all_grid_cells(const Image *grid_img, GridCells cells, const char *base_dir);
int         save_all_words(const Image *list_img, Rectangle *lines, int num_lines, const char *base_dir);

#endif