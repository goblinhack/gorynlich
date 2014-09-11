/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "tex.h"
#include "init_fn.h"
#include "wid_intro_demo.h"
#include "wid_intro.h"
#include "glapi.h"

static float x = -1.0;
static double y = 0.1;
static double dy = 0.0005;

static void wid_rock_tick (void)
{
    y += dy;
    x += 0.009;

    static const double wall_start = 0.30;
    static const double accell_down = 1.11;
    static const double friction_up = 0.90;
    static const double elasticity = 0.6;

    if (y > wall_start) {
        y = wall_start;
        dy = -dy * elasticity;
        y += dy;
    }

    if (dy < 0) {
        dy *= friction_up;

        if (dy > -0.0001) {
            dy = 0.0001;
        }
    } else {
        dy *= accell_down;
    }
}

int sdl_intro_demo_update (void)
{
    static widp wid_intro;
    static widp wid_rock;
    static widp wid_wall_floor;

    if (!wid_intro) {
        wid_intro = wid_new_window("splash icon");

        fpoint tl = { -2.0, 0.0 };
        fpoint br = { 5.0, 1.0 };

        wid_set_tl_br_pct(wid_intro, tl, br);

        color c = BLACK;
        wid_set_color(wid_intro, WID_COLOR_TL, c);
        wid_set_color(wid_intro, WID_COLOR_BR, c);
        wid_set_color(wid_intro, WID_COLOR_BG, c);

        wid_raise(wid_intro);
        wid_update(wid_intro);
    }

    if (!wid_rock) {
        wid_rock = wid_new_container(wid_intro, "splash icon");

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 0.05, 0.5 };

        wid_set_tl_br_pct(wid_rock, tl, br);

        wid_set_tex(wid_rock, 0, "giant_rock");

        wid_set_mode(wid_rock, WID_MODE_NORMAL);

        color c = WHITE;
        wid_set_color(wid_rock, WID_COLOR_TL, c);
        wid_set_color(wid_rock, WID_COLOR_BR, c);
        wid_set_color(wid_rock, WID_COLOR_BG, c);

        wid_raise(wid_rock);
        wid_update(wid_rock);

        wid_rotate_to_pct_in(wid_rock,
                             0,
                             1000,
                             10000,
                             0);
    }

    if (!wid_wall_floor) {
        wid_wall_floor = wid_new_window("splash icon");

        fpoint tl = { 0.0, 0.80 };
        fpoint br = { 1.0, 1.0 };

        wid_set_tl_br_pct(wid_wall_floor, tl, br);

        wid_set_tex(wid_wall_floor, 0, "wall_floor");

        wid_set_mode(wid_wall_floor, WID_MODE_NORMAL);

        color c = WHITE;
        wid_set_color(wid_wall_floor, WID_COLOR_TL, c);
        wid_set_color(wid_wall_floor, WID_COLOR_BR, c);
        wid_set_color(wid_wall_floor, WID_COLOR_BG, c);

        wid_raise(wid_wall_floor);
        wid_update(wid_wall_floor);
    }

    wid_rock_tick();
    wid_move_to_pct(wid_rock, x, y);

    if (x > 1.0) {
        wid_destroy_nodelay(&wid_intro);
        wid_destroy_nodelay(&wid_wall_floor);
        wid_intro_init();
        return (false);
    }

    return (true);
}
