#ifndef SOLVER_H
#define SOLVER_H
#include "../utils/structs.h"

Result solve_grid(Grid grid, WordList words);
int find_word(Grid grid, const char *word, int *start_x, int *start_y, int *end_x, int *end_y);

#endif
