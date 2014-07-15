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
                                         double i,
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
        DIE("no explosion");
    }

    double delay = DISTANCE(ox, oy, x, y);

    thing_place_and_destroy_timed(thing_template,
                                  owner,
                                  x,
                                  y,
                                  delay * 100,
                                  500,     // destroy in
                                  10);     // jitter

    return (true);
}

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

    dmap_generate_player_map(x, y);

    uint32_t ix, iy;

    for (ix = 1; ix < MAP_WIDTH - 1; ix++) {
        for (iy = 1; iy < MAP_HEIGHT - 1; iy++) {
            int8_t dist = dmap_player_map_treat_doors_as_walls.walls[ix][iy];

            if (dist > radius) {
                continue;
            }

            double density = 0.5;
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
                                                    dist,
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
                           7, // nargs
                           "data/things/explosion1",
                           "data/things/explosion2",
                           "data/things/explosion3",
                           "data/things/explosion4",
                           "data/things/explosion5",
                           "data/things/explosion6",
                           "data/things/explosion7");
}

void level_place_small_explosion (levelp level, 
                                  thingp owner,
                                  double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           3, // radius
                           7, // nargs
                           "data/things/explosion1",
                           "data/things/explosion2",
                           "data/things/explosion3",
                           "data/things/explosion4",
                           "data/things/explosion5",
                           "data/things/explosion6",
                           "data/things/explosion7");
}

void level_place_potion_effect_fireball (levelp level, 
                                         thingp owner,
                                         double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           7, // radius
                           7, // nargs
                           "data/things/explosion1",
                           "data/things/explosion2",
                           "data/things/explosion3",
                           "data/things/explosion4",
                           "data/things/explosion5",
                           "data/things/explosion6",
                           "data/things/explosion7");
}

void level_place_potion_effect_poison (levelp level, 
                                       thingp owner,
                                       double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           9, // radius
                           7, // nargs
                           "data/things/poison1",
                           "data/things/poison2",
                           "data/things/poison3",
                           "data/things/poison4",
                           "data/things/poison5",
                           "data/things/poison6",
                           "data/things/poison7");
}

void level_place_potion_effect_cloudkill (levelp level, 
                                       thingp owner,
                                       double x, double y)
{
    level_place_explosion_(level, 
                           owner,
                           x, y,
                           12, // radius
                           2, // nargs
                           "data/things/cloudkill1",
                           "data/things/cloudkill2",
                           "data/things/cloudkill3",
                           "data/things/cloudkill4",
                           "data/things/cloudkill5",
                           "data/things/cloudkill6",
                           "data/things/cloudkill7");
}
