/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_intro.h"
#include "wid_popup.h"
#include "wid_dead.h"
#include "thing.h"

static widp wid_dead;
static widp wid_dead_credits;
static uint8_t wid_dead_init_done;
static void wid_dead_create(void);
static void wid_dead_destroy(void);
static void wid_dead_finished(widp wid);

uint8_t wid_dead_init (void)
{
    if (!wid_dead_init_done) {
    }

    wid_dead_init_done = true;

    return (true);
}

void wid_dead_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_dead_init_done) {
        wid_dead_init_done = false;

        wid_dead_destroy();
    }
}

void wid_dead_hide (void)
{
    wid_dead_destroy();
}

void wid_dead_visible (void)
{
    wid_dead_create();
}

static void wid_dead_destroy (void)
{
    wid_destroy(&wid_dead);
    wid_destroy(&wid_dead_credits);
}

static void wid_dead_finished (widp wid)
{
    if (sdl_is_exiting()) {
        return;
    }

    wid_dead_hide();

    wid_intro_visible();
}

static void wid_dead_create (void)
{
    if (sdl_is_exiting()) {
        return;
    }

    widp w = wid_new_window("dead");
    fpoint tl = { 0, 0 };
    fpoint br = { 1, 1 };

    wid_set_tl_br_pct(w, tl, br);

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, RED);
    wid_set_color(w, WID_COLOR_BR, WHITE);
    wid_set_color(w, WID_COLOR_BG, WHITE);
    wid_set_tex(w, 0, "gravestone");

    wid_destroy_in(w, 110000);
    wid_set_on_destroy(w, wid_dead_finished);
    wid_move_to_pct_centered(w, 0.5f, 0.5f);

    wid_dead_credits = wid_popup(
          "A piece of cheese!"
          ,
          0,
          0.5, 0.5,                 /* x,y postition in percent */
          large_font,               /* title font */
          large_font,               /* body font */
          med_font,                 /* button font */
          0);

    wid_move_to_pct_centered(wid_dead_credits, 0.5, 2.5);
    wid_move_to_pct_centered_in(wid_dead_credits, 0.5, 0.9, 2000);

    color c = WHITE;
    c.a = 0;
    wid_set_color(wid_dead_credits, WID_COLOR_BG, c);

    c = WHITE;
    c.a = 30;
    wid_set_color(wid_dead_credits, WID_COLOR_TL, c);
    wid_set_color(wid_dead_credits, WID_COLOR_BR, c);
    wid_set_color(wid_dead_credits, WID_COLOR_TEXT, GREEN);

    wid_raise(w);
    wid_raise(wid_dead_credits);

    wid_update(w);
}
