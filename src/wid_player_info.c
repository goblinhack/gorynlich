/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_player_info.h"
#include "string.h"

static widp wid_player_info;
static widp wid_player_info_container;
static uint8_t wid_player_info_init_done;

static void wid_player_info_create(player_stats_t *);
static void wid_player_info_destroy(void);
static player_stats_t *player_stats;

uint8_t wid_player_info_init (void)
{
    if (!wid_player_info_init_done) {
    }

    wid_player_info_init_done = true;

    return (true);
}

void wid_player_info_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_player_info_init_done) {
        wid_player_info_init_done = false;

        wid_player_info_destroy();
    }
}

void wid_player_info_hide (void)
{
    wid_player_info_destroy();
}

void wid_player_info_visible (player_stats_t *s)
{
    wid_player_info_create(s);
}

static void wid_player_info_create (player_stats_t *s)
{
    player_stats = s;

    if (!wid_player_info) {
        widp w = wid_player_info = wid_new_rounded_window("wid player_stats");

        fpoint tl = {0.0, 0.0};
        fpoint br = {0.3, 0.9};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_font(w, small_font);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);

        wid_set_tex(wid_player_info, 0, "gothic_tall_skull");
        wid_set_square(wid_player_info);
    }

    {
        widp w = wid_player_info_container =
            wid_new_container(wid_player_info, "wid player_stats container");

        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(w, tl, br);
    }

    wid_move_to_pct_centered(wid_player_info, 0.5, -1.0);
    wid_move_to_pct_centered_in(wid_player_info, 0.5, 0.45, wid_swipe_delay);

    wid_raise(wid_player_info);
    wid_update(wid_player_info);
}

static void wid_player_info_destroy (void)
{
    wid_destroy(&wid_player_info);
}
