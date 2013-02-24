/*
    Retro.
    Copyright (C) 1999, 2000 Neil McGill

    This game is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This game is distributed in the hope that it will be fun,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this game; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Neil McGill

    $Id: pixel.c 1037 2012-08-22 11:55:24Z 4670 $
*/

#include <SDL.h>

#include "main.h"
#include "gl.h"
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
//        DIE("out of range x %u, max %u", x, surface->w);
        return;
    }

    if (y >= (uint32_t)surface->h) {
//        DIE("out of range y %u, max %u", y, surface->h);
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
            return;
        case 24:
            putPixel_24x32bpp(surface, x, y, rgb);
            return;
        case 32:
            putPixel_32x32bpp(surface, x, y, rgb);
            return;
        case 48:
            putPixel_48x32bpp(surface, x, y, rgb);
            return;
        case 64:
            putPixel_64x32bpp(surface, x, y, rgb);
            return;
        case 128:
            putPixel_128x32bpp(surface, x, y, rgb);
            return;
        case 256:
            putPixel_256x32bpp(surface, x, y, rgb);
            return;
        case 320:
            putPixel_320x32bpp(surface, x, y, rgb);
            return;
        case 640:
            putPixel_640x32bpp(surface, x, y, rgb);
            return;
        case 800:
            putPixel_800x32bpp(surface, x, y, rgb);
            return;
        case 1024:
            putPixel_1024x32bpp(surface, x, y, rgb);
            return;
        default:
            putPixel_32bpp(surface, x, y, rgb);
            return;
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
        DIE("out of range");
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
