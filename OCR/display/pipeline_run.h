#ifndef PIPELINE_RUN_H
#define PIPELINE_RUN_H

int clean_output(void);
int run_pipeline_full(const char *input_image, double rotation_angle);
void run_pipeline(const char *input_image);
int run_command(const char *cmd, const char *description);
int run_cooo(const char *cells_path, const char *image_path, const char *coord_path);

#endif
