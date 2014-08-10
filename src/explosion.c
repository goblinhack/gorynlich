/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include <errno.h>
#include "slre.h"

#include "main.h"
#include "math.h"
#include "string.h"
#include "wid.h"
#include "level.h"
#include "thing.h"
#include "command.h"
#include "time.h"
#include "marshal.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"
#include "wid_editor_map.h"
#include "map.h"
#include "timer.h"
#include "sound.h"
#include "wid_console.h"
#include "wid_editor.h"
#include "socket.h"
#include "level_private.h"

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

    thing_templatep thing_template = thing_template_find(name);
    if (!thing_template) {
        DIE("no explosion for name %s", name);
    }

    double delay = DISTANCE(ox, oy, x, y) * 50;

    /*
     * Make the delay on the server a lot smaller so we don't see things die 
     * after the explosion.
     */
    if (level == server_level) {
        delay *= 0.75;
    }

    thing_place_and_destroy_timed(thing_template,
                                  owner,
                                  x,
                                  y,
                                  delay,
                                  2000, // destroy in
                                  10, // jitter
                                  level == server_level ? 1 : 0,
                                  dist == 0 ? 1 : 0);

    return (true);
}

static int can_see_obstacle (levelp level, int32_t x, int32_t y)
{
    if (map_find_wall_at(level, x, y, 0) ||
        map_find_door_at(level, x, y, 0) ||
        map_find_rock_at(level, x, y, 0)) {

        return (true);
    }

    return (false);
}

static int can_see_ (levelp level,
                     int32_t x0_in, 
                     int32_t y0_in, 
                     int32_t x1_in, 
                     int32_t y1_in, 
                     int32_t flag)
{
    double temp;
    double dx;
    double dy;
    double tdy;
    double dydx;
    double p;
    double x;
    double y;
    double i;

    double x0 = x0_in;
    double y0 = y0_in;
    double x1 = x1_in;
    double y1 = y1_in;

    if (x0 > x1) {
        temp = x0;
        x0 = x1;
        x1 = temp;

        temp = y0;
        y0 = y1;
        y1 = temp;
    }

    dx = x1 - x0;
    dy = y1 - y0;

    tdy = 2.0 * dy;
    dydx = tdy - (2.0 * dx);

    p = tdy - dx;
    x = x0;
    y = y0;

    if (flag == 0) {
        if (can_see_obstacle(level, (int32_t)x, (int32_t)y)) {
            return (true);
        }
    } else if (flag == 1) {
        if (can_see_obstacle(level, (int32_t)y, (int32_t)x)) {
            return (true);
        }
    } else if (flag == 2) {
        if (can_see_obstacle(level, (int32_t)y, (int32_t)-x)) {
            return (true);
        }
    } else if (flag == 3) {
        if (can_see_obstacle(level, (int32_t)x, (int32_t)-y)) {
            return (true);
        }
    }

    for (i = 1; i <= dx; i++){
        x++;

        if (p < 0) {
            p += tdy;
        } else {
            p += dydx;
            y++;
        }

        if (flag == 0) {
            if (can_see_obstacle(level, (int32_t)x, (int32_t)y)) {
                return (true);
            }
        } else if (flag == 1) {
            if (can_see_obstacle(level, (int32_t)y, (int32_t)x)) {
                return (true);
            }
        } else if (flag == 2) {
            if (can_see_obstacle(level, (int32_t)y, (int32_t)-x)) {
                return (true);
            }
        } else if (flag == 3) {
            if (can_see_obstacle(level, (int32_t)x, (int32_t)-y)) {
                return (true);
            }
        }
    }

    return (false);
}

/*
 * Can A see B unimpeded?
 */
static int can_see (levelp level, int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
    double slope = 100.0;

    if (x0 != x1) {
        slope = (y1 - y0) * (1.0 / (x1 - x0));
    }

    int r;

    if ((0 <= slope) && (slope <= 1)) {
        r = can_see_(level, x0, y0, x1, y1, 0);
    } else if ((-1 <= slope) && (slope <= 0)) {
        r = can_see_(level, x0, -y0, x1, -y1, 3);
    } else if (slope > 1) {
        r = can_see_(level, y0, x0, y1, x1, 1);
    } else {
        r = can_see_(level, -y0, x0, -y1, x1, 2);
    }

    return (!r);
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

    if (!can_see(level, x, y, this_explosion_x, this_explosion_y)) {
        this_explosion[x][y] = (uint8_t)-1;
        return;
    }

    uint8_t distance = DISTANCE(x, y, this_explosion_x, this_explosion_y);

    if (distance > this_explosion_radius) {
        return;
    }

    if (map_find_wall_at(level, x, y, 0) ||
        map_find_door_at(level, x, y, 0) ||
        map_find_rock_at(level, x, y, 0)) {
        this_explosion[x][y] = (uint8_t)-1;
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
                                    uint32_t radius,
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

            double density = 1.0;
            double dx, dy;

            for (dx = -0.5; dx < 0.5; dx += density) {
                for (dy = -0.5; dy < 0.5; dy += density) {
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

void level_place_sparks (levelp level, 
                         thingp owner,
                         double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           1, // radius
                           3, // nargs
                           "data/things/sparks1",
                           "data/things/sparks2",
                           "data/things/sparks3");
}

void level_place_fireball (levelp level, 
                           thingp owner,
                           double x, double y)
{
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
