#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "pipeline_run.h"
#include "display.h"

int run_command(const char *cmd, const char *description){
    (void)description;
    int ret = system(cmd);
    if(ret != 0) return -1;
    return 0;
}

static void ensure_output_folder(void){
    if(mkdir("../output", 0777) != 0 && errno != EEXIST) return;
}

int clean_output(void){
    system("rm -rf ../output/*");
    ensure_output_folder();
    return 0;
}

int run_pipeline_full(const char *input_image, double rotation_angle, AppWidgets *aw){
    (void)rotation_angle;
    ensure_output_folder();

    char cmd[2048];
    struct stat st;

    snprintf(cmd, sizeof(cmd),
             "make -C ../Preprocessing && ../Preprocessing/preprocessing_test '%s'",
             input_image);
    if(run_command(cmd, "Preprocessing") != 0) return -1;

    const char *prep_output = "../output/image_noise_reduc_auto.bmp";
    const char *image_to_use = input_image;
    if(stat(prep_output, &st) == 0) image_to_use = prep_output;

    snprintf(cmd, sizeof(cmd),
             "make -C ../detection && cd ../detection && ./test_decoupe '%s' test",
             image_to_use);
    if(run_command(cmd, "Decoupage") != 0) return -1;

    snprintf(cmd, sizeof(cmd),
             "make -C ../ocr && cd ../ocr && ./main 3 ../output/test ../Solver");
    if(run_command(cmd, "OCR") != 0) return -1;

    snprintf(cmd, sizeof(cmd),
             "cd ../Solver && make && ./solver grid mots");
    if(run_command(cmd, "Solver") != 0) return -1;

    snprintf(cmd, sizeof(cmd),
             "make -C ../Cooo && ../Cooo/cooo ../output/test/2_cells/ ../output/test/1_blocks/block_0_grille_raw.pbm ../Solver/coordonnees");
    if(run_command(cmd, "Cooo") != 0) return -1;

    const char *cooo_output = "../output/output_with_coords.pbm";

    if(aw->orig_pixbuf) g_object_unref(aw->orig_pixbuf);
    GError *err = NULL;
    aw->orig_pixbuf = gdk_pixbuf_new_from_file(cooo_output, &err);
    if(!aw->orig_pixbuf){
        if(err) g_error_free(err);
        return -1;
    }

    aw->rotation_angle = 0.0;
    strncpy(aw->current_image_path, cooo_output, sizeof(aw->current_image_path) - 1);
    aw->current_image_path[sizeof(aw->current_image_path) - 1] = '\0';
    update_display_pixbuf(aw);

    return 0;
}

void run_pipeline(const char *input_image, AppWidgets *aw){
    run_pipeline_full(input_image, 0.0, aw);
}
