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
#include "map_jigsaw.h"

/*
 * map_combine
 */
void map_combine (map_t *map)
{
    int32_t x, y;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            if (map->tiles_copy[x][y].template_id) {
                map_set_thing_template(map, x, y,
                                       map->tiles_copy[x][y].template_id);
                continue;
            }
        }
    }
}

/*
 * map_init_tiles
 */
void map_init_tiles (map_t *map)
{
    uint16_t t;

#if 0
        cave_gen(map, ROCK_0_ID,
                 0, /* z */
                 40, /* fill prob */
                 5,  /* R1 */
                 2,  /* R2 */
                 10   /* generations */);
#endif

#if 0
    memcpy(map->tiles_copy, map->tiles, sizeof(map->tiles));
    memset(map->tiles, 0, sizeof(map->tiles));

    fractal_gen(map, 200.5, 0.3, ROCK_0_ID, BRICK_0_ID);

    map_combine(map);
    fractal_gen(map, 20.5, 0.85, ROCK_0_ID, BRICK_0_ID);
#endif

#if 0
    int32_t z;

t = ROCK_1_ID;

        cave_gen(map, t,
                 0, /* z */
                 30, /* fill prob */
                 5,  /* R1 */
                 2,  /* R2 */
                 10  /* generations */);

    memcpy(map->tiles_copy, map->tiles, sizeof(map->tiles));

    memset(map->tiles, 0, sizeof(map->tiles));

    map_combine(map);
#endif

    int32_t x;
    int32_t y;
    uint8_t c;

    for (x = 0; x < MAP_JIGSAW_BUFFER2_WIDTH; x++) {
        for (y = 0; y < MAP_JIGSAW_BUFFER2_HEIGHT; y++) {
            c = map_jigsaw_buffer2_getchar(x, y);
            switch (c) {
            case MAP_GRASS:
                t = ROCK_0_ID;
                break;
            case MAP_WALL:
                t = ROCK_1_ID;
                break;
            case MAP_ROCK:
                t = BRICK_0_ID;
                break;
            case MAP_LAVA:
                t = LAVA_0_ID;
                break;
            case MAP_CORRIDOR_WALL:
                t = BRICK_3_ID;
                break;
            case MAP_WATER:
                t = WATER_0_ID;
                break;
            default:
                t = 0;
                break;
            }

            uint32_t mx = ((MAP_WIDTH - MAP_JIGSAW_BUFFER2_WIDTH) / 2) + x;
            uint32_t my = ((MAP_HEIGHT - MAP_JIGSAW_BUFFER2_HEIGHT) / 2) + y;

            if (t) {
                map_set_thing_template(map, mx, my, t);
            }
        }
    }

    int d;
    for (d = 0; d < 10; d++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            map_set_thing_template(map, x, d, ROCK_1_ID);
            map_set_thing_template(map, x, MAP_HEIGHT - d - 1, ROCK_1_ID);
        }

        for (y = 0; y < MAP_HEIGHT; y++) {
            map_set_thing_template(map, d, y, ROCK_1_ID);
            map_set_thing_template(map, MAP_WIDTH - d - 1, y, ROCK_1_ID);
        }
    }
    
}

/*
 * map_set_thing_template
 */
uint16_t map_set_thing_template (map_t *map, int32_t x, int32_t y,
                                 uint16_t template_id)
{
    thing_templatep thing_template = id_to_thing_template(template_id);

    if (map_out_of_bounds(x, y)) {
        DIE("out of bounds on set map at %d,%d for %s", x, y,
            thing_template_shortname(thing_template));
        return (template_id);
    }

    map_tile_t *map_tile = &map->tiles[x][y];
    if (template_id == map_tile->template_id) {
        return (template_id);
    }

    map_tile->template_id = template_id;

    /*
     * Allow clearing.
     */
    if (!template_id) {
        map_tile->tile = 0;
        return (template_id);
    }

    tree_rootp thing_tiles = thing_template_get_tiles(thing_template);
    if (!thing_tiles) {
        map_tile->tile = 0;
        return (template_id);
    }

    thing_tilep thing_tile = (typeof(thing_tile)) tree_root_first(thing_tiles);
    if (!thing_tile) {
        map_tile->tile = 0;
        return (template_id);
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

    map_tile->tile = index;

    return (template_id);
}

/*
 * map_get
 */
uint16_t map_get (map_t *map, int32_t x, int32_t y)
{
    if (map_out_of_bounds(x, y)) {
        ERR("out of bounds on get map at %d,%d", x, y);
        return (0);
    }

    return (map->tiles[x][y].template_id);
}

/*
 * map_get_thing_template
 */
thing_templatep map_get_thing_template (map_t *map, int32_t x, int32_t y)
{
    thing_templatep thing_template;
    uint16_t template_id;

    template_id = map_get(map, x, y);
    if (!template_id) {
        return (0);
    }

    thing_template = id_to_thing_template(template_id);

    return (thing_template);
}
