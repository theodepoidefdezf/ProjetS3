#ifndef PIPELINE_RUN_H
#define PIPELINE_RUN_H

int clean_output(void);
int run_pipeline_full(const char *input_image, double rotation_angle);
void run_pipeline(const char *input_image);

#endif
