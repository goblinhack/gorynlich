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
void map_combine (map_frame_ctx_t *map)
{
    int32_t x, y, z;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            for (z = 0; z < MAP_DEPTH; z++) {
                if (map->tiles_copy[x][y][z].thing_template) {
                    map_set(map, x, y, z, 
                            map->tiles_copy[x][y][z].thing_template);
                    continue;
                }
            }
        }
    }
}

/*
 * map_init_tiles
 */
void map_init_tiles (map_frame_ctx_t *map)
{
#if 0
    int32_t z;

    for (z = 0; z < 1; z++) {
        srand(10);

        cave_gen(map, ROCK_0,
                 z, /* z */
                 40, /* fill prob */
                 5,  /* R1 */
                 2,  /* R2 */
                 10 + z   /* generations */);
    }

    memcpy(map->tiles_copy, map->tiles, sizeof(map->tiles));
    memset(map->tiles, 0, sizeof(map->tiles));

    fractal_gen(map, 200.5, 0.3, ROCK_0, WALL_0);

    map_combine(map);
//    fractal_gen(map, 20.5, 0.85, ROCK_0, WALL_0);
#endif

    int32_t z;
    int32_t gen;
    thing_templatep t;

    for (z = 0; z < 10; z++) {

        switch (z) {
        case 0: gen = 5; t = WALL_0; break;
        case 1: gen = 4; t = WALL_0; break;
        case 2: gen = 3; t = WALL_0; break;
        case 3: gen = 2; t = WALL_0; break;
        case 4: gen = 1; t = WALL_0; break;
        case 5: gen = 1; t = WALL_0; break;
        case 6: gen = 1; t = WALL_0; break;
        case 7: gen = 1; t = WALL_0; break;
        case 8: gen = 1; t = WALL_0; break;
        case 9: gen = 0; t = ROCK_0; break;
        default: DIE("bug");
        }

        srand(10);
        cave_gen(map, t,
                 z, /* z */
                 30, /* fill prob */
                 5,  /* R1 */
                 2,  /* R2 */
                 gen  /* generations */);
    }

    memcpy(map->tiles_copy, map->tiles, sizeof(map->tiles));

    memset(map->tiles, 0, sizeof(map->tiles));

    cave_gen(map, 
             WALL_0,
             0, /* z */
             100, /* fill prob */
             5,  /* R1 */
             2,  /* R2 */
             3  /* generations */);

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
