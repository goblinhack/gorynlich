/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "tex.h"
#include "init_fn.h"
#include "tree.h"
#include "wid_splash.h"

void sdl_splashscreen_update (void)
{
    static widp wid_splash;
    static widp wid_icon;
    static uint32_t maxsize;
    static uint32_t size;

    if (wid_splash) {
        wid_destroy(&wid_splash);
    }

    if (wid_icon) {
        wid_destroy(&wid_icon);
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

    fpoint tl = {0.2f, 0.5f};
    fpoint br = {0.8f, 0.52f};

    br.x = ((0.6 / (float)maxsize) * (float)(maxsize - size)) + 0.2;

    wid_set_tl_br_pct(wid_splash, tl, br);
    wid_set_text(wid_splash, "Loading...");
    wid_set_font(wid_splash, med_font);

    color col = RED;
    glcolor(col);

    wid_set_mode(wid_splash, WID_MODE_NORMAL);
    wid_set_color(wid_splash, WID_COLOR_BG, col);

    wid_raise(wid_splash);
    wid_update(wid_splash);

    texp tex = tex_find("snailly");
    if (tex) {
        wid_icon = wid_new_window("splash icon");

        tex = tex_find("snailly");
        uint32_t tw = tex_get_width(tex);
        uint32_t th = tex_get_height(tex);

        fpoint tl = { 0, 0 };
        fpoint br = { (float) tw, (float) th };

        wid_set_tl_br(wid_icon, tl, br);

        wid_set_tex(wid_icon, 0, "snailly");

        wid_raise(wid_icon);
        wid_set_do_not_raise(wid_icon, true);

        wid_move_to_pct_centered(wid_icon, 0.5f, 0.4f);

        wid_set_mode(wid_icon, WID_MODE_NORMAL);
        wid_set_color(wid_icon, WID_COLOR_TL, WHITE);
        wid_set_color(wid_icon, WID_COLOR_BR, WHITE);
        wid_set_color(wid_icon, WID_COLOR_BG, WHITE);

        wid_raise(wid_icon);
        wid_update(wid_icon);
    }
}
