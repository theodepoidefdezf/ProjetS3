#include "decoupe.h"
#include <dirent.h>
#include <unistd.h>


static void clean_directory(const char *dir_path) {
    DIR *dir = opendir(dir_path);
    if (!dir) return;
    
    struct dirent *entry;
    char path[1024];
    
    while ((entry = readdir(dir)) != NULL) {
        
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);
        
        struct stat st;
        if (stat(path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
               
                clean_directory(path);
                rmdir(path);
            } else {
                
                unlink(path);
            }
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <image.bmp> [nom_test]\n", argv[0]);
        return 1;
    }
    
    const char *test_name = (argc >= 3) ? argv[2] : "test";
    
    SDL_Init(SDL_INIT_VIDEO);
    
    
    char base_dir[512], dir_debug[512], dir_blocks[512], dir_cells[512], dir_words[512];
    snprintf(base_dir, sizeof(base_dir), "../output/%s", test_name);
    snprintf(dir_debug, sizeof(dir_debug), "%s/0_debug", base_dir);
    snprintf(dir_blocks, sizeof(dir_blocks), "%s/1_blocks", base_dir);
    snprintf(dir_cells, sizeof(dir_cells), "%s/2_cells", base_dir);
    snprintf(dir_words, sizeof(dir_words), "%s/3_words", base_dir);
    
    
    clean_directory(dir_debug);
    clean_directory(dir_blocks);
    clean_directory(dir_cells);
    clean_directory(dir_words);
    
    create_dir_recursively(dir_debug);
    create_dir_recursively(dir_blocks);
    create_dir_recursively(dir_cells);
    create_dir_recursively(dir_words);
    
    printf("========================================\n");
    printf("  DÉCOUPE COMPLÈTE : %s\n", test_name);
    printf("========================================\n\n");
    
    char path[512];
    
    
    printf("[ÉTAPE 1] Chargement...\n");
    printf("  Fichier: %s\n", argv[1]);
    
    Image img = load_bmp_to_image(argv[1]);
    if (!img.data) {
        fprintf(stderr, "ERREUR: Chargement impossible\n");
        return 1;
    }
    printf("  ✓ %dx%d pixels\n\n", img.width, img.height);
    
    snprintf(path, sizeof(path), "%s/original.pbm", dir_debug);
    write_pbm(&img, path);
    
    
    printf("[ÉTAPE 2] Nettoyage...\n");
    
    int num_comp = 0;
    Rectangle *comp = find_all_components(&img, &num_comp);
    printf("  → %d composantes\n", num_comp);
    
    Image clean = clean_image(&img, comp, num_comp);
    free(comp);
    free_image(&img);
    
    snprintf(path, sizeof(path), "%s/cleaned.pbm", dir_debug);
    write_pbm(&clean, path);
    printf("  ✓ Image nettoyée\n\n");
   
    printf("[ÉTAPE 3] Détection grille/liste...\n");
    
    int num_blocs = 0;
    Rectangle *blocs = detect_grid_and_list(&clean, &num_blocs);
    
    if (!blocs || num_blocs == 0) {
        fprintf(stderr, "ERREUR: Aucun bloc détecté\n");
        free_image(&clean);
        return 1;
    }
    
    int grid_idx = -1;
    long max_surf = 0;
    
    printf("\n[Identification]\n");
    for (int i = 0; i < num_blocs; i++) {
        long s = (long)blocs[i].width * blocs[i].height;
        printf("  Bloc %d: %dx%d → ", i, blocs[i].width, blocs[i].height);
        
        if (is_likely_grid(blocs[i]) && s > max_surf) {
            printf("GRILLE\n");
            max_surf = s;
            grid_idx = i;
        } else {
            printf("LISTE\n");
        }
    }
    
    if (grid_idx == -1) grid_idx = 0;
    printf("  ✓ Grille = Bloc %d\n\n", grid_idx);
    
    
    printf("[ÉTAPE 4] Extraction des blocs...\n");
    
    Image grid_img = {0};
    Image list_img = {0};
    
    for (int i = 0; i < num_blocs; i++) {
        Image sub = create_sub_image(&clean, blocs[i]);
        
        if (i == grid_idx) {
            snprintf(path, sizeof(path), "%s/block_%d_grille_raw.pbm", dir_blocks, i);
            write_pbm(&sub, path);
            printf("  ✓ GRILLE (brute): %s\n", path);
            grid_img = sub;
        } else {
            snprintf(path, sizeof(path), "%s/block_%d_liste.pbm", dir_blocks, i);
            write_pbm(&sub, path);
            printf("  ✓ LISTE: %s\n", path);
            if (!list_img.data) {
                list_img = sub;
            } else {
                free_image(&sub);
            }
        }
    }
    free(blocs);
    free_image(&clean);
    printf("\n");
    
   
    if (grid_img.data) {
        printf("[ÉTAPE 4bis] Suppression des bordures de la grille...\n");
        
        Image grid_clean = remove_grid_frame(&grid_img);
        
        if (grid_clean.data) {
           
            snprintf(path, sizeof(path), "%s/block_%d_grille_clean.pbm", dir_blocks, grid_idx);
            write_pbm(&grid_clean, path);
            printf("  ✓ GRILLE (nettoyée): %s\n", path);
            
            
            free_image(&grid_img);
            grid_img = grid_clean;
        } else {
            printf("  ℹ Pas de modification nécessaire\n");
        }
        printf("\n");
    }
    
    
    if (grid_img.data) {
        printf("[ÉTAPE 5] Segmentation de la grille...\n");
        
        GridCells cells = segment_grid_cells(&grid_img);
        
        if (cells.num_cells > 0) {
            printf("  ✓ Grille: %d lignes × %d colonnes = %d cellules\n",
                   cells.rows, cells.cols, cells.num_cells);
            
            save_all_grid_cells(&grid_img, cells, base_dir);
            free(cells.cells);
        } else {
            printf("  [ERREUR] Aucune cellule détectée\n");
        }
        
        free_image(&grid_img);
    }
   
    if (list_img.data) {
        printf("\n[ÉTAPE 6] Segmentation de la liste...\n");
        
        int num_lines = 0;
        Rectangle *lines = segment_word_lines(&list_img, &num_lines);
        
        if (lines && num_lines > 0) {
            save_all_words(&list_img, lines, num_lines, base_dir);
            free(lines);
        } else {
            printf("  [ERREUR] Aucune ligne détectée\n");
        }
        
        free_image(&list_img);
    }
    
    
    SDL_Quit();
    
    printf("\n========================================\n");
    printf("✓ DÉCOUPE TERMINÉE\n");
    printf("========================================\n");
    printf("Résultats dans: %s/\n", base_dir);
    printf("  0_debug/   → Images de debug\n");
    printf("  1_blocks/  → Grille (brute + nettoyée) et liste\n");
    printf("  2_cells/   → Cellules de la grille (%dx%d)\n", TARGET_SIZE, TARGET_SIZE);
    printf("  3_words/   → Mots et caractères (%dx%d)\n", TARGET_SIZE, TARGET_SIZE);
    
    return 0;
}
