/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "math_util.h"
#include "thing.h"

static void level_place_spatter (levelp level, 
                                 thingp owner,
                                 double x, 
                                 double y,
                                 double radius,
                                 int amount,
                                 uint32_t nargs, ...)
{
    va_list args;

    while (amount--) {
        double px = gauss(x, radius);
        double py = gauss(y, radius);
        va_start(args, nargs);

        (void) level_place_explosion_at(level, 
                                        owner,
                                        px,
                                        py,
                                        px, 
                                        py, 
                                        0,
                                        true, /* epicenter */
                                        0,
                                        nargs, args);
        va_end(args);
    }
}

void level_place_hit_success (levelp level, 
                              thingp owner,
                              double x, double y)
{
    level_place_spatter(level, 
                        owner,
                        x, y,
                        0.0, // radius
                        1, // amount
                        1, // nargs
                        "hit_success");
}

void level_place_hit_miss (levelp level, 
                          thingp owner,
                          double x, double y)
{
    level_place_spatter(level, 
                        owner,
                        x, y,
                        0.0, // radius
                        1, // amount
                        1, // nargs
                        "hit_miss");
}

void level_place_blood (levelp level, 
                        thingp owner,
                        double x, double y)
{
    level_place_spatter(level, 
                        owner,
                        x, y,
                        0.05, // radius
                        1, // amount
                        1, // nargs
                        "blood1");
}

void level_place_flames (levelp level, 
                        thingp owner,
                        double x, double y)
{
    level_place_spatter(level, 
                        owner,
                        x, y,
                        0.05, // radius
                        1, // amount
                        1, // nargs
                        "flames1");
}

void level_place_bubbles (levelp level, 
                        thingp owner,
                        double x, double y)
{
    level_place_spatter(level, 
                        owner,
                        x, y,
                        0.05, // radius
                        1, // amount
                        1, // nargs
                        "bubbles1");
}

void level_place_blood_crit (levelp level, 
                             thingp owner,
                             double x, double y)
{
    level_place_spatter(level, 
                        owner,
                        x, y,
                        0.1, // radius
                        5, // amount
                        1, // nargs
                        "blood2");
}
