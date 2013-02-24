/*
    goblinhack.
    Copyright (C) 1999-2010 Neil McGill

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

    $Id: getput_pixel_macros.h 513 2007-05-07 01:46:54Z goblinhack $
*/

#ifndef _GETPUT_PIXEL_MACROS_H_
#define _GETPUT_PIXEL_MACROS_H_

/*
 * Quick row-color bit-shift lookups for certain common color screen size widths:
 *
 * <<1  2
 * <<2  4
 * <<3  8
 * <<4  16
 * <<5  32
 * <<6  64
 * <<7  128
 * <<8  256
 * <<9  512
 * <<10 1024
 */

#define pixel_16w(y)     ((y)<<4)
#define pixel_24w(y)     ((y)<<4)+((y)<<3)
#define pixel_32w(y)     ((y)<<5)
#define pixel_48w(y)     ((y)<<5)+((y)<<4)
#define pixel_64w(y)     ((y)<<6)
#define pixel_128w(y)    ((y)<<7)
#define pixel_256w(y)    ((y)<<8)
#define pixel_320w(y)    (((y)<<8)+((y)<<6))
#define pixel_512w(y)    ((y)<<9)
#define pixel_640w(y)    (((y)<<9)+((y)<<7))
#define pixel_800w(y)    (((y)<<9)+((y)<<8)+((y)<<5))
#define pixel_1024w(y)   (((y)<<10))
#define pixel_ANYw(s,y)  ((y)*(s->pitch/s->format->BytesPerPixel))

/*
 * 32 bpp macros:
 */
#define pixel_16x32bpp(s, x, y) ((uint32_t*)s->pixels+(x)+pixel_16w(y))
#define pixel_24x32bpp(s, x, y) ((uint32_t*)s->pixels+(x)+pixel_24w(y))
#define pixel_32x32bpp(s, x, y) ((uint32_t*)s->pixels+(x)+pixel_32w(y))
#define pixel_48x32bpp(s, x, y) ((uint32_t*)s->pixels+(x)+pixel_48w(y))
#define pixel_64x32bpp(s, x, y) ((uint32_t*)s->pixels+(x)+pixel_64w(y))
#define pixel_128x32bpp(s, x, y)    ((uint32_t*)s->pixels+(x)+pixel_128w(y))
#define pixel_256x32bpp(s, x, y)    ((uint32_t*)s->pixels+(x)+pixel_256w(y))
#define pixel_320x32bpp(s, x, y)    ((uint32_t*)s->pixels+(x)+pixel_320w(y))
#define pixel_640x32bpp(s, x, y)    ((uint32_t*)s->pixels+(x)+pixel_640w(y))
#define pixel_800x32bpp(s, x, y)    ((uint32_t*)s->pixels+(x)+pixel_800w(y))
#define pixel_1024x32bpp(s, x, y)   ((uint32_t*)s->pixels+(x)+pixel_1024w(y))
#define pixel_32bpp(s, x, y)        ((uint32_t*)s->pixels+(x)+pixel_ANYw(s,y))

#define putPixel_16x32bpp(s, x, y, rgb)     *pixel_16x32bpp(s, x, y) = rgb
#define putPixel_24x32bpp(s, x, y, rgb)     *pixel_24x32bpp(s, x, y) = rgb
#define putPixel_32x32bpp(s, x, y, rgb)     *pixel_32x32bpp(s, x, y) = rgb
#define putPixel_48x32bpp(s, x, y, rgb)     *pixel_48x32bpp(s, x, y) = rgb
#define putPixel_64x32bpp(s, x, y, rgb)     *pixel_64x32bpp(s, x, y) = rgb
#define putPixel_128x32bpp(s, x, y, rgb)    *pixel_128x32bpp(s, x, y) = rgb
#define putPixel_256x32bpp(s, x, y, rgb)    *pixel_256x32bpp(s, x, y) = rgb
#define putPixel_320x32bpp(s, x, y, rgb)    *pixel_320x32bpp(s, x, y) = rgb
#define putPixel_640x32bpp(s, x, y, rgb)    *pixel_640x32bpp(s, x, y) = rgb
#define putPixel_800x32bpp(s, x, y, rgb)    *pixel_800x32bpp(s, x, y) = rgb
#define putPixel_1024x32bpp(s, x, y, rgb)   *pixel_1024x32bpp(s, x, y) = rgb
#define putPixel_32bpp(s, x, y, rgb)        *pixel_32bpp(s, x, y) = rgb

#define getPixel_16x32bpp(s, x, y, rgb)     *(rgb) = *pixel_16x32bpp(s, x, y)
#define getPixel_24x32bpp(s, x, y, rgb)     *(rgb) = *pixel_24x32bpp(s, x, y)
#define getPixel_32x32bpp(s, x, y, rgb)     *(rgb) = *pixel_32x32bpp(s, x, y)
#define getPixel_48x32bpp(s, x, y, rgb)     *(rgb) = *pixel_48x32bpp(s, x, y)
#define getPixel_64x32bpp(s, x, y, rgb)     *(rgb) = *pixel_64x32bpp(s, x, y)
#define getPixel_128x32bpp(s, x, y, rgb)    *(rgb) = *pixel_128x32bpp(s, x, y)
#define getPixel_256x32bpp(s, x, y, rgb)    *(rgb) = *pixel_256x32bpp(s, x, y)
#define getPixel_320x32bpp(s, x, y, rgb)    *(rgb) = *pixel_320x32bpp(s, x, y)
#define getPixel_640x32bpp(s, x, y, rgb)    *(rgb) = *pixel_640x32bpp(s, x, y)
#define getPixel_800x32bpp(s, x, y, rgb)    *(rgb) = *pixel_800x32bpp(s, x, y)
#define getPixel_1024x32bpp(s, x, y, rgb)   *(rgb) = *pixel_1024x32bpp(s, x, y)
#define getPixel_32bpp(s, x, y, rgb)        *(rgb) = *pixel_32bpp(s, x, y)

/*
 * Public interface. Clips to surface and performs locking. Slow.
 */
void putPixel(SDL_Surface *dst, uint32_t x, uint32_t, color pix);

/*
 * Public interface. Clips to surface and performs locking. Slow.
 */
color getPixel(SDL_Surface *dst, uint32_t x, uint32_t y);

#endif
