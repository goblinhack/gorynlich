/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "map.h"
#include "wid.h"
#include "thing.h"
#include "thing_template.h"
#include "thing_tile.h"
#include "level.h"
#include "level_private.h"
#include "tile.h"

/*
 * map_combine
 */
static void map_combine (map_frame_ctx_t *map)
{
    int32_t x, y, z;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            if (map->tiles_copy[x][y][0].thing_template) {
                map_set(map, x, y, 0, ROCK);
                continue;
            }

            for (z = 0; z < MAP_DEPTH; z++) {
                map_set(map, x, y, z, 0);
            }
        }
    }
}

/*
 * map_init_tiles
 */
void map_init_tiles (map_frame_ctx_t *map)
{
    cave_gen(map, ROCK, 0);

    memcpy(map->tiles_copy, map->tiles, sizeof(map->tiles));
    memset(map->tiles, 0, sizeof(map->tiles));

    fractal_gen(map, 100.5, 0.55, ROCK, WALL_1);

    map_combine(map);
}

/*
 * map_set
 */
thing_templatep map_set (map_frame_ctx_t *map,
                         int32_t x, int32_t y, int32_t z,
                         thing_templatep thing_template)
{
    if (map_out_of_bounds(x, y, z)) {
        DIE("out of bounds on set map at %d,%d,%d for %s", x, y, z,
            thing_template_shortname(thing_template));
        return (thing_template);
    }

    map->tiles[x][y][z].thing_template = thing_template;

    /*
     * Allow clearing.
     */
    if (!thing_template) {
        map->tiles[x][y][z].tile = 0;
        return (thing_template);
    }

    tree_rootp thing_tiles = thing_template_get_tiles(thing_template);
    if (!thing_tiles) {
        return (thing_template);
    }

    thing_tilep thing_tile = (typeof(thing_tile)) tree_root_first(thing_tiles);
    if (!thing_tile) {
        return (thing_template);
    }

    const char *tilename = thing_tile_name(thing_tile);
    if (!tilename) {
        DIE("tile null name from thing %s not found",
            thing_template_shortname(thing_template));
    }

    tilep tile = tile_find(tilename);
    if (!tile) {
        DIE("tile name %s from thing %s not found",
            tilename,
            thing_template_shortname(thing_template));
    }

    uint32_t index = tile_get_index(tile);

    map->tiles[x][y][z].tile = index;

    return (thing_template);
}

/*
 * map_get
 */
thing_templatep map_get (map_frame_ctx_t *map,
                         int32_t x, int32_t y, int32_t z)
{
    if (map_out_of_bounds(x, y, z)) {
        ERR("out of bounds on get map at %d,%d,%d", x, y, z);
        return (0);
    }

    return (map->tiles[x][y][z].thing_template);
}
