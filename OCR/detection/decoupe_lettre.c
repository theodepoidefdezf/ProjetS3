#include "decoupe.h"

/* =============================================================================
   SECTION 1 : FONCTIONS UTILITAIRES
   ============================================================================= */

static int *calculate_histogram(const Image *img, int horizontal) {
    int size = horizontal ? img->height : img->width;
    int *hist = (int *)calloc(size, sizeof(int));
    if (!hist) return NULL;
    
    if (horizontal) {
        for (int y = 0; y < img->height; y++) {
            for (int x = 0; x < img->width; x++) {
                if (img->data[y * img->width + x] == 1) hist[y]++;
            }
        }
    } else {
        for (int x = 0; x < img->width; x++) {
            for (int y = 0; y < img->height; y++) {
                if (img->data[y * img->width + x] == 1) hist[x]++;
            }
        }
    }
    return hist;
}

static Rectangle apply_padding(Rectangle rect, int pad_x, int pad_y, int img_w, int img_h) {
    Rectangle p = {
        rect.x - pad_x,
        rect.y - pad_y,
        rect.width + 2 * pad_x,
        rect.height + 2 * pad_y
    };
    
    if (p.x < 0) { p.width += p.x; p.x = 0; }
    if (p.y < 0) { p.height += p.y; p.y = 0; }
    if (p.x + p.width > img_w) p.width = img_w - p.x;
    if (p.y + p.height > img_h) p.height = img_h - p.y;
    if (p.width < 1) p.width = 1;
    if (p.height < 1) p.height = 1;
    
    return p;
}

/* =============================================================================
   SECTION 2 : DÉTECTION DE ZONES PAR VALLÉES
   ============================================================================= */

static Segment *find_zones_by_valleys(int *hist, int size, int min_gap, int *num) {
    *num = 0;
    Segment *zones = (Segment *)malloc(size * sizeof(Segment));
    if (!zones) return NULL;
    
    int threshold = 5;
    int in_content = 0, start = 0, gap = 0;
    
    for (int i = 0; i < size; i++) {
        if (hist[i] > threshold) {
            if (!in_content) { in_content = 1; start = i; }
            gap = 0;
        } else if (in_content) {
            gap++;
            if (gap >= min_gap) {
                zones[*num].start = start;
                zones[*num].end = i - gap - 1;
                if (zones[*num].end - zones[*num].start >= 5) (*num)++;
                in_content = 0;
                gap = 0;
            }
        }
    }
    
    if (in_content) {
        zones[*num].start = start;
        zones[*num].end = size - 1;
        if (zones[*num].end - zones[*num].start >= 5) (*num)++;
    }
    
    return zones;
}

/* =============================================================================
   SECTION 3 : SEGMENTATION DE LA GRILLE
   ============================================================================= */

// Seuil pour considérer une ligne/colonne comme une bordure de grille
#define GRID_BORDER_THRESHOLD 0.65

// Trouve les groupes de lignes/colonnes à haute densité (bordures)
static Segment *find_border_groups(int *hist, int size, int dimension, double threshold, int *num_groups) {
    *num_groups = 0;
    
    // Trouver toutes les lignes/colonnes à haute densité
    int *borders = (int *)malloc(size * sizeof(int));
    int num_borders = 0;
    
    for (int i = 0; i < size; i++) {
        double density = (double)hist[i] / dimension;
        if (density >= threshold) {
            borders[num_borders++] = i;
        }
    }
    
    if (num_borders == 0) {
        free(borders);
        return NULL;
    }
    
    // Grouper les bordures consécutives (gap max = 3 pixels)
    Segment *groups = (Segment *)malloc(num_borders * sizeof(Segment));
    int start = borders[0];
    int end = borders[0];
    
    for (int i = 1; i < num_borders; i++) {
        if (borders[i] - end <= 3) {
            end = borders[i];
        } else {
            groups[*num_groups].start = start;
            groups[*num_groups].end = end;
            (*num_groups)++;
            start = borders[i];
            end = borders[i];
        }
    }
    groups[*num_groups].start = start;
    groups[*num_groups].end = end;
    (*num_groups)++;
    
    free(borders);
    return groups;
}

// Méthode 1 : Détection par vallées (grilles sans bordures épaisses)
static GridCells segment_by_valleys(const Image *grid_img, int *h_hist, int *v_hist) {
    GridCells result = {NULL, 0, 0, 0};
    
    printf("    [Méthode vallées] Analyse...\n");
    
    // Trouver les vallées (zones de faible densité)
    int num_h_zones = 0;
    Segment *h_zones = find_zones_by_valleys(h_hist, grid_img->height, 3, &num_h_zones);
    
    int num_v_zones = 0;
    Segment *v_zones = find_zones_by_valleys(v_hist, grid_img->width, 3, &num_v_zones);
    
    if (!h_zones || !v_zones || num_h_zones < 2 || num_v_zones < 2) {
        printf("      → Échec: zones insuffisantes\n");
        if (h_zones) free(h_zones);
        if (v_zones) free(v_zones);
        return result;
    }
    
    printf("      → %d lignes × %d colonnes détectées\n", num_h_zones, num_v_zones);
    
    // Filtrer les zones trop denses (bordures)
    int *valid_rows = (int *)malloc(num_h_zones * sizeof(int));
    int valid_row_count = 0;
    
    for (int i = 0; i < num_h_zones; i++) {
        int black = 0, total = 0;
        for (int y = h_zones[i].start; y <= h_zones[i].end && y < grid_img->height; y++) {
            for (int x = 0; x < grid_img->width; x++) {
                total++;
                if (grid_img->data[y * grid_img->width + x] == 1) black++;
            }
        }
        double density = (double)black / total;
        if (density < 0.55) {
            valid_rows[valid_row_count++] = i;
        }
    }
    
    int *valid_cols = (int *)malloc(num_v_zones * sizeof(int));
    int valid_col_count = 0;
    
    for (int j = 0; j < num_v_zones; j++) {
        int black = 0, total = 0;
        for (int x = v_zones[j].start; x <= v_zones[j].end && x < grid_img->width; x++) {
            for (int y = 0; y < grid_img->height; y++) {
                total++;
                if (grid_img->data[y * grid_img->width + x] == 1) black++;
            }
        }
        double density = (double)black / total;
        if (density < 0.55) {
            valid_cols[valid_col_count++] = j;
        }
    }
    
    printf("      → %d lignes valides × %d colonnes valides\n", valid_row_count, valid_col_count);
    
    if (valid_row_count < 2 || valid_col_count < 2) {
        free(valid_rows);
        free(valid_cols);
        free(h_zones);
        free(v_zones);
        return result;
    }
    
    result.rows = valid_row_count;
    result.cols = valid_col_count;
    result.cells = (Rectangle *)malloc(valid_row_count * valid_col_count * sizeof(Rectangle));
    
    for (int i = 0; i < valid_row_count; i++) {
        for (int j = 0; j < valid_col_count; j++) {
            int ri = valid_rows[i];
            int ci = valid_cols[j];
            
            Rectangle cell = {
                v_zones[ci].start, h_zones[ri].start,
                v_zones[ci].end - v_zones[ci].start + 1,
                h_zones[ri].end - h_zones[ri].start + 1
            };
            result.cells[result.num_cells++] = apply_padding(cell, PADDING_X, PADDING_Y,
                                                              grid_img->width, grid_img->height);
        }
    }
    
    free(valid_rows);
    free(valid_cols);
    free(h_zones);
    free(v_zones);
    
    return result;
}

// Méthode 2 : Détection par bordures (grilles avec cadres épais)
static GridCells segment_by_borders(const Image *grid_img, int *h_hist, int *v_hist) {
    GridCells result = {NULL, 0, 0, 0};
    
    printf("    [Méthode bordures] Analyse...\n");
    
    int num_h_borders = 0;
    Segment *h_borders = find_border_groups(h_hist, grid_img->height, grid_img->width,
                                             GRID_BORDER_THRESHOLD, &num_h_borders);
    
    int num_v_borders = 0;
    Segment *v_borders = find_border_groups(v_hist, grid_img->width, grid_img->height,
                                             GRID_BORDER_THRESHOLD, &num_v_borders);
    
    printf("      → %d bordures H × %d bordures V\n", num_h_borders, num_v_borders);
    
    if (num_h_borders < 2 || num_v_borders < 2) {
        if (h_borders) free(h_borders);
        if (v_borders) free(v_borders);
        return result;
    }
    
    int num_rows = num_h_borders - 1;
    int num_cols = num_v_borders - 1;
    
    result.rows = num_rows;
    result.cols = num_cols;
    result.cells = (Rectangle *)malloc(num_rows * num_cols * sizeof(Rectangle));
    
    for (int i = 0; i < num_rows; i++) {
        for (int j = 0; j < num_cols; j++) {
            int y1 = h_borders[i].end + 1;
            int y2 = h_borders[i+1].start - 1;
            int x1 = v_borders[j].end + 1;
            int x2 = v_borders[j+1].start - 1;
            
            if (y1 < 0) y1 = 0;
            if (x1 < 0) x1 = 0;
            if (y2 >= grid_img->height) y2 = grid_img->height - 1;
            if (x2 >= grid_img->width) x2 = grid_img->width - 1;
            
            if (y2 > y1 && x2 > x1) {
                result.cells[result.num_cells].x = x1;
                result.cells[result.num_cells].y = y1;
                result.cells[result.num_cells].width = x2 - x1 + 1;
                result.cells[result.num_cells].height = y2 - y1 + 1;
                result.num_cells++;
            }
        }
    }
    
    free(h_borders);
    free(v_borders);
    
    return result;
}

GridCells segment_grid_cells(const Image *grid_img) {
    GridCells result = {NULL, 0, 0, 0};
    
    printf("\n[Segmentation Grille] Analyse (%dx%d)...\n", grid_img->width, grid_img->height);
    
    // Calculer les histogrammes une seule fois
    int *h_hist = calculate_histogram(grid_img, 1);
    int *v_hist = calculate_histogram(grid_img, 0);
    
    // STRATÉGIE 1 : Essayer d'abord la détection par bordures
    result = segment_by_borders(grid_img, h_hist, v_hist);
    
    if (result.num_cells > 0) {
        printf("      → Succès avec méthode bordures: %d×%d\n", result.rows, result.cols);
        free(h_hist);
        free(v_hist);
        return result;
    }
    
    // STRATÉGIE 2 : Si échec, utiliser la méthode par vallées
    printf("    [Fallback] Passage à la méthode vallées...\n");
    result = segment_by_valleys(grid_img, h_hist, v_hist);
    
    free(h_hist);
    free(v_hist);
    
    if (result.num_cells > 0) {
        printf("      → Succès avec méthode vallées: %d×%d\n", result.rows, result.cols);
    } else {
        printf("      → Échec des deux méthodes\n");
    }
    
    return result;
}

/* =============================================================================
   SECTION 5 : SEGMENTATION DES MOTS
   ============================================================================= */

Rectangle *segment_word_lines(const Image *list_img, int *num_lines) {
    *num_lines = 0;
    printf("\n[Segmentation Liste] Détection des lignes...\n");
    
    int *h_hist = calculate_histogram(list_img, 1);
    Segment *zones = find_zones_by_valleys(h_hist, list_img->height, 8, num_lines);
    free(h_hist);
    
    if (!zones) {
        printf("  [ERREUR] Aucune ligne détectée\n");
        return NULL;
    }
    printf("  → %d lignes détectées\n", *num_lines);
    
    Rectangle *lines = (Rectangle *)malloc(*num_lines * sizeof(Rectangle));
    for (int i = 0; i < *num_lines; i++) {
        Rectangle base = {0, zones[i].start, list_img->width, zones[i].end - zones[i].start + 1};
        lines[i] = apply_padding(base, PADDING_X, PADDING_Y, list_img->width, list_img->height);
    }
    
    free(zones);
    return lines;
}

/* =============================================================================
   SECTION 6 : COMPOSANTES CONNEXES
   ============================================================================= */

typedef struct {
    int *pixels;
    int count;
    int min_x, max_x, min_y, max_y;
} ConnectedComponent;

static void break_weak_connections(Image *img) {
    int broken = 0;
    
    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            int idx = y * img->width + x;
            
            if (img->data[idx] == 1) {
                int up    = img->data[(y-1) * img->width + x];
                int down  = img->data[(y+1) * img->width + x];
                int left  = img->data[y * img->width + (x-1)];
                int right = img->data[y * img->width + (x+1)];
                int neighbors = up + down + left + right;
                
                if (neighbors <= 2) {
                    int h_bridge = (left && right && !up && !down);
                    int v_bridge = (up && down && !left && !right);
                    int diag = img->data[(y-1)*img->width+(x-1)] + img->data[(y-1)*img->width+(x+1)]
                             + img->data[(y+1)*img->width+(x-1)] + img->data[(y+1)*img->width+(x+1)];
                    int d_bridge = (neighbors == 0 && diag >= 2);
                    
                    if (h_bridge || v_bridge || d_bridge) {
                        img->data[idx] = 0;
                        broken++;
                    }
                }
            }
        }
    }
    
    if (broken > 0) {
        printf("      → %d connexions cassées\n", broken);
    }
}

static void flood_fill(const Image *img, int x, int y, int *visited, int *pixels, int *count) {
    if (x < 0 || x >= img->width || y < 0 || y >= img->height) return;
    
    int idx = y * img->width + x;
    if (visited[idx] || img->data[idx] == 0) return;
    
    visited[idx] = 1;
    pixels[(*count)++] = idx;
    
    flood_fill(img, x-1, y-1, visited, pixels, count);
    flood_fill(img, x,   y-1, visited, pixels, count);
    flood_fill(img, x+1, y-1, visited, pixels, count);
    flood_fill(img, x-1, y,   visited, pixels, count);
    flood_fill(img, x+1, y,   visited, pixels, count);
    flood_fill(img, x-1, y+1, visited, pixels, count);
    flood_fill(img, x,   y+1, visited, pixels, count);
    flood_fill(img, x+1, y+1, visited, pixels, count);
}

static ConnectedComponent *detect_components(const Image *img, int *num_comp) {
    *num_comp = 0;
    
    int *visited = (int *)calloc(img->width * img->height, sizeof(int));
    ConnectedComponent *comp = (ConnectedComponent *)malloc(100 * sizeof(ConnectedComponent));
    
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int idx = y * img->width + x;
            
            if (img->data[idx] == 1 && !visited[idx]) {
                int *pixels = (int *)malloc(img->width * img->height * sizeof(int));
                int count = 0;
                
                flood_fill(img, x, y, visited, pixels, &count);
                
                if (count < 5) {
                    free(pixels);
                    continue;
                }
                
                int min_x = img->width, max_x = 0;
                int min_y = img->height, max_y = 0;
                
                for (int i = 0; i < count; i++) {
                    int px = pixels[i] % img->width;
                    int py = pixels[i] / img->width;
                    if (px < min_x) min_x = px;
                    if (px > max_x) max_x = px;
                    if (py < min_y) min_y = py;
                    if (py > max_y) max_y = py;
                }
                
                comp[*num_comp].pixels = pixels;
                comp[*num_comp].count = count;
                comp[*num_comp].min_x = min_x;
                comp[*num_comp].max_x = max_x;
                comp[*num_comp].min_y = min_y;
                comp[*num_comp].max_y = max_y;
                (*num_comp)++;
            }
        }
    }
    
    free(visited);
    return comp;
}

Rectangle *segment_line_characters(const Image *line_img, int *num_chars) {
    *num_chars = 0;
    
    printf("    [Segmentation Chars] (%dx%d)...\n", line_img->width, line_img->height);
    
    Image copy;
    copy.width = line_img->width;
    copy.height = line_img->height;
    copy.data = (unsigned char *)malloc(line_img->width * line_img->height);
    memcpy(copy.data, line_img->data, line_img->width * line_img->height);
    
    break_weak_connections(&copy);
    
    int num_comp;
    ConnectedComponent *comp = detect_components(&copy, &num_comp);
    free(copy.data);
    
    if (num_comp == 0) {
        printf("      → Aucune composante\n");
        return NULL;
    }
    
    printf("      → %d composantes\n", num_comp);
    
    for (int i = 0; i < num_comp - 1; i++) {
        for (int j = i + 1; j < num_comp; j++) {
            if (comp[j].min_x < comp[i].min_x) {
                ConnectedComponent tmp = comp[i];
                comp[i] = comp[j];
                comp[j] = tmp;
            }
        }
    }
    
    Rectangle *chars = (Rectangle *)malloc(num_comp * sizeof(Rectangle));
    
    for (int i = 0; i < num_comp; i++) {
        Rectangle base = {
            comp[i].min_x, comp[i].min_y,
            comp[i].max_x - comp[i].min_x + 1,
            comp[i].max_y - comp[i].min_y + 1
        };
        chars[(*num_chars)++] = apply_padding(base, PADDING_X, PADDING_Y,
                                               line_img->width, line_img->height);
        free(comp[i].pixels);
    }
    
    free(comp);
    printf("      → %d caractères\n", *num_chars);
    
    return chars;
}

/* =============================================================================
   SECTION 7 : NETTOYAGE INTELLIGENT DES CELLULES DE GRILLE
   ============================================================================= */

#define CELL_BORDER_THRESHOLD 0.80

static int cell_has_borders(const Image *cell) {
    if (!cell || !cell->data || cell->width < 5 || cell->height < 5) return 0;
    
    // Vérifier les premières lignes
    for (int y = 0; y < 2; y++) {
        int black = 0;
        for (int x = 0; x < cell->width; x++) {
            if (cell->data[y * cell->width + x] == 1) black++;
        }
        if ((double)black / cell->width >= CELL_BORDER_THRESHOLD) return 1;
    }
    
    // Vérifier les dernières lignes
    for (int y = cell->height - 2; y < cell->height; y++) {
        int black = 0;
        for (int x = 0; x < cell->width; x++) {
            if (cell->data[y * cell->width + x] == 1) black++;
        }
        if ((double)black / cell->width >= CELL_BORDER_THRESHOLD) return 1;
    }
    
    // Vérifier les premières colonnes
    for (int x = 0; x < 2; x++) {
        int black = 0;
        for (int y = 0; y < cell->height; y++) {
            if (cell->data[y * cell->width + x] == 1) black++;
        }
        if ((double)black / cell->height >= CELL_BORDER_THRESHOLD) return 1;
    }
    
    // Vérifier les dernières colonnes
    for (int x = cell->width - 2; x < cell->width; x++) {
        int black = 0;
        for (int y = 0; y < cell->height; y++) {
            if (cell->data[y * cell->width + x] == 1) black++;
        }
        if ((double)black / cell->height >= CELL_BORDER_THRESHOLD) return 1;
    }
    
    return 0;
}

static Image clean_cell_borders(const Image *cell) {
    if (!cell || !cell->data) return (Image){NULL, 0, 0};
    
    int top = 0, bottom = cell->height - 1;
    int left = 0, right = cell->width - 1;
    
    // Supprimer les lignes du haut
    while (top < cell->height) {
        int black = 0;
        for (int x = 0; x < cell->width; x++) {
            if (cell->data[top * cell->width + x] == 1) black++;
        }
        if ((double)black / cell->width >= CELL_BORDER_THRESHOLD) {
            top++;
        } else {
            break;
        }
    }
    
    // Supprimer les lignes du bas
    while (bottom > top) {
        int black = 0;
        for (int x = 0; x < cell->width; x++) {
            if (cell->data[bottom * cell->width + x] == 1) black++;
        }
        if ((double)black / cell->width >= CELL_BORDER_THRESHOLD) {
            bottom--;
        } else {
            break;
        }
    }
    
    // Supprimer les colonnes de gauche
    while (left < cell->width) {
        int black = 0;
        for (int y = top; y <= bottom; y++) {
            if (cell->data[y * cell->width + left] == 1) black++;
        }
        int height = bottom - top + 1;
        if (height > 0 && (double)black / height >= CELL_BORDER_THRESHOLD) {
            left++;
        } else {
            break;
        }
    }
    
    // Supprimer les colonnes de droite
    while (right > left) {
        int black = 0;
        for (int y = top; y <= bottom; y++) {
            if (cell->data[y * cell->width + right] == 1) black++;
        }
        int height = bottom - top + 1;
        if (height > 0 && (double)black / height >= CELL_BORDER_THRESHOLD) {
            right--;
        } else {
            break;
        }
    }
    
    int new_w = right - left + 1;
    int new_h = bottom - top + 1;
    
    if (new_w <= 0 || new_h <= 0) {
        return copy_image(cell);
    }
    
    Rectangle crop = {left, top, new_w, new_h};
    return create_sub_image(cell, crop);
}

// Recadre l'image sur son contenu (bounding box des pixels noirs)
static Image crop_to_content(const Image *img) {
    if (!img || !img->data) return (Image){NULL, 0, 0};
    
    int min_x = img->width, max_x = 0;
    int min_y = img->height, max_y = 0;
    int has_content = 0;
    
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            if (img->data[y * img->width + x] == 1) {
                has_content = 1;
                if (x < min_x) min_x = x;
                if (x > max_x) max_x = x;
                if (y < min_y) min_y = y;
                if (y > max_y) max_y = y;
            }
        }
    }
    
    if (!has_content) {
        return copy_image(img);
    }
    
    int new_w = max_x - min_x + 1;
    int new_h = max_y - min_y + 1;
    
    Image result;
    result.width = new_w;
    result.height = new_h;
    result.data = (unsigned char *)calloc(new_w * new_h, 1);
    
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            result.data[(y - min_y) * new_w + (x - min_x)] =
                img->data[y * img->width + x];
        }
    }
    
    return result;
}

static Image clean_cell_content(const Image *cell) {
    if (!cell || !cell->data) return (Image){NULL, 0, 0};
    
    // Si pas de bordures, juste recadrer sur le contenu
    if (!cell_has_borders(cell)) {
        return crop_to_content(cell);
    }
    
    // ÉTAPE 1 : Supprimer les bordures continues
    Image cleaned = clean_cell_borders(cell);
    if (!cleaned.data) {
        return crop_to_content(cell);
    }
    
    // ÉTAPE 2 : Recadrer sur le contenu
    Image cropped = crop_to_content(&cleaned);
    free_image(&cleaned);
    
    if (!cropped.data) {
        return crop_to_content(cell);
    }
    
    // ÉTAPE 3 : Si encore des bordures, nettoyer à nouveau
    if (cell_has_borders(&cropped)) {
        Image final = clean_cell_borders(&cropped);
        free_image(&cropped);
        if (final.data) {
            Image result = crop_to_content(&final);
            free_image(&final);
            return result;
        }
        return crop_to_content(cell);
    }
    
    return cropped;
}

/* =============================================================================
   SECTION 8 : NORMALISATION 50x50
   ============================================================================= */

static Image normalize_to_target(const Image *letter) {
    Image result;
    result.width = TARGET_SIZE;
    result.height = TARGET_SIZE;
    result.data = (unsigned char *)calloc(TARGET_SIZE * TARGET_SIZE, 1);
    
    double scale_x = (double)TARGET_SIZE / letter->width;
    double scale_y = (double)TARGET_SIZE / letter->height;
    double scale = (scale_x < scale_y) ? scale_x : scale_y;
    if (scale > 1.0) scale = 1.0;
    
    int new_w = (int)(letter->width * scale);
    int new_h = (int)(letter->height * scale);
    int off_x = (TARGET_SIZE - new_w) / 2;
    int off_y = (TARGET_SIZE - new_h) / 2;
    
    for (int y = 0; y < new_h; y++) {
        for (int x = 0; x < new_w; x++) {
            int src_x = (int)(x / scale);
            int src_y = (int)(y / scale);
            if (src_x < letter->width && src_y < letter->height) {
                result.data[(y + off_y) * TARGET_SIZE + (x + off_x)] =
                    letter->data[src_y * letter->width + src_x];
            }
        }
    }
    
    return result;
}

/* =============================================================================
   SECTION 9 : SAUVEGARDE
   ============================================================================= */

int save_all_grid_cells(const Image *grid_img, GridCells cells, const char *base_dir) {
    printf("\n[Sauvegarde Grille] %d cellules (%dx%d)...\n",
           cells.num_cells, cells.rows, cells.cols);
    
    char cells_dir[512];
    snprintf(cells_dir, sizeof(cells_dir), "%s/2_cells", base_dir);
    create_dir_recursively(cells_dir);
    
    int saved = 0;
    int cleaned_count = 0;
    
    for (int row = 0; row < cells.rows; row++) {
        char line_dir[512];
        snprintf(line_dir, sizeof(line_dir), "%s/line_%02d", cells_dir, row);
        create_dir_recursively(line_dir);
        
        for (int col = 0; col < cells.cols; col++) {
            int idx = row * cells.cols + col;
            Image cell = create_sub_image(grid_img, cells.cells[idx]);
            if (!cell.data) continue;
            
            int needs_cleaning = cell_has_borders(&cell);
            
            Image to_normalize;
            if (needs_cleaning) {
                to_normalize = clean_cell_content(&cell);
                cleaned_count++;
            } else {
                to_normalize = copy_image(&cell);
            }
            
            Image norm = normalize_to_target(&to_normalize);
            
            char path[512];
            snprintf(path, sizeof(path), "%s/cell_%02d.pbm", line_dir, col);
            if (write_pbm(&norm, path) == 0) saved++;
            
            free_image(&norm);
            free_image(&to_normalize);
            free_image(&cell);
        }
    }
    
    printf("  ✓ %d/%d cellules sauvegardées (%dx%d)\n", saved, cells.num_cells, TARGET_SIZE, TARGET_SIZE);
    if (cleaned_count > 0) {
        printf("  ℹ %d cellules avec bordures nettoyées\n", cleaned_count);
    }
    return saved;
}

int save_all_words(const Image *list_img, Rectangle *lines, int num_lines, const char *base_dir) {
    printf("\n[Sauvegarde Mots] %d lignes...\n", num_lines);
    
    char words_dir[512];
    snprintf(words_dir, sizeof(words_dir), "%s/3_words", base_dir);
    create_dir_recursively(words_dir);
    
    int total = 0;
    
    for (int i = 0; i < num_lines; i++) {
        Image line = create_sub_image(list_img, lines[i]);
        if (!line.data) continue;
        
        char word_dir[512];
        snprintf(word_dir, sizeof(word_dir), "%s/word_%02d", words_dir, i);
        create_dir_recursively(word_dir);
        
        int num_chars = 0;
        Rectangle *chars = segment_line_characters(&line, &num_chars);
        printf("  Mot %02d: %d caractères\n", i, num_chars);
        
        if (chars) {
            for (int j = 0; j < num_chars; j++) {
                Image ch = create_sub_image(&line, chars[j]);
                if (ch.data) {
                    Image norm = normalize_to_target(&ch);
                    
                    char path[512];
                    snprintf(path, sizeof(path), "%s/char_%02d.pbm", word_dir, j);
                    write_pbm(&norm, path);
                    
                    free_image(&norm);
                    free_image(&ch);
                    total++;
                }
            }
            free(chars);
        }
        free_image(&line);
    }
    
    printf("  ✓ %d caractères sauvegardés\n", total);
    return total;
}


