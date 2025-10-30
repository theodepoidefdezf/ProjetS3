#include <SDL2/SDL.h>
#include "../Utils/image.h"
#include "color_modif.h"

void conversion(SDL_Surface *surface)
{
    if (!surface)
        return;

    for (int y = 0; y < surface->h; y++)
    {
        for (int x = 0; x < surface->w; x++)
        {
            Uint8 r, g, b;
            Uint32 pixel = image_get_pixel(surface, y, x);
            SDL_GetRGB(pixel, surface->format, &r, &g, &b);

            float brightness = sqrtf(
                (r * r * 0.241f) +
                (g * g * 0.691f) +
                (b * b * 0.068f)
            );

            float avg = (r + g + b) / 3.0f;
            float saturation = fabsf(r - avg) + fabsf(g - avg) + fabsf(b - avg);
            if (saturation > 50.0f)
                brightness = (brightness * 0.9f) + (avg * 0.1f);

            Uint8 gray = (Uint8)(brightness > 255 ? 255 : brightness);

            Uint32 gray_pixel = SDL_MapRGB(surface->format, gray, gray, gray);
            image_set_pixel(surface, y, x, gray_pixel);
        }
    }
}
