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
#include "glapi.h"

static float x;

int sdl_intro_demo_update (void)
{
    static widp wid_icon;

    color col = STEELBLUE;
    glcolor(col);

    if (!wid_icon) {
        wid_icon = wid_new_window("splash icon");

        fpoint tl = { 0.3, 0.15 };
        fpoint br = { 0.7, 0.7 };

        wid_set_tl_br_pct(wid_icon, tl, br);

        wid_set_tex(wid_icon, 0, "giant_rock");

        wid_raise(wid_icon);
        wid_set_do_not_raise(wid_icon, true);

        wid_set_mode(wid_icon, WID_MODE_NORMAL);
        color c = WHITE;

        wid_set_color(wid_icon, WID_COLOR_TL, c);
        wid_set_color(wid_icon, WID_COLOR_BR, c);
        wid_set_color(wid_icon, WID_COLOR_BG, c);

        wid_raise(wid_icon);
        wid_update(wid_icon);
    }

    wid_move_to_pct(wid_icon, x, 0.8f);
    x += 0.01;

    if (x > 1.0) {
        wid_destroy_nodelay(&wid_icon);
        return (false);
    }

    return (true);
}
