/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include "main.h"
#include "thing.h"
#include "math_util.h"

void thing_dir (thingp t, double *dx, double *dy)
{
    *dx = 0;
    *dy = 0;

    if (thing_is_dir_down(t)) {
        *dy = 1.0;
    }

    if (thing_is_dir_up(t)) {
        *dy = -1.0;
    }

    if (thing_is_dir_right(t)) {
        *dx = 1.0;
    }

    if (thing_is_dir_left(t)) {
        *dx = -1.0;
    }

    if (thing_is_dir_tl(t)) {
        *dx = -1.0;
        *dy = -1.0;
    }

    if (thing_is_dir_tr(t)) {
        *dx = 1.0;
        *dy = -1.0;
    }

    if (thing_is_dir_bl(t)) {
        *dx = -1.0;
        *dy = 1.0;
    }

    if (thing_is_dir_br(t)) {
        *dx = 1.0;
        *dy = 1.0;
    }
}

int thing_angle_to_dir (double dx, double dy)
{
    fpoint r;
    r.x = dx;
    r.y = -dy;

    int segment = ((int)(round(radians2angle(anglerot(r)) / 45.0))) % 8;

    switch (segment) {
        case 2:
            return (THING_DIR_UP);
        case 1:
            return (THING_DIR_TR);
        case 0:
            return (THING_DIR_RIGHT);
        case 7:
            return (THING_DIR_BR);
        case 6:
            return (THING_DIR_DOWN);
        case 5:
            return (THING_DIR_BL);
        case 4:
            return (THING_DIR_LEFT);
        case 3:
            return (THING_DIR_TL);
    }

    DIE("thing_angle_to_dir bug");
}
