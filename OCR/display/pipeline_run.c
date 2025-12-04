#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pipeline_run.h"

void run_pipeline(const char *input_image)
{
    char cmd[512];
    system("make -C ../Preprocessing");
    snprintf(cmd, sizeof(cmd), "../Preprocessing/preprocessing_test %s", input_image);
    system(cmd);
    const char *prep_output = "../output/image_rotation_manuelle.bmp";
    system("make -C ../detection");
    snprintf(cmd, sizeof(cmd), "../detection/test_decoupe %s auto_run", prep_output);
    system(cmd);
    const char *grid_file  = "../output/auto_run/grid.txt";
    const char *words_file = "../output/auto_run/words.txt";
    system("make -C ../Solver");
    snprintf(cmd, sizeof(cmd), "../Solver/solver %s %s", grid_file, words_file);
    system(cmd);
}
