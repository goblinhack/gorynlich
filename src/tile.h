/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info.
 */

#pragma once

#include "tile_private.h"
#include "glapi.h"

uint8_t tile_init(void);
void tile_fini(void);
void tile_load(const char *file, uint32_t width, uint32_t height,
               uint32_t nargs, ...);
void tile_load_arr(const char *tex, 
                   const char *tex_black_and_white,
                   uint32_t width, uint32_t height,
                   uint32_t nargs, const char *arr[]);
tilep tile_find(const char *name);
tilep tile_from_surface(SDL_Surface *surface,
                      const char *optional_file,
                      const char *name);
void tile_blit_mask_fat(tilep tile, char *name, fpoint tl, fpoint br);
void blit_quad(point tl, point br);
int32_t tile_get_gl_binding(tilep);
int32_t tile_get_width(tilep);
int32_t tile_get_height(tilep);
const char *tile_name(tilep);
texp tile_get_tex(tilep);
uint32_t tile_get_index(tilep);
tilep string2tile(const char **s);
void tile_get_coords(tilep, float *x1, float *y1, float *x2, float *y2);

/*
 * Blits a whole tile. Y co-ords are inverted.
 */
static inline
void tile_blit_fat (tile *tile, char *name, fpoint tl, fpoint br)
{
#if 0
    if (!tile) {
        if (!name) {
            DIE("no name for tile blit");
        }

        tile = tile_find(name);
    }
#endif
    if (!tile) {
        return;
    }

    blit(tile->gl_surface_binding,
         tile->x1, tile->y2, tile->x2, tile->y1, tl.x, br.y, br.x, tl.y);
}

/*
 * Blits a whole tile. Y co-ords are inverted.
 */
static inline
void tile_blit_fat_black_and_white (tile *tile, char *name, fpoint tl, fpoint br)
{
#if 0
    if (!tile) {
        if (!name) {
            DIE("no name for tile blit");
        }

        tile = tile_find(name);
    }
#endif
    if (!tile) {
        return;
    }

    if (!tile->gl_surface_binding_black_and_white) {
        blit(tile->gl_surface_binding,
            tile->x1, tile->y2, tile->x2, tile->y1, tl.x, br.y, br.x, tl.y);
    } else {
        blit(tile->gl_surface_binding_black_and_white,
            tile->x1, tile->y2, tile->x2, tile->y1, tl.x, br.y, br.x, tl.y);
    }
}

/*
 * Blits a whole tile.
 */
static inline
void tile_blit_at (tile *tile, char *name, point tl, point br)
{
#if 0
    if (!tile) {
        if (!name) {
            DIE("no name for tile blit");
        }

        tile = tile_find(name);
    }
#endif

    blit(tile->gl_surface_binding,
         tile->x1, tile->y2, tile->x2, tile->y1, tl.x, tl.y, br.x, br.y);
}

/*
 * Blits a whole tile.
 */
static inline
void tile_blit (tile *tile, char *name, point at)
{
    point tl, br;

#if 0
    if (!tile) {
        if (!name) {
            DIE("no name for tile blit");
        }

        tile = tile_find(name);
    }
#endif

    tl.x = at.x - tile->width/2;
    br.y = at.y - tile->height/2;
    br.x = at.x + tile->width/2;
    tl.y = at.y + tile->height/2;

    tile_blit_at(tile, name, tl, br);
}
