/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

// REMOVED #include <SDL.h>
// REMOVED #include "slre.h"

// REMOVED #include "main.h"
#include "math_util.h"
// REMOVED #include "level.h"
#include "thing.h"
// REMOVED #include "wid_game_map_server.h"
// REMOVED #include "map.h"
// REMOVED #include "vision.h"
// REMOVED #include "wid.h"

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
                                    true, /* epicenter */
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

    for (ix = x - radius - 1; ix <= x + radius + 1; ix++) {
        if (ix < 1) {
            continue;
        }

        if (ix > MAP_WIDTH - 1) {
            continue;
        }

        for (iy = y - radius - 1; iy <= y + radius + 1; iy++) {
                                
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
                                                    false, /* epicenter */
                                                    nargs, args);
                    va_end(args);
                }
            }
        }
    }
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
                        1, // radius
                        0.5, // density
                        1, // nargs
                        "data/things/blood2");
}
