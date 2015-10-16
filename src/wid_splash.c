/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "tex.h"
#include "init_fn.h"
#include "wid_splash.h"
#include "glapi.h"

void sdl_splashscreen_update (void)
{
    static widp wid_production;
    static widp wid_splash;
    static widp wid_icon;
    static uint32_t maxsize;
    static uint32_t size;

    if (wid_splash) {
        wid_destroy_nodelay(&wid_splash);
    }

    if (wid_icon) {
        wid_destroy_nodelay(&wid_icon);
    }

    if (wid_production) {
        wid_destroy_nodelay(&wid_production);
    }

    if (!init_fns) {
        return;
    }

    size = tree_root_size(init_fns);
    if (size <= 3) {
        return;
    }

    if (!maxsize) {
        maxsize = tree_root_size(init_fns);
    }

    wid_splash = wid_new_square_window("splash");

    wid_set_bevelled(wid_splash, true);
    wid_set_bevel(wid_splash, 2);

    /*
     * Progress bar.
     */
    fpoint tl = {0.2f, 0.85f};
    fpoint br = {0.8f, 0.90f};

    br.x = ((0.6 / (float)maxsize) * (float)(maxsize - size)) + 0.2;

    wid_set_tl_br_pct(wid_splash, tl, br);
    wid_set_text(wid_splash, "Glorping...");
    wid_set_font(wid_splash, med_font);

    color col = BLACK;
    glcolor(col);

    wid_set_mode(wid_splash, WID_MODE_NORMAL);
    wid_set_color(wid_splash, WID_COLOR_BG, col);

    wid_raise(wid_splash);
    wid_update(wid_splash);

    texp tex = tex_find("gorynlich");
    if (tex) {
        wid_icon = wid_new_window("splash icon");

        fpoint tl = { 0.3, 0.15 };
        fpoint br = { 0.7, 0.7 };

        wid_set_tl_br_pct(wid_icon, tl, br);

        wid_set_tex(wid_icon, 0, "gorynlich");

        wid_raise(wid_icon);
        wid_set_do_not_raise(wid_icon, true);

        wid_move_to_pct_centered(wid_icon, 0.5f, 0.5f);

        wid_set_mode(wid_icon, WID_MODE_NORMAL);
        color c = WHITE;

        wid_set_color(wid_icon, WID_COLOR_TL, c);
        wid_set_color(wid_icon, WID_COLOR_BR, c);
        wid_set_color(wid_icon, WID_COLOR_BG, c);

        wid_raise(wid_icon);
        wid_update(wid_icon);
    }

    if (!wid_production) {
        wid_production = wid_new_square_window("urk");
        wid_set_font(wid_production, med_font);
        wid_set_no_shape(wid_production);

        fpoint tl = {0.2f, 0.70f};
        fpoint br = {0.8f, 0.90f};

        wid_set_tl_br_pct(wid_production, tl, br);
        wid_set_text(wid_production, "A GoblinHack production");

        wid_set_color(wid_production, WID_COLOR_TEXT, WHITE);
        color c = WHITE;
        c.a = 200;
        wid_set_color(wid_production, WID_COLOR_TEXT, c);

        wid_set_mode(wid_production, WID_MODE_OVER);
        c.a = 200;
        wid_set_color(wid_production, WID_COLOR_TEXT, c);

        wid_set_mode(wid_production, WID_MODE_FOCUS);
        c.a = 100;
        wid_set_color(wid_production, WID_COLOR_TEXT, c);
        wid_set_text_outline(wid_production, true);

        wid_update(wid_production);
    }
}
