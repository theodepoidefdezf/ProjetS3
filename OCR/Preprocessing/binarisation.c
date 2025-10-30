#include <SDL2/SDL.h>
#include <stdlib.h>
#include <math.h>
#include "../Utils/image.h"
#include "binarisation.h"

static Uint8 get_grayscale(Uint8 r, Uint8 g, Uint8 b)
{
    return (Uint8)(0.2126 * r + 0.7152 * g + 0.0722 * b);
}

static void build_histogram(SDL_Surface *img, unsigned long hist[256])
{
    for (int i = 0; i < 256; i++)
        hist[i] = 0;

    for (int y = 0; y < img->h; y++)
    {
        for (int x = 0; x < img->w; x++)
        {
            Uint8 r, g, b;
            Uint32 pixel = image_get_pixel(img, y, x);
            SDL_GetRGB(pixel, img->format, &r, &g, &b);

            Uint8 gray = get_grayscale(r, g, b);
            hist[gray]++;
        }
    }
}

static int otsu_threshold(SDL_Surface *img)
{
    unsigned long hist[256];
    build_histogram(img, hist);

    unsigned long total_pixels = img->w * img->h;
    double sum_total = 0;
    for (int i = 0; i < 256; i++)
        sum_total += i * hist[i];

    unsigned long weight_bg = 0;  
    double sum_bg = 0;           
    double max_variance = 0.0;
    int threshold = 127;

    for (int t = 0; t < 256; t++)
    {
        weight_bg += hist[t];
        if (weight_bg == 0)
            continue;

        unsigned long weight_fg = total_pixels - weight_bg;
        if (weight_fg == 0)
            break;

        sum_bg += (double)t * hist[t];

        double mean_bg = sum_bg / weight_bg;
        double mean_fg = (sum_total - sum_bg) / weight_fg;

        double diff = mean_bg - mean_fg;
        double variance_between = weight_bg * weight_fg * diff * diff;

        if (variance_between > max_variance)
        {
            max_variance = variance_between;
            threshold = t;
        }
    }

    return threshold;
}

void conversion_bina(SDL_Surface *surface)
{
    int threshold = otsu_threshold(surface);

    for (int y = 0; y < surface->h; y++)
    {
        for (int x = 0; x < surface->w; x++)
        {
            Uint8 r, g, b;
            Uint32 pixel = image_get_pixel(surface, y, x);
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);

            Uint8 gray = get_grayscale(r, g, b);
            Uint32 new_pixel;

            if (gray > threshold + 5)
                new_pixel = SDL_MapRGB(surface->format, 255, 255, 255);
            else
                new_pixel = SDL_MapRGB(surface->format, 0, 0, 0);

            image_set_pixel(surface, y, x, new_pixel);
        }
    }
}
