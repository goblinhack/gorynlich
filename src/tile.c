/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>
#include "glapi.h"

#include "main.h"
#include "gl.h"
#include "tile.h"
#include "tree.h"
#include "tex.h"
#include "tile_private.h"

tree_root *tiles;

static boolean tile_init_done;

boolean tile_init (void)
{
    tile_init_done = true;

    return (true);
}

static void tile_destroy (tile *t)
{
}

void tile_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (tile_init_done) {
        tile_init_done = false;

        tree_destroy(&tiles, (tree_destroy_func)tile_destroy);
    }
}

void tile_load (const char *tex_name, uint32_t width, uint32_t height,
                uint32_t nargs, ...)
{
    va_list args;

    texp tex = tex_load(0, tex_name);
    float fw = 1.0 / (((float)tex_get_width(tex)) / ((float)width));
    float fh = 1.0 / (((float)tex_get_height(tex)) / ((float)height));
    uint32_t x = 0;
    uint32_t y = 0;

    va_start(args, nargs);

    while (nargs--) {

        const char *name = va_arg(args, char *);

        if (name) {
            tile *t;

            if (tile_find(name)) {
                DIE("tile name [%s] already used", name);
            }

            if (!tiles) {
                tiles = tree_alloc(TREE_KEY_STRING, "TREE ROOT: tile");
            }

            t = (typeof(t)) myzalloc(sizeof(*t), "TREE NODE: tile");
            t->tree.key = dupstr(name, "TREE KEY: tile");

            if (!tree_insert(tiles, &t->tree.node)) {
                DIE("tile insert name [%s] failed", name);
            }

            t->width = width;
            t->height = height;
            t->tex = tex;
            t->gl_surface_binding = tex_get_gl_binding(tex);

            t->x1 = fw * (float)(x);
            t->y1 = fh * (float)(y);
            t->x2 = t->x1 + fw;
            t->y2 = t->y1 + fh;

            DBG("Tile: -%10s %ux%u (%u, %u)", name, width, height, x, y);
        }

        x++;

        if (x * width >= tex_get_width(tex)) {
            x = 0;
            y++;
        }

        if (y * height > tex_get_height(tex)) {
            if (name) {
                DIE("overflow reading tiles [%s]", name);
            } else {
                DIE("overflow reading tiles");
            }
        }
    }

    va_end(args);
}

void tile_load_arr (const char *tex_name, uint32_t width, uint32_t height,
                    uint32_t nargs, const char *arr[])
{
    texp tex = tex_load(0, tex_name);
    float fw = 1.0 / (((float)tex_get_width(tex)) / ((float)width));
    float fh = 1.0 / (((float)tex_get_height(tex)) / ((float)height));
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t idx = 0;

    while (nargs--) {

        const char *name = arr[idx++];

        if (name) {
            tile *t;

            if (tile_find(name)) {
                DIE("tile name [%s] already used", name);
            }

            if (!tiles) {
                tiles = tree_alloc(TREE_KEY_STRING, "TREE ROOT: tile");
            }

            t = (typeof(t)) myzalloc(sizeof(*t), "TREE NODE: tile");
            t->tree.key = dupstr(name, "TREE KEY: tile");

            if (!tree_insert(tiles, &t->tree.node)) {
                DIE("tile insert name [%s] failed", name);
            }

            t->index = idx - 1;
            t->width = width;
            t->height = height;
            t->tex = tex;
            t->gl_surface_binding = tex_get_gl_binding(tex);

            t->x1 = fw * (float)(x);
            t->y1 = fh * (float)(y);
            t->x2 = t->x1 + fw;
            t->y2 = t->y1 + fh;

            DBG("Tile: %-10s %ux%u (%u, %u)", name, width, height, x, y);
        }

        x++;

        if (x * width >= tex_get_width(tex)) {
            x = 0;
            y++;
        }

        if (y * height > tex_get_height(tex)) {
            if (name) {
                DIE("overflow reading tile arr[%s]", name);
            } else {
                DIE("overflow reading tile arr at x %d y %d", x, y);
            }
        }
    }
}

/*
 * Find an existing tile.
 */
tilep tile_find (const char *name)
{
    tile target;
    tile *result;

    if (!name) {
        DIE("no name for tile find");
    }

    memset(&target, 0, sizeof(target));
    target.tree.key = (char*) name;

    result = (typeof(result)) tree_find(tiles, &target.tree.node);
    if (!result) {
        return (0);
    }

    verify(result->tex);

    return (result);
}

int32_t tile_get_gl_binding (tile *tile)
{
    return (tile->gl_surface_binding);
}

int32_t tile_get_width (tile *tile)
{
    return (tile->width);
}

int32_t tile_get_height (tile *tile)
{
    return (tile->height);
}

texp tile_get_tex (tile *tile)
{
    return (tile->tex);
}

uint32_t tile_get_index (tile *tile)
{
    return (tile->index);
}

const char *tile_name (tile *tile)
{
    return (tile->tree.key);
}

/*
 * Blits a whole tile. Y co-ords are inverted.
 */
void tile_blit_fat (tile *tile, char *name, fpoint tl, fpoint br)
{
    if (!tile) {
        if (!name) {
            DIE("no name for tile blit");
        }

        tile = tile_find(name);
    }

    glBindTexture(GL_TEXTURE_2D, tile->gl_surface_binding);

    blit(tile->x1, tile->y2, tile->x2, tile->y1, tl.x, br.y, br.x, tl.y);
}

/*
 * Blits a whole tile.
 */
void tile_blit_at (tile *tile, char *name, point tl, point br)
{
    if (!tile) {
        if (!name) {
            DIE("no name for tile blit");
        }

        tile = tile_find(name);
    }

    glBindTexture(GL_TEXTURE_2D, tile->gl_surface_binding);

    blit(tile->x1, tile->y2, tile->x2, tile->y1, tl.x, tl.y, br.x, br.y);
}

/*
 * Blits a whole tile.
 */
void tile_blit (tile *tile, char *name, point at)
{
    point tl, br;

    if (!tile) {
        if (!name) {
            DIE("no name for tile blit");
        }

        tile = tile_find(name);
    }

    tl.x = at.x - tile->width/2;
    br.y = at.y - tile->height/2;
    br.x = at.x + tile->width/2;
    tl.y = at.y + tile->height/2;

    tile_blit_at(tile, name, tl, br);
}

void tile_get_coords (tile *tile, float *x1, float *y1, float *x2, float *y2)
{
    *x1 = tile->x1;
    *y1 = tile->x1;
    *x2 = tile->x2;
    *x2 = tile->x2;
}

tilep string2tile (const char **s)
{
    static char tmp[MAXSTR];
    static const char *eo_tmp = tmp + MAXSTR;
    const char *c = *s;
    char *t = tmp;

    while (t < eo_tmp) {
        if ((*c == '\0') || (*c == '$')) {
            break;
        }

        *t++ = *c++;
    }

    if (c == eo_tmp) {
        return (0);
    }

    *t++ = '\0';
    *s += (t - tmp);

    tile find;
    tile *target;

    memset(&find, 0, sizeof(find));
    find.tree.key = tmp;

    target = (typeof(target)) tree_find(tiles, &find.tree.node);
    if (!target) {
        DIE("unknown tile [%s]", tmp);
    }

    return (target);
}
