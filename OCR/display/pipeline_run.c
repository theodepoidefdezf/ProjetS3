#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "pipeline_run.h"

int run_command(const char *cmd, const char *description){
    printf("\n========================================\n");
    printf("[PIPELINE] %s\n", description);
    printf("  Commande: %s\n", cmd);
    printf("========================================\n");
    int ret = system(cmd);
    if(ret != 0){
        printf("[ERREUR] %s a echoue (code: %d)\n", description, ret);
        return -1;
    }
    printf("[OK] %s termine\n", description);
    return 0;
}

static void ensure_output_folder(void){
    if(mkdir("../output", 0777) != 0 && errno != EEXIST){
        fprintf(stderr, "Impossible de creer ../output\n");
    }
}

int clean_output(void){
    system("rm -rf ../output/*");
    ensure_output_folder();
    return 0;
}

int run_pipeline_full(const char *input_image, double rotation_angle){
    (void)rotation_angle;
    ensure_output_folder();

    char cmd[2048];
    struct stat st;

    snprintf(cmd, sizeof(cmd),
             "make -C ../Preprocessing && ../Preprocessing/preprocessing_test '%s'",
             input_image);
    if(run_command(cmd, "Compilation et execution Preprocessing") != 0) return -1;

    const char *prep_output = "../output/image_auto_rotation.bmp";
    const char *image_to_use = input_image;
    if(stat(prep_output, &st) == 0){
        image_to_use = prep_output;
    }

    snprintf(cmd, sizeof(cmd),
             "make -C ../detection && cd ../detection && ./test_decoupe '%s' auto_run",
             image_to_use);
    if(run_command(cmd, "Compilation et execution Decoupage") != 0) return -1;

    snprintf(cmd, sizeof(cmd),
             "make -C ../ocr && cd ../ocr && ./main 3 ../output/auto_run ../Solver");
    if(run_command(cmd, "Compilation et execution OCR") != 0) return -1;

    snprintf(cmd, sizeof(cmd),
             "make -C ../Solver && ../Solver/solver '../Solver/grid' '../Solver/mots'");
    if(run_command(cmd, "Compilation et execution Solver") != 0) return -1;

    return 0;
}

void run_pipeline(const char *input_image){
    run_pipeline_full(input_image, 0.0);
}
