#ifndef UTILS_IMAGE_H
#define UTILS_IMAGE_H

#include <stdbool.h>
#include <SDL2/SDL.h>

// Chargement et création d'image
SDL_Surface *image_load(const char *path);
SDL_Surface *image_new(int height, int width);

// Accès aux pixels
Uint32 image_get_pixel(SDL_Surface *image, int h, int w);
void image_set_pixel(SDL_Surface *image, int h, int w, Uint32 pixel);

bool is_white_pixel(SDL_Surface *image, int h, int w);
bool is_blank_line(SDL_Surface *text, int height);
bool is_blank_column(SDL_Surface *line, int width);
void draw_line(SDL_Surface *image, Uint32 pixel, int height, int w_start, int w_end);
void draw_column(SDL_Surface *image, Uint32 pixel, int width, int h_start, int h_end);

#endif
