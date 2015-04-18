/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include "slre.h"

#include "main.h"
#include "math_util.h"
#include "level.h"
#include "thing.h"
#include "wid_game_map_server.h"
#include "map.h"
#include "vision.h"
#include "wid.h"

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
    uint32_t r = (myrand() % nargs) + 1;

    const char *name = 0;
    while (r--) {
        name = va_arg(args, char *);
    }

    if (!name) {
        DIE("cannot place explosion thing");
    }

    tpp tp = tp_find(name);
    if (!tp) {
        DIE("no explosion for name %s", name);
    }

    double delay = DISTANCE(ox, oy, x, y) * 150;

    /*
     * Make the delay on the server a lot smaller so we don't see things die 
     * after the explosion.
     */
    uint32_t destroy_in;
    uint32_t jitter;

    if (level == server_level) {
        destroy_in = 100;
        jitter = 10;
    } else {
        destroy_in = 3000;
        jitter = 100;
    }

    thing_place_and_destroy_timed(level,
                                  tp,
                                  owner,
                                  x,
                                  y,
                                  delay,
                                  destroy_in,
                                  jitter,
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
    if (x < 1) {
        return;
    }

    if (y < 1) {
        return;
    }

    if (x > MAP_WIDTH - 1) {
        return;
    }

    if (y > MAP_HEIGHT - 1) {
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

#if 0
    /*
     * Why do this ? Explosions can't bend around corners.
     */
    if (!can_see(level, x, y, this_explosion_x, this_explosion_y)) {
        /*
         * Don't go any further but allow the explosion to overlap into this 
         * tile.
         */
        this_explosion[x][y] = distance;
        return;
    }
#endif

    this_explosion[x][y] = distance;
    explosion_flood(level, x-1, y);
    explosion_flood(level, x+1, y);
    explosion_flood(level, x, y-1);
    explosion_flood(level, x, y+1);
}

#define DEBUG_EXPLOSION
#ifdef DEBUG_EXPLOSION
static FILE *fp;

static void debug_explosion (levelp level, int ix, int iy)
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

            if ((x == ix) && (y == iy)) {
                fprintf(fp,"*");
                continue;
            }
                
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
                                    double density,
                                    uint32_t nargs, ...)
{
    va_list args;

    /*
     * However, as the player can move close to walls, the current tile might 
     * end up being a wall. If this is the case, look around for a closer tile 
     * that has no wall.
     */
    if (map_find_wall_at(level, x, y, 0) ||
        map_find_door_at(level, x, y, 0) ||
        map_find_rock_at(level, x, y, 0)) {

        double dx, dy;
        double best_x, best_y;
        double best_distance;
        int gotone = false;

        best_x = -1;
        best_y = -1;
        best_distance = 999;

        for (dx = -0.5; dx <= 0.5; dx+=0.5) {
            for (dy = -0.5; dy <= 0.5; dy+=0.5) {
                double tx, ty;

                tx = x + dx;
                ty = y + dy;

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

    /*
     * Record the start of this explosion. We will do a map flood fill to find 
     * out the extent of the detonation.
     */
    memset(this_explosion, 0, sizeof(this_explosion));
    this_explosion_x = x;
    this_explosion_y = y;
    this_explosion_radius = radius;
    explosion_flood(level, x, y);

    int ix, iy;

#ifdef DEBUG_EXPLOSION
    if (0) {
        debug_explosion(level, x, y);
    }

    if (level == server_level) {
        printf("server explosion at x %d y %d\n",(int)x,(int)y);
    } else {
        printf("client explosion at x %d y %d\n",(int)x,(int)y);
    }

    for (iy = 0; iy < MAP_HEIGHT; iy++) {
        for (ix = 0; ix < MAP_WIDTH; ix++) {
            if (((int)x == ix) && ((int)y == iy)) {
                printf("*");
                continue;
            }

            if (map_find_wall_at(level, ix, iy, 0) ||
                map_find_rock_at(level, ix, iy, 0)) {
                printf("+");
                continue;
            }

            if (this_explosion[ix][iy]) {
                printf("%u", this_explosion[ix][iy]);
            } else {
                printf(".");
            }
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

    /*
     * If 0 radius, then we want to only place the epicenter of the explosion 
     * so that it gets synced to the client. This is useful for explosions 
     * that are effects only and do not interact, so we don't need to place
     * the explosion tiles on the server to do collisions.
     */
    if (!radius) {
        return;
    }

    for (ix = x - radius - 1; ix < x + radius - 1; ix++) {
        if (ix < 1) {
            continue;
        }

        if (ix > MAP_WIDTH - 1) {
            continue;
        }

        for (iy = y - radius - 1; iy < y + radius - 1; iy++) {
                                
            if (iy < 1) {
                continue;
            }

            if (iy > MAP_HEIGHT - 1) {
                continue;
            }

            int8_t distance = this_explosion[ix][iy];
            if (!distance) {
                continue;
            }

            if (distance > radius) {
                continue;
            }

            double dx, dy;

            for (dx = -0.5; dx <= 0.5; dx += density) {
                for (dy = -0.5; dy <= 0.5; dy += density) {
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
                }
            }
        }
    }
}

static void level_place_spatter (levelp level, 
                                 thingp owner,
                                 double x, 
                                 double y,
                                 int radius,
                                 double density,
                                 uint32_t nargs, ...)
{
    va_list args;

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

    /*
     * If 0 radius, then we want to only place the epicenter of the explosion 
     * so that it gets synced to the client. This is useful for explosions 
     * that are effects only and do not interact, so we don't need to place
     * the explosion tiles on the server to do collisions.
     */
    if (!radius) {
        return;
    }

    int ix, iy;

    for (ix = x - radius - 1; ix < x + radius - 1; ix++) {
        if (ix < 1) {
            continue;
        }

        if (ix > MAP_WIDTH - 1) {
            continue;
        }

        for (iy = y - radius - 1; iy < y + radius - 1; iy++) {
                                
            if (iy < 1) {
                continue;
            }

            if (iy > MAP_HEIGHT - 1) {
                continue;
            }

            double distance = DISTANCE(x, y, ix, iy);
            if (distance > radius) {
                continue;
            }

            double dx, dy;

            for (dx = -0.5; dx <= 0.5; dx += density) {
                for (dy = -0.5; dy <= 0.5; dy += density) {
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
                           0.5, // density
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
                           0.5, // density
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
    level_place_spatter(level, 
                        owner,
                        x, y,
                        0, // radius
                        0.5, // density
                        1, // nargs
                        "data/things/hit_success");
}

void level_place_hit_miss (levelp level, 
                          thingp owner,
                          double x, double y)
{
    level_place_spatter(level, 
                        owner,
                        x, y,
                        0, // radius
                        0.5, // density
                        1, // nargs
                        "data/things/hit_miss");
}

void level_place_blood (levelp level, 
                        thingp owner,
                        double x, double y)
{
    level_place_spatter(level, 
                        owner,
                        x, y,
                        0, // radius
                        0.5, // density
                        1, // nargs
                        "data/things/blood1");
}

void level_place_blood_crit (levelp level, 
                             thingp owner,
                             double x, double y)
{
    level_place_spatter(level, 
                        owner,
                        x, y,
                        0, // radius
                        0.5, // density
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
                           0.5, // density
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
                           0.5, // density
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
                           0.5, // density
                           4, // nargs
                           "data/things/cloudkill1",
                           "data/things/cloudkill2",
                           "data/things/cloudkill3",
                           "data/things/cloudkill4");
}

void level_place_bomb (levelp level, 
                       thingp owner,
                       double x, double y)
{
    widp w = thing_place_behind(owner, 
                                id_to_tp(THING_BOMB), 0 /* itemp */);
    if (!w) {
        ERR("could not place bomb");
        return;
    }

    thingp t = wid_get_thing(w);

    /*
     * Set no owner. Bombs should kill their owner too!
     *
    thing_set_owner(t, owner);
     */

    thing_wake(t);
}

void level_place_small_cloudkill (levelp level, 
                                  thingp owner,
                                  double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           2, // radius
                           0.5, // density
                           4, // nargs
                           "data/things/cloudkill1",
                           "data/things/cloudkill2",
                           "data/things/cloudkill3",
                           "data/things/cloudkill4");
}
