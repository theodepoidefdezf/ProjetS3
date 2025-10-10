#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H
#include "../utils/structs.h"

Image load_image(const char *filename);
void save_image(Image img, const char *filename);

#endif
