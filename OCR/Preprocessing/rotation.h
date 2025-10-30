#ifndef ROTATION_H
#define ROTATION_H

#include <SDL2/SDL.h>

SDL_Surface *image_deskew(SDL_Surface *img);

void manual_rotation(const char *image_path);

#endif 
