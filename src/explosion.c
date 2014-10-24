/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include "slre.h"

#include "main.h"
#include "math.h"
#include "level.h"
#include "thing.h"
#include "wid_game_map_server.h"
#include "map.h"
#include "vision.h"

int level_explosion_flash_effect;

/*
 * Place an explosion
 */
static uint8_t level_place_explosion_at (levelp level,
                                         thingp owner,
                                         double ox, 
                                         double oy, 
                                         double x, 
                                         double y, 
                                         uint8_t dist,
                                         uint32_t nargs,
                                         va_list args)
{
    /*
     * Choose one of the things in the args list to place.
     */
    uint32_t r = (rand() % nargs) + 1;

    const char *name = 0;
    while (r--) {
        name = va_arg(args, char *);
    }

    if (!name) {
        DIE("cannot place explosion thing");
    }

    thing_templatep tp = thing_template_find(name);
    if (!tp) {
        DIE("no explosion for name %s", name);
    }

    double delay = DISTANCE(ox, oy, x, y) * 150;

    /*
     * Make the delay on the server a lot smaller so we don't see things die 
     * after the explosion.
     */
    if (level == server_level) {
        delay *= 0.75;
    }

    thing_place_and_destroy_timed(tp,
                                  owner,
                                  x,
                                  y,
                                  delay,
                                  3000, // destroy in
                                  100, // jitter
                                  level == server_level ? 1 : 0,
                                  dist == 0 ? 1 : 0);

    return (true);
}

static uint8_t this_explosion[MAP_WIDTH][MAP_HEIGHT];
static uint8_t this_explosion_x;
static uint8_t this_explosion_y;
static uint8_t this_explosion_radius;

static void explosion_flood (levelp level, uint8_t x, uint8_t y)
{
    if (x < MAP_BORDER) {
        return;
    }

    if (y < MAP_BORDER) {
        return;
    }

    if (x > MAP_WIDTH - MAP_BORDER) {
        return;
    }

    if (y > MAP_HEIGHT - MAP_BORDER) {
        return;
    }

    if (this_explosion[x][y]) {
        return;
    }

    uint8_t distance = DISTANCE(x, y, this_explosion_x, this_explosion_y);

    if (distance > this_explosion_radius) {
        return;
    }

    if (map_find_wall_at(level, x, y, 0) ||
        map_find_door_at(level, x, y, 0) ||
        map_find_rock_at(level, x, y, 0)) {
        /*
         * Don't go any further but allow the explosion to overlap into this 
         * tile.
         */
        this_explosion[x][y] = distance;
        return;
    }

    if (!can_see(level, x, y, this_explosion_x, this_explosion_y)) {
        /*
         * Don't go any further but allow the explosion to overlap into this 
         * tile.
         */
        this_explosion[x][y] = distance;
        return;
    }

    this_explosion[x][y] = distance;
    explosion_flood(level, x-1, y);
    explosion_flood(level, x+1, y);
    explosion_flood(level, x, y-1);
    explosion_flood(level, x, y+1);
}

#ifdef DEBUG_EXPLOSION
static FILE *fp;

static void debug_explosion (levelp level)
{
    int32_t x;
    int32_t y;
    widp w;
                
    if (!fp) {
        fp = fopen("exp.txt","w");
    }

    if (level == server_level) {
        fprintf(fp,"test server level %p\n", level);
    } else {
        fprintf(fp,"test client level %p\n",level);
    }

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {

            widp mywid = 0;

            if (map_find_wall_at(level, x, y, &w)) {
                fprintf(fp,"x");
                mywid = w;
            } else if (map_find_pipe_at(level, x, y, &w)) {
                mywid = w;
            } else if (map_find_door_at(level, x, y, &w)) {
                fprintf(fp,"D");
                mywid = w;
            }

            if (!mywid) {
                fprintf(fp," ");
                continue;
            }
        }
        fprintf(fp,"\n");
    }
    fprintf(fp,"\n");
    fprintf(fp,"\n");
}
#endif

/*
 * Place an explosion
 */
static void level_place_explosion_ (levelp level, 
                                    thingp owner,
                                    double x, 
                                    double y,
                                    int radius,
                                    uint32_t nargs, ...)
{
    va_list args;

    x += 0.5;
    y += 0.5;

    /*
     * However, as the player can move close to walls, the current tile might 
     * end up being a wall. If this is the case, look around for a closer tile 
     * that has no wall.
     */
    if (map_find_wall_at(level, x, y, 0) ||
        map_find_door_at(level, x, y, 0) ||
        map_find_rock_at(level, x, y, 0)) {

        int32_t dx, dy;
        double best_x, best_y;
        double best_distance;
        int gotone = false;

        best_x = -1;
        best_y = -1;
        best_distance = 999;

        for (dx = -1; dx <= 1; dx++) {
            for (dy = -1; dy <= 1; dy++) {
                double tx, ty;

                tx = x + (double)dx;
                ty = y + (double)dy;

                if (map_find_wall_at(level, tx, ty, 0) ||
                    map_find_door_at(level, tx, ty, 0) ||
                    map_find_rock_at(level, tx, ty, 0)) {
                    continue;
                }

                double distance = DISTANCE(x, y, tx, ty);
                if (!gotone || (distance < best_distance)) {
                    best_distance = distance;
                    best_x = tx;
                    best_y = ty;
                    gotone = true;
                }
            }
        }

        if (gotone) {
            x = best_x;
            y = best_y;
        }
    }

    x = (int)x;
    y = (int)y;

    /*
     * Record the start of this explosion. We will do a map flood fill to find 
     * out the extent of the detonation.
     */
    memset(this_explosion, 0, sizeof(this_explosion));
    this_explosion_x = x;
    this_explosion_y = y;
    this_explosion_radius = radius;
    explosion_flood(level, x, y);

    uint32_t ix, iy;

#ifdef DEBUG_EXPLOSION
    debug_explosion(level);

    for (iy = 1; iy < MAP_HEIGHT - 1; iy++) {
        for (ix = 1; ix < MAP_WIDTH - 1; ix++) {
            printf("%u", this_explosion[ix][iy]);
        }
            printf("\n");
    }

    for (iy = 1; iy < MAP_HEIGHT - 1; iy++) {
        for (ix = 1; ix < MAP_WIDTH - 1; ix++) {
            if (map_find_wall_at(level, ix, iy, 0) ||
                map_find_rock_at(level, ix, iy, 0)) {
            printf("+");
            continue;
            }
            printf(" ");
        }
            printf("\n");
    }
#endif
    /*
     * Place the epicenter. This is what on the server gets sent to the 
     * client.
     */
    va_start(args, nargs);

    (void) level_place_explosion_at(level, 
                                    owner,
                                    x,
                                    y,
                                    x, 
                                    y, 
                                    0,
                                    nargs, args);
    va_end(args);

    for (ix = 1; ix < MAP_WIDTH - 1; ix++) {
        for (iy = 1; iy < MAP_HEIGHT - 1; iy++) {
            int8_t distance = this_explosion[ix][iy];
            if (!distance) {
                continue;
            }

            if (distance > radius) {
                continue;
            }

            double density = 0.5;
            double dx, dy;

            for (dx = -0.5; dx < 0.5; dx += density) {
                for (dy = -0.5; dy < 0.5; dy += density) {
                    dx = 0;dy = 0;
                    double ex = ix + dx;
                    double ey = iy + dy;

                    va_start(args, nargs);

                    (void) level_place_explosion_at(level, 
                                                    owner,
                                                    x,
                                                    y,
                                                    ex, 
                                                    ey, 
                                                    distance,
                                                    nargs, args);
                    va_end(args);
                    break;
                }
            }
        }
    }
}

void level_place_explosion (levelp level, 
                            thingp owner,
                            double x, double y)
{
    level_explosion_flash_effect = 20;

    level_place_explosion_(level, 
                           owner,
                           x, y,
                           6, // radius
                           4, // nargs
                           "data/things/explosion1",
                           "data/things/explosion2",
                           "data/things/explosion3",
                           "data/things/explosion4");
}

void level_place_small_explosion (levelp level, 
                                  thingp owner,
                                  double x, double y)
{
    level_explosion_flash_effect = 5;

    level_place_explosion_(level, 
                           owner,
                           x, y,
                           1, // radius
                           4, // nargs
                           "data/things/explosion1",
                           "data/things/explosion2",
                           "data/things/explosion3",
                           "data/things/explosion4");
}

void level_place_hit_success (levelp level, 
                              thingp owner,
                              double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           0, // radius
                           1, // nargs
                           "data/things/hit_success");
}

void level_place_hit_miss (levelp level, 
                          thingp owner,
                          double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           0, // radius
                           1, // nargs
                           "data/things/hit_miss");
}

void level_place_blood (levelp level, 
                        thingp owner,
                        double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           0, // radius
                           1, // nargs
                           "data/things/blood1");
}

void level_place_blood_crit (levelp level, 
                             thingp owner,
                             double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           0, // radius
                           1, // nargs
                           "data/things/blood2");
}

void level_place_fireball (levelp level, 
                           thingp owner,
                           double x, double y)
{
    level_explosion_flash_effect = 20;

    level_place_explosion_(level, 
                           owner,
                           x, y,
                           7, // radius
                           4, // nargs
                           "data/things/explosion1",
                           "data/things/explosion2",
                           "data/things/explosion3",
                           "data/things/explosion4");
}

void level_place_poison (levelp level, 
                         thingp owner,
                         double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           9, // radius
                           4, // nargs
                           "data/things/poison1",
                           "data/things/poison2",
                           "data/things/poison3",
                           "data/things/poison4");
}

void level_place_cloudkill (levelp level, 
                            thingp owner,
                            double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           12, // radius
                           4, // nargs
                           "data/things/cloudkill1",
                           "data/things/cloudkill2",
                           "data/things/cloudkill3",
                           "data/things/cloudkill4");
}

void level_place_small_cloudkill (levelp level, 
                                  thingp owner,
                                  double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           2, // radius
                           4, // nargs
                           "data/things/cloudkill1",
                           "data/things/cloudkill2",
                           "data/things/cloudkill3",
                           "data/things/cloudkill4");
}
