/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_intro.h"
#include "wid_popup.h"
#include "wid_game_over.h"
#include "music.h"

static widp wid_game_over;
static widp wid_game_over_credits;
static uint8_t wid_game_over_init_done;
static void wid_game_over_create(void);
static void wid_game_over_destroy(void);
static void wid_game_over_finished(widp wid);

uint8_t wid_game_over_init (void)
{
    if (!wid_game_over_init_done) {
    }

    wid_game_over_init_done = true;

    return (true);
}

void wid_game_over_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_game_over_init_done) {
        wid_game_over_init_done = false;

        wid_game_over_destroy();
    }
}

void wid_game_over_hide (void)
{
    wid_game_over_destroy();
}

void wid_game_over_visible (void)
{
    wid_game_over_create();
}

static void wid_game_over_destroy (void)
{
    wid_destroy(&wid_game_over);
    wid_destroy(&wid_game_over_credits);
}

static void wid_game_over_finished (widp wid)
{
    if (sdl_is_exiting()) {
        return;
    }

    wid_game_over_hide();

    wid_intro_visible();
}

static void wid_game_over_create (void)
{
    if (sdl_is_exiting()) {
        return;
    }

    widp w = wid_new_window("game_over");
    fpoint tl = { 0, 0 };
    fpoint br = { 1, 1 };

    wid_set_tl_br_pct(w, tl, br);

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, BLACK);
    wid_set_color(w, WID_COLOR_BR, BLACK);
    wid_set_color(w, WID_COLOR_BG, BLACK);

    wid_destroy_in(w, 110000);
    wid_set_on_destroy(w, wid_game_over_finished);
    wid_move_to_pct_centered(w, 0.5f, 0.5f);

    music_play_game_over();

    wid_game_over_credits = wid_popup(
          "",
          "Game Over!\n"
          "\n"
          "\n"
          "Congratulations!\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "This game was written badly\n"
          "by Neil McGill\n"
          "\n"
          "Hope you liked it.\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "Bye!\n"
          ,
          0.5, 0.5,                 /* x,y postition in percent */
          large_font,               /* title font */
          large_font,               /* body font */
          med_font,                 /* button font */
          0);

    wid_move_to_pct_centered(wid_game_over_credits, 0.5, 2.5);
    wid_move_to_pct_centered_in(wid_game_over_credits, 0.5, 0.5, 2000);

    wid_raise(w);
    wid_raise(wid_game_over_credits);

    wid_update(w);
}
