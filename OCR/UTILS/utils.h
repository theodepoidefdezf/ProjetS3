#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include "../utils/structs.h"

void log_message(const char *msg);
void check_allocation(void *ptr, const char *context);
void free_image(Image img);
void free_grid(Grid grid);

#endif
