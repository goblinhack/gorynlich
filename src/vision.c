/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>
#include <errno.h>
#include "slre.h"

#include "main.h"
#include "math_util.h"
#include "string_util.h"
#include "wid.h"
#include "level.h"
#include "vision.h"
#include "map.h"
#include "thing_template.h"
#include "wid_game_map_server.h"
#include "thing.h"

static int can_see_obstacle (levelp level, int32_t x, int32_t y)
{
    if (map_find_wall_at(level, x, y, 0) ||
        map_find_door_at(level, x, y, 0) ||
        map_find_crystal_at(level, x, y, 0) ||
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
int can_see (levelp level, int32_t x0, int32_t y0, int32_t x1, int32_t y1)
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
