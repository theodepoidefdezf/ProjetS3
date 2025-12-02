#include "decoupe.h"

/* =============================================================================
   SECTION 1 : GESTION DES FICHIERS ET MÉMOIRE
   ============================================================================= */

int create_dir_recursively(const char *path) {
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s", path);
    
    size_t len = strlen(tmp);
    if (tmp[len - 1] == '/') tmp[len - 1] = 0;
    
    for (char *p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    
    if (mkdir(tmp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}

Image load_bmp_to_image(const char *filepath) {
    Image img = {NULL, 0, 0};
    
    SDL_Surface *surface = SDL_LoadBMP(filepath);
    if (!surface) {
        fprintf(stderr, "Erreur SDL_LoadBMP: %s\n", SDL_GetError());
        return img;
    }
    
    img.width = surface->w;
    img.height = surface->h;
    img.data = (unsigned char *)malloc(img.width * img.height);
    
    if (!img.data) {
        SDL_FreeSurface(surface);
        return img;
    }
    
    SDL_PixelFormat *fmt = surface->format;
    int bpp = fmt->BytesPerPixel;
    
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
            Uint32 pixel;
            
            switch (bpp) {
                case 1: pixel = *p; break;
                case 2: pixel = *(Uint16 *)p; break;
                case 3:
                    pixel = (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                        ? (p[0] << 16 | p[1] << 8 | p[2])
                        : (p[0] | p[1] << 8 | p[2] << 16);
                    break;
                case 4: pixel = *(Uint32 *)p; break;
                default: pixel = 0;
            }
            
            Uint8 r, g, b;
            SDL_GetRGB(pixel, fmt, &r, &g, &b);
            img.data[y * img.width + x] = (r < 128) ? 1 : 0;
        }
    }
    
    SDL_FreeSurface(surface);
    return img;
}

int write_pbm(const Image *img, const char *filepath) {
    FILE *f = fopen(filepath, "w");
    if (!f) return -1;
    
    fprintf(f, "P1\n%d %d\n", img->width, img->height);
    
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            fprintf(f, "%d ", img->data[y * img->width + x]);
        }
        fprintf(f, "\n");
    }
    
    fclose(f);
    return 0;
}

Image create_sub_image(const Image *source, Rectangle rect) {
    Image sub = {NULL, 0, 0};
    
    // Clamping
    if (rect.x < 0) { rect.width += rect.x; rect.x = 0; }
    if (rect.y < 0) { rect.height += rect.y; rect.y = 0; }
    if (rect.x + rect.width > source->width) rect.width = source->width - rect.x;
    if (rect.y + rect.height > source->height) rect.height = source->height - rect.y;
    if (rect.width <= 0 || rect.height <= 0) return sub;
    
    sub.width = rect.width;
    sub.height = rect.height;
    sub.data = (unsigned char *)malloc(sub.width * sub.height);
    if (!sub.data) return sub;
    
    for (int y = 0; y < sub.height; y++) {
        for (int x = 0; x < sub.width; x++) {
            sub.data[y * sub.width + x] = source->data[(rect.y + y) * source->width + (rect.x + x)];
        }
    }
    
    return sub;
}

Image copy_image(const Image *source) {
    Image copy = {NULL, 0, 0};
    if (!source || !source->data) return copy;
    
    copy.width = source->width;
    copy.height = source->height;
    copy.data = (unsigned char *)malloc(copy.width * copy.height);
    
    if (copy.data) {
        memcpy(copy.data, source->data, copy.width * copy.height);
    }
    return copy;
}

void free_image(Image *img) {
    if (img && img->data) {
        free(img->data);
        img->data = NULL;
        img->width = 0;
        img->height = 0;
    }
}

/* =============================================================================
   SECTION 2 : HISTOGRAMMES ET DÉTECTION DES ZONES
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

static Segment *find_content_zones(int *hist, int size, int gap_threshold, int *num_zones) {
    *num_zones = 0;
    Segment *zones = (Segment *)malloc(size * sizeof(Segment));
    if (!zones) return NULL;
    
    int in_content = 0, start = 0, gap = 0;
    
    for (int i = 0; i < size; i++) {
        if (hist[i] > HIST_NOISE_TOLERANCE) {
            if (!in_content) { in_content = 1; start = i; }
            gap = 0;
        } else if (in_content) {
            gap++;
            if (gap >= gap_threshold) {
                zones[*num_zones].start = start;
                zones[*num_zones].end = i - gap - 1;
                if (zones[*num_zones].end - zones[*num_zones].start >= 2) (*num_zones)++;
                in_content = 0;
                gap = 0;
            }
        }
    }
    
    if (in_content) {
        zones[*num_zones].start = start;
        zones[*num_zones].end = size - 1;
        if (zones[*num_zones].end - zones[*num_zones].start >= 2) (*num_zones)++;
    }
    
    return zones;
}

static Segment *merge_close_zones(Segment *zones, int num_zones, int *num_merged) {
    if (num_zones == 0) { *num_merged = 0; return NULL; }
    
    Segment *merged = (Segment *)malloc(num_zones * sizeof(Segment));
    if (!merged) return NULL;
    
    *num_merged = 0;
    int cur_start = zones[0].start;
    int cur_end = zones[0].end;
    
    for (int i = 1; i < num_zones; i++) {
        if (zones[i].start - cur_end - 1 <= MERGE_GAP_THRESHOLD) {
            cur_end = zones[i].end;
        } else {
            merged[*num_merged].start = cur_start;
            merged[*num_merged].end = cur_end;
            (*num_merged)++;
            cur_start = zones[i].start;
            cur_end = zones[i].end;
        }
    }
    
    merged[*num_merged].start = cur_start;
    merged[*num_merged].end = cur_end;
    (*num_merged)++;
    
    return merged;
}

/* =============================================================================
   SECTION 3 : DÉTECTION DES COMPOSANTES (pour nettoyage)
   ============================================================================= */

static Rectangle *find_segments_from_hist(int *hist, int size, int gap_threshold,
                                          int noise_threshold, int *num_segments) {
    Rectangle *segments = (Rectangle *)malloc(size * sizeof(Rectangle));
    *num_segments = 0;
    
    int in_block = 0, start = 0, gap = 0;
    
    for (int i = 0; i < size; i++) {
        if (hist[i] > noise_threshold) {
            if (!in_block) { in_block = 1; start = i; }
            gap = 0;
        } else if (in_block) {
            gap++;
            if (gap > gap_threshold) {
                segments[*num_segments].x = start;
                segments[*num_segments].width = (i - gap) - start;
                (*num_segments)++;
                in_block = 0;
                gap = 0;
            }
        }
    }
    
    if (in_block) {
        segments[*num_segments].x = start;
        segments[*num_segments].width = (size - 1) - start;
        (*num_segments)++;
    }
    
    return segments;
}

static int check_intersection_content(const Image *img, Rectangle rect, int threshold) {
    int count = 0;
    for (int y = rect.y; y < rect.y + rect.height && y < img->height; y++) {
        for (int x = rect.x; x < rect.x + rect.width && x < img->width; x++) {
            if (y >= 0 && x >= 0 && img->data[y * img->width + x] == 1) count++;
        }
    }
    return count > threshold;
}

Rectangle *find_all_components(const Image *img, int *num_blocks) {
    *num_blocks = 0;
    
    int *v_hist = calculate_histogram(img, 0);
    if (!v_hist) return NULL;
    
    int num_x = 0;
    Rectangle *x_seg = find_segments_from_hist(v_hist, img->width, 20, 2, &num_x);
    free(v_hist);
    
    int *h_hist = calculate_histogram(img, 1);
    int num_y = 0;
    Rectangle *y_seg = find_segments_from_hist(h_hist, img->height, 10, 2, &num_y);
    free(h_hist);
    
    Rectangle *blocks = (Rectangle *)malloc(num_x * num_y * sizeof(Rectangle));
    
    for (int i = 0; i < num_x; i++) {
        for (int j = 0; j < num_y; j++) {
            Rectangle r = {x_seg[i].x, y_seg[j].x, x_seg[i].width, y_seg[j].width};
            if (check_intersection_content(img, r, 50)) {
                blocks[(*num_blocks)++] = r;
            }
        }
    }
    
    free(x_seg);
    free(y_seg);
    return blocks;
}

/* =============================================================================
   SECTION 4 : NETTOYAGE DE L'IMAGE
   ============================================================================= */

static double calculate_block_density(const Image *img, Rectangle rect) {
    long pixels = 0;
    long area = (long)rect.width * rect.height;
    if (area == 0) return 0;
    
    for (int y = rect.y; y < rect.y + rect.height && y < img->height; y++) {
        for (int x = rect.x; x < rect.x + rect.width && x < img->width; x++) {
            if (y >= 0 && x >= 0 && img->data[y * img->width + x] == 1) pixels++;
        }
    }
    return (double)pixels / area;
}

static void copy_block_to_image(const Image *src, Image *dst, Rectangle rect) {
    for (int y = rect.y; y < rect.y + rect.height && y < dst->height; y++) {
        for (int x = rect.x; x < rect.x + rect.width && x < dst->width; x++) {
            if (y >= 0 && x >= 0 && src->data[y * src->width + x] == 1) {
                dst->data[y * dst->width + x] = 1;
            }
        }
    }
}

Image clean_image(const Image *original, Rectangle *blocks, int num_blocks) {
    Image clean = {NULL, original->width, original->height};
    clean.data = (unsigned char *)calloc(clean.width * clean.height, 1);
    
    if (!clean.data) return clean;
    
    printf("[Nettoyage] Analyse de %d composantes...\n", num_blocks);
    
    for (int i = 0; i < num_blocks; i++) {
        Rectangle rect = blocks[i];
        long surface = (long)rect.width * rect.height;
        double density = calculate_block_density(original, rect);
        
        // Filtrer dessins, bannières, lignes faibles, bruit
        if (surface > DRAWING_MIN_SURFACE && density > DRAWING_MIN_DENSITY) continue;
        if (rect.width > original->width * 0.7 && rect.height < original->height * 0.15 &&
            rect.y < original->height / 2 && density > 0.05) continue;
        if (rect.width > original->width * 0.5 && rect.height < 40 && density < 0.05) continue;
        if (surface < 100) continue;
        
        copy_block_to_image(original, &clean, rect);
    }
    
    printf("[Nettoyage] Terminé.\n");
    return clean;
}

/* =============================================================================
   SECTION 5 : SUPPRESSION DES BORDURES DE GRILLE
   ============================================================================= */

// Vérifie si une ligne contient un trait continu (bordure)
static int is_border_line(const Image *img, int y) {
    int consecutive = 0, max_consecutive = 0;
    
    for (int x = 0; x < img->width; x++) {
        if (img->data[y * img->width + x] == 1) {
            consecutive++;
            if (consecutive > max_consecutive) max_consecutive = consecutive;
        } else {
            consecutive = 0;
        }
    }
    
    return (max_consecutive >= img->width * BORDER_CONTINUITY_THRESHOLD);
}

// Vérifie si une colonne contient un trait continu (bordure)
static int is_border_col(const Image *img, int x) {
    int consecutive = 0, max_consecutive = 0;
    
    for (int y = 0; y < img->height; y++) {
        if (img->data[y * img->width + x] == 1) {
            consecutive++;
            if (consecutive > max_consecutive) max_consecutive = consecutive;
        } else {
            consecutive = 0;
        }
    }
    
    return (max_consecutive >= img->height * BORDER_CONTINUITY_THRESHOLD);
}

Image remove_grid_frame(const Image *grid_img) {
    if (!grid_img || !grid_img->data) {
        return (Image){NULL, 0, 0};
    }
    
    int top = 0, bottom = grid_img->height - 1;
    int left = 0, right = grid_img->width - 1;
    
    // Épaisseur max autorisée (5% de chaque dimension)
    int max_v_border = grid_img->height * 0.05;
    int max_h_border = grid_img->width * 0.05;
    if (max_v_border < 2) max_v_border = 2;
    if (max_h_border < 2) max_h_border = 2;
    
    // Scanner depuis le haut
    int removed_top = 0;
    while (top < grid_img->height && removed_top < max_v_border && is_border_line(grid_img, top)) {
        top++;
        removed_top++;
    }
    
    // Scanner depuis le bas
    int removed_bottom = 0;
    while (bottom > top && removed_bottom < max_v_border && is_border_line(grid_img, bottom)) {
        bottom--;
        removed_bottom++;
    }
    
    // Scanner depuis la gauche
    int removed_left = 0;
    while (left < grid_img->width && removed_left < max_h_border && is_border_col(grid_img, left)) {
        left++;
        removed_left++;
    }
    
    // Scanner depuis la droite
    int removed_right = 0;
    while (right > left && removed_right < max_h_border && is_border_col(grid_img, right)) {
        right--;
        removed_right++;
    }
    
    // Vérifier si des bordures ont été détectées
    int new_width = right - left + 1;
    int new_height = bottom - top + 1;
    
    if (new_width <= 0 || new_height <= 0) {
        printf("[Bordures] ERREUR: Dimensions invalides après suppression\n");
        return copy_image(grid_img);
    }
    
    if (removed_top > 0 || removed_bottom > 0 || removed_left > 0 || removed_right > 0) {
        printf("[Bordures] Supprimées: haut=%dpx, bas=%dpx, gauche=%dpx, droite=%dpx\n",
               removed_top, removed_bottom, removed_left, removed_right);
        printf("[Bordures] Taille: %dx%d → %dx%d\n",
               grid_img->width, grid_img->height, new_width, new_height);
    } else {
        printf("[Bordures] Aucune bordure détectée\n");
        return copy_image(grid_img);
    }
    
    // Créer l'image croppée
    Rectangle crop = {left, top, new_width, new_height};
    return create_sub_image(grid_img, crop);
}

/* =============================================================================
   SECTION 6 : DÉTECTION GRILLE/LISTE
   ============================================================================= */

Rectangle *detect_grid_and_list(const Image *img, int *num_blocks) {
    *num_blocks = 0;
    
    int *v_hist = calculate_histogram(img, 0);
    int *h_hist = calculate_histogram(img, 1);
    
    if (!v_hist || !h_hist) {
        free(v_hist);
        free(h_hist);
        return NULL;
    }
    
    int num_x = 0, num_y = 0;
    Segment *x_zones = find_content_zones(v_hist, img->width, GAP_BLOCKS, &num_x);
    Segment *y_zones = find_content_zones(h_hist, img->height, GAP_BLOCKS, &num_y);
    
    free(v_hist);
    free(h_hist);
    
    if (!x_zones || !y_zones || num_x == 0 || num_y == 0) {
        printf("[ERREUR] Aucune zone détectée (X:%d, Y:%d)\n", num_x, num_y);
        free(x_zones);
        free(y_zones);
        return NULL;
    }
    
    printf("[Détection] Zones brutes: X:%d, Y:%d\n", num_x, num_y);
    
    // Fusion si trop de zones
    Segment *x_final = x_zones;
    Segment *y_final = y_zones;
    int num_x_final = num_x;
    int num_y_final = num_y;
    
    if (num_x > 6) {
        x_final = merge_close_zones(x_zones, num_x, &num_x_final);
        free(x_zones);
    }
    if (num_y > 4) {
        y_final = merge_close_zones(y_zones, num_y, &num_y_final);
        free(y_zones);
    }
    
    // Créer les blocs
    Rectangle *blocks = (Rectangle *)malloc(num_x_final * num_y_final * sizeof(Rectangle));
    if (!blocks) {
        free(x_final);
        free(y_final);
        return NULL;
    }
    
    for (int i = 0; i < num_x_final; i++) {
        for (int j = 0; j < num_y_final; j++) {
            int w = x_final[i].end - x_final[i].start + 1;
            int h = y_final[j].end - y_final[j].start + 1;
            
            if (w >= MIN_BLOCK_WIDTH && h >= MIN_BLOCK_HEIGHT) {
                blocks[*num_blocks].x = x_final[i].start;
                blocks[*num_blocks].y = y_final[j].start;
                blocks[*num_blocks].width = w;
                blocks[*num_blocks].height = h;
                (*num_blocks)++;
            }
        }
    }
    
    free(x_final);
    free(y_final);
    
    printf("[Détection] %d blocs détectés\n", *num_blocks);
    return blocks;
}

int is_likely_grid(Rectangle rect) {
    double ratio = (double)rect.height / rect.width;
    return (ratio >= 0.6 && ratio <= 1.6);
}
