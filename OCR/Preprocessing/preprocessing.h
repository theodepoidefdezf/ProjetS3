#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include "../utils/structs.h"

Image to_grayscale(Image img);
Image binarize(Image img);
Image preprocess(Image img);

#endif
