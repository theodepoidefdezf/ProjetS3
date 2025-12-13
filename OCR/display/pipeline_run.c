#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "pipeline_run.h"

static int run_command(const char *cmd, const char *description){
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

static int make_executable(const char *path){
    if(chmod(path, 0755) != 0){
        fprintf(stderr, "Impossible de rendre %s executable: %s\n", path, strerror(errno));
        return -1;
    }
    return 0;
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

    snprintf(cmd, sizeof(cmd), "make -C ../Preprocessing");
    if(run_command(cmd, "Compilation Preprocessing") != 0) return -1;
    make_executable("../Preprocessing/preprocessing_test");

    snprintf(cmd, sizeof(cmd), "../Preprocessing/preprocessing_test '%s'", input_image);
    if(run_command(cmd, "Execution Preprocessing") != 0) return -1;

    const char *prep_output = "../output/image_auto_rotation.bmp";
    const char *image_to_use = input_image;
    if(stat(prep_output, &st) == 0){
        image_to_use = prep_output;
    }

    snprintf(cmd, sizeof(cmd), "make -C ../detection");
    if(run_command(cmd, "Compilation Decoupage") != 0) return -1;
    make_executable("../detection/test_decoupe");

    snprintf(cmd, sizeof(cmd), "../detection/test_decoupe '%s' auto_run", image_to_use);
    if(run_command(cmd, "Execution Decoupage") != 0) return -1;

    snprintf(cmd, sizeof(cmd), "make -C ../ocr");
    if(run_command(cmd, "Compilation OCR") != 0) return -1;
    make_executable("../ocr/main");

    snprintf(cmd, sizeof(cmd), "../ocr/main 3 ../output/auto_run ../output/auto_run");
    if(run_command(cmd, "Execution OCR") != 0) return -1;

    snprintf(cmd, sizeof(cmd), "make -C ../Solver");
    if(run_command(cmd, "Compilation Solver") != 0) return -1;
    make_executable("../Solver/solver");

    snprintf(cmd, sizeof(cmd), "../Solver/solver '../output/auto_run/grid.txt' '../output/auto_run/word.txt'");
    if(run_command(cmd, "Execution Solver") != 0) return -1;

    return 0;
}

void run_pipeline(const char *input_image){
    run_pipeline_full(input_image, 0.0);
}
