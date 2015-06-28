/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "pixel.h"

/*
 * Lock a SDL_Surface for safe color access
 */
static void lock (SDL_Surface *surface)
{
    if (SDL_MUSTLOCK(surface)) {
        SDL_LockSurface(surface);
    }
}

/*
 * Unlock a SDL_Surface, after lock()
 */
static void unlock (SDL_Surface *surface)
{
    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }
}

/*
 * This function sets the specified color on a SDL_Surface. Sanity checks are
 * performed on the co-ordinates and the SDL_Surface is locked for you.
 * Safe, but slow. For more speed, try the lower level access function.
 */
void putPixel (SDL_Surface * surface, uint32_t x, uint32_t y, color col)
{
    uint8_t   bpp = surface->format->BytesPerPixel;
    uint16_t pitch = surface->pitch/bpp;
    uint32_t rgb;

    if (x >= (uint32_t)surface->w) {
//        ERR("out of range x %u, max %u", x, surface->w);
        return;
    }

    if (y >= (uint32_t)surface->h) {
//        ERR("out of range y %u, max %u", y, surface->h);
        return;
    }

    lock(surface);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rgb = ((col.r << 24) | (col.g << 16) | (col.b << 8) | col.a);
#else
    rgb = (col.r | (col.g << 8) | (col.b << 16) | (col.a << 24));
#endif

    switch (pitch) {
        case 16:
            putPixel_16x32bpp(surface, x, y, rgb);
            break;
        case 24:
            putPixel_24x32bpp(surface, x, y, rgb);
            break;
        case 32:
            putPixel_32x32bpp(surface, x, y, rgb);
            break;
        case 48:
            putPixel_48x32bpp(surface, x, y, rgb);
            break;
        case 64:
            putPixel_64x32bpp(surface, x, y, rgb);
            break;
        case 128:
            putPixel_128x32bpp(surface, x, y, rgb);
            break;
        case 256:
            putPixel_256x32bpp(surface, x, y, rgb);
            break;
        case 320:
            putPixel_320x32bpp(surface, x, y, rgb);
            break;
        case 640:
            putPixel_640x32bpp(surface, x, y, rgb);
            break;
        case 800:
            putPixel_800x32bpp(surface, x, y, rgb);
            break;
        case 1024:
            putPixel_1024x32bpp(surface, x, y, rgb);
            break;
        default:
            putPixel_32bpp(surface, x, y, rgb);
            break;
    }

    unlock(surface);
}

/*
 * This function sets the specified color on a SDL_Surface. Sanity checks are
 * performed on the co-ordinates and the SDL_Surface is locked for you.
 * Safe, but slow. For more speed, try the lower level access function.
 */
color getPixel (SDL_Surface * surface, uint32_t x, uint32_t y)
{
    uint8_t   bpp = surface->format->BytesPerPixel;
    uint16_t pitch = surface->pitch/bpp;
    uint32_t rgb;
    color     col;

    if ((x >= (uint32_t)surface->w) || (y >= (uint32_t)surface->h)) {
        ERR("getPixel out of range, pix %d,%d in size %d,%d",
            x, y, surface->w, surface->h);
    }

    lock(surface);

    switch (pitch) {
        case 16:
            getPixel_16x32bpp(surface, x, y, &rgb);
            break;
        case 24:
            getPixel_24x32bpp(surface, x, y, &rgb);
            break;
        case 32:
            getPixel_32x32bpp(surface, x, y, &rgb);
            break;
        case 48:
            getPixel_48x32bpp(surface, x, y, &rgb);
            break;
        case 64:
            getPixel_64x32bpp(surface, x, y, &rgb);
            break;
        case 128:
            getPixel_128x32bpp(surface, x, y, &rgb);
            break;
        case 256:
            getPixel_256x32bpp(surface, x, y, &rgb);
            break;
        case 320:
            getPixel_320x32bpp(surface, x, y, &rgb);
            break;
        case 640:
            getPixel_640x32bpp(surface, x, y, &rgb);
            break;
        case 800:
            getPixel_800x32bpp(surface, x, y, &rgb);
            break;
        case 1024:
            getPixel_1024x32bpp(surface, x, y, &rgb);
            break;
        default:
            getPixel_32bpp(surface, x, y, &rgb);
            break;
    }

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    col.r = (rgb & 0xff000000) >> 24;
    col.g = (rgb & 0x00ff0000) >> 16;
    col.b = (rgb & 0x0000ff00) >> 8;
    col.a = (rgb & 0x000000ff);
#else
    col.r = (rgb & 0x000000ff);
    col.g = (rgb & 0x0000ff00) >> 8;
    col.b = (rgb & 0x00ff0000) >> 16;
    col.a = (rgb & 0xff000000) >> 24;
#endif

    unlock(surface);

    return (col);
}
