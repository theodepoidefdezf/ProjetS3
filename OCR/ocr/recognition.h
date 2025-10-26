#ifndef RECOGNITION_H
#define RECOGNITION_H

#include "../utils/structs.h"

Tensor *image_to_tensor_input(Image img);
char recognize_letter(NeuralNetwork *net, Image input_image);
char decode_argmax(Tensor *probs);

#endif // RECOGNITION_H