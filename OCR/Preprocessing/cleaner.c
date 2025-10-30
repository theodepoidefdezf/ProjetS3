#include "../Utils/image.h"
#include "preprocessing.h"

#include "cleaner.h"

SDL_Surface *reduire_bruit(SDL_Surface *surface)
{
    if (!surface)
        return NULL;

    SDL_Surface *output = image_new(surface->h, surface->w);

    for (int y = 1; y < surface->h - 1; y++)
    {
        for (int x = 1; x < surface->w - 1; x++)
        {
            int white_score = 0;
            int black_score = 0;

            for (int dy = -1; dy <= 1; dy++)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    if (dx == 0 && dy == 0)
                        continue;

                    int weight = (dx == 0 || dy == 0) ? 2 : 1;

                    if (is_white_pixel(surface, y + dy, x + dx))
                        white_score += weight;
                    else
                        black_score += weight;
                }
            }

            int center_white = is_white_pixel(surface, y, x);
            if (center_white)
                white_score += 3;
            else
                black_score += 3;

            Uint32 new_pixel;
            if (white_score > black_score)
                new_pixel = SDL_MapRGB(surface->format, 255, 255, 255);
            else
                new_pixel = SDL_MapRGB(surface->format, 0, 0, 0);

            image_set_pixel(output, y, x, new_pixel);
        }
    }

    for (int x = 0; x < surface->w; x++)
    {
        image_set_pixel(output, 0, x, image_get_pixel(surface, 0, x));
        image_set_pixel(output, surface->h - 1, x,
                        image_get_pixel(surface, surface->h - 1, x));
    }
    for (int y = 0; y < surface->h; y++)
    {
        image_set_pixel(output, y, 0, image_get_pixel(surface, y, 0));
        image_set_pixel(output, y, surface->w - 1,
                        image_get_pixel(surface, y, surface->w - 1));
    }

    return output;
}
