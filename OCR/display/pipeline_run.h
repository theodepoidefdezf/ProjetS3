#ifndef PIPELINE_RUN_H
#define PIPELINE_RUN_H

#include "display.h"

int clean_output(void);
int run_pipeline_full(const char *input_image, double rotation_angle, AppWidgets *aw);
void run_pipeline(const char *input_image, AppWidgets *aw);
int run_command(const char *cmd, const char *description);

#endif
