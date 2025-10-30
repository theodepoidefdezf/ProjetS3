#ifndef CLEANER_H
#define CLEANER_H

#include <SDL2/SDL.h>

SDL_Surface *image_noise_reduction(SDL_Surface *surface);

void image_set_pixel(SDL_Surface *image, int h, int w, Uint32 pixel);

#endif 
