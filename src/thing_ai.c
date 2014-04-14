/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#define __STDC_LIMIT_MACROS
#include <SDL.h>

#include "main.h"
#include "thing.h"
#include "level_private.h"
#include "wid_game_map_server.h"
#include "map.h"

static FILE *fp;
static const int8_t not_preferred = 63;

/*
 * Print the Dijkstra map scores shared by all things of the same type.
 */
static void dmap_print (thing_templatep t, levelp level)
{
    int8_t x;
    int8_t y;

    if (!fp) {
        fp = fopen("map.txt", "w");
    }

    for (y = 0; y < TILES_MAP_HEIGHT; y++) {
        for (x = 0; x < TILES_MAP_WIDTH; x++) {
            if (map_is_wall_at(level, x, y)) {
                fprintf(fp, " ## ");
                continue;
            }
            if (map_is_bomb_at(level, x, y)) {
                fprintf(fp, " Bo ");
                continue;
            }
            if (map_is_exit_at(level, x, y)) {
                fprintf(fp, " Ex ");
                continue;
            }
            if (map_is_spam_at(level, x, y)) {
                fprintf(fp, " Sp ");
                continue;
            }
            if (map_is_door_at(level, x, y)) {
                fprintf(fp, " Do ");
                continue;
            }
            if (map_is_generator_at(level, x, y)) {
                fprintf(fp, " Gn ");
                continue;
            }
            if (map_is_food_at(level, x, y)) {
                fprintf(fp, " Fd ");
                continue;
            }

            if (!map_is_floor_at(level, x, y)) {
                fprintf(fp, " -- ");
                continue;
            }

            if (level->monst_walls[x][y] != ' ') {
                fprintf(fp, " ## ");
                continue;
            }

            fprintf(fp, "%4d", t->dmap[x][y]);
        }

        fprintf(fp, "\n");
    }

    fprintf(fp, "\n");
}

/*
 * Print the Dijkstra map scores shared by all things of the same type.
 */
static void dmap_thing_print (thingp t, 
                              levelp level,
                              int8_t nexthop_x,
                              int8_t nexthop_y)
{
    thing_templatep temp = t->thing_template;
    int8_t x;
    int8_t y;

    if (!fp) {
        fp = fopen("map.txt", "w");
    }

    int8_t tx;
    int8_t ty;

    tx = (int)(t->x + 0.5);
    ty = (int)(t->y + 0.5);

    for (y = 0; y < TILES_MAP_HEIGHT; y++) {
        for (x = 0; x < TILES_MAP_WIDTH; x++) {
            if ((nexthop_x == x) && (nexthop_y == y)) {
                fprintf(fp, " Nh ");
            } else {
                if ((x == tx) && (y == ty)) {
                    fprintf(fp, " Mo ");
                } else {
                    fprintf(fp, "%4d", temp->dmap[x][y]);
                }
            }
        }

        fprintf(fp, "\n");
    }

    fprintf(fp, "\n");
}

/*
 * http://www.roguebasin.com/index.php?title=The_Incredible_Power_of_Dijkstra_Maps
 *
 * To get a Dijkstra map, you start with an integer array representing your
 * map, with some set of goal cells set to zero and all the rest set to a very
 * high number. 
 *
 * Iterate through the map's "floor" cells -- skip the impassable wall cells.
 * If any floor tile has a value that is at least 2 greater than its 
 * lowest-value floor neighbor, set it to be exactly 1 greater than its lowest 
 * value neighbor. Repeat until no changes are made. The resulting grid of 
 * numbers represents the number of steps that it will take to get from any 
 * given tile to the nearest goal. 
 */
static void dmap_process (thing_templatep t, levelp level)
{
    uint8_t x;
    uint8_t y;
    int8_t a;
    int8_t b;
    int8_t c;
    int8_t d;
    int8_t *e;
    int8_t f;
    int8_t g;
    int8_t h;
    int8_t i;
    int8_t lowest;
    uint8_t changed;

    do {
        changed = false;

        for (x = 1; x < TILES_MAP_WIDTH - 1; x++) {
            for (y = 1; y < TILES_MAP_HEIGHT - 1; y++) {
                if (level->monst_walls[x][y] != ' ') {
                    continue;
                }

                a =  t->dmap[x-1][y-1] * 2;
                b =  t->dmap[x  ][y-1];
                c =  t->dmap[x+1][y-1] * 2;

                d =  t->dmap[x-1][y];
                e = &t->dmap[x  ][y];
                f =  t->dmap[x+1][y];
                 
                g =  t->dmap[x-1][y+1] * 2;
                h =  t->dmap[x  ][y+1];
                i =  t->dmap[x+1][y+1] * 2;

                lowest = min(a, min(b, min(c, min(d, min(f, min(g, min(h,i)))))));

                if (*e - lowest >= 2) {
                    *e = lowest + 1;
                    changed = true;
                }
            }
        }
    } while (changed);
}

/*
 * Generate goal points with a low value.
 */
static uint32_t dmap_goals_set (thing_templatep t, boolean test)
{
    uint32_t checksum = 0;
    thingp thing_it;
    int8_t x;
    int8_t y;

    { TREE_OFFSET_WALK_UNSAFE(server_player_things, thing_it) {
        /*
         * Only chase players.
         */
        if (!thing_is_player_fast(thing_it)) {
            continue;
        }

        /*
         * Ignore dead players.
         */
        if (thing_is_dead_fast(thing_it)) {
            continue;
        }

        /*
         * Aim for center of tile.
         */
        x = (int)(thing_it->x + 0.5);
        y = (int)(thing_it->y + 0.5);

        if (!test) {
            t->dmap[x][y] = 0;
        }

        checksum ^= x | (y << 16);
        checksum = checksum << 1;
    } }

    return (checksum);
}

/*
 * Initialize the djkstra map with high values.
 */
static void dmap_init (thing_templatep t)
{
    int8_t x;
    int8_t y;

    for (x = 0; x < TILES_MAP_WIDTH; x++) {
        for (y = 0; y < TILES_MAP_HEIGHT; y++) {
            t->dmap[x][y] = not_preferred;
        }
    }
}

/*
 * Generate a djkstra map for the thing.
 */
static void dmap_generate (uint32_t i, levelp level)
{
    thing_templatep t = id_to_thing_template(i);

    t->dmap_valid = false;

    /*
     * If no level yet, there is nothing to chase.
     */
    if (!server_level) {
        return;
    }

    /*
     * Only reprocess the djkstra map if something has changed on the map
     * We use a checksum of the goals to indicate this with reasonable 
     * certainty.
     */
    uint32_t checksum = dmap_goals_set(t, true /* test */);
    if (!checksum) {
        return;
    }

    t->dmap_valid = true;
    if (t->dmap_checksum == checksum) {
        return;
    }

    t->dmap_checksum = checksum;

    dmap_init(t);
    dmap_goals_set(t, false /* test */); // redo for real this time.
    dmap_process(t, level);

#ifdef ENABLE_MAP_DEBUG
    if (1)
#else
    if (0)
#endif
    dmap_print(t, level);
}

/*
 * Generate a djkstra map for the thing.
e*/
void thing_generate_dmaps (void)
{
    dmap_generate(THING_GHOST, server_level);
}

boolean thing_find_nexthop (thingp t, int32_t *nexthop_x, int32_t *nexthop_y)
{
    thing_templatep temp = t->thing_template;

    if (!temp->dmap_valid) {
        return (false);
    }

    int8_t x;
    int8_t y;

    x = (int)(t->x + 0.5);
    y = (int)(t->y + 0.5);

    int8_t a;
    int8_t b;
    int8_t c;
    int8_t d;
    int8_t e;
    int8_t f;
    int8_t g;
    int8_t h;
    int8_t i;
    int8_t lowest;

    a = temp->dmap[x-1][y-1];
    b = temp->dmap[x  ][y-1];
    c = temp->dmap[x+1][y-1];

    d = temp->dmap[x-1][y];
    e = temp->dmap[x  ][y];
    f = temp->dmap[x+1][y];
        
    g = temp->dmap[x-1][y+1];
    h = temp->dmap[x  ][y+1];
    i = temp->dmap[x+1][y+1];

    lowest = min(a, min(b, min(c, min(d, min(e, min(f, min(g, min(h,i))))))));

    if (a != lowest) { a += not_preferred; }
    if (b != lowest) { b += not_preferred; }
    if (c != lowest) { c += not_preferred; }
    if (d != lowest) { d += not_preferred; }
    if (e != lowest) { e += not_preferred; }
    if (f != lowest) { f += not_preferred; }
    if (g != lowest) { g += not_preferred; }
    if (h != lowest) { h += not_preferred; }
    if (i != lowest) { i += not_preferred; }

    a += t->visited[x-1][y-1];
    b += t->visited[x  ][y-1];
    c += t->visited[x+1][y-1];

    d += t->visited[x-1][y];
    e += t->visited[x  ][y];
    f += t->visited[x+1][y];
        
    g += t->visited[x-1][y+1];
    h += t->visited[x  ][y+1];
    i += t->visited[x+1][y+1];

    lowest = min(a, min(b, min(c, min(d, min(e, min(f, min(g, min(h,i))))))));

    int8_t dx = 0;
    int8_t dy = 0;

    if (a == lowest) { dx = -1; dy = -1; }
    else if (b == lowest) { dx =  0; dy = -1; }
    else if (c == lowest) { dx = +1; dy = -1; }
    else if (d == lowest) { dx = -1; dy =  0; }
    else if (e == lowest) { dx =  0; dy =  0; }
    else if (f == lowest) { dx = +1; dy =  0; }
    else if (g == lowest) { dx = -1; dy =  1; }
    else if (h == lowest) { dx =  0; dy =  1; }
    else if (i == lowest) { dx = +1; dy =  1; }

    *nexthop_x = x + dx;
    *nexthop_y = y + dy;

#ifdef ENABLE_MAP_DEBUG
    if (1)
#else
    if (0)
#endif
    dmap_thing_print(t, server_level, *nexthop_x, *nexthop_y);

    return (true);
}
