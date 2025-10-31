#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include <SDL2/SDL.h>

void preprocessing(const char *image_path);

void conversion(SDL_Surface *image);
void conversion_bina(SDL_Surface *image);
SDL_Surface *correction_inclinaison(SDL_Surface *image);
SDL_Surface *image_noise_reduction(SDL_Surface *image);

void manual_rotation(const char *image_path);

#endif
