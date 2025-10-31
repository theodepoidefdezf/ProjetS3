#ifndef GRID_DETECTION_H
#define GRID_DETECTION_H
#include "../UTILS/structs.h" 
Image isolate_grid_image(Image img);
Image isolate_word_list_image(Image img);
Letters extract_grid_letters(Image grid_image);
#endif 
