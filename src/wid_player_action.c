/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_player_action.h"
#include "wid_player_inventory.h"
#include "thing_template.h"
#include "string.h"

static widp wid_player_action;
static uint8_t wid_player_action_init_done;

static void wid_player_action_create(player_stats_t *);
static void wid_player_action_destroy(void);
static player_stats_t *player_stats;

uint8_t wid_player_action_init (void)
{
    if (!wid_player_action_init_done) {
    }

    wid_player_action_init_done = true;

    return (true);
}

void wid_player_action_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_player_action_init_done) {
        wid_player_action_init_done = false;

        wid_player_action_destroy();
    }
}

void wid_player_action_hide (void)
{
    wid_player_action_destroy();
}

void wid_player_action_visible (player_stats_t *s)
{
    wid_player_action_create(s);
}

static void wid_player_action_create (player_stats_t *s)
{
    player_stats = s;

    if (!wid_player_action) {
        widp w = wid_player_action = 
                        wid_new_rounded_window("wid player_stats");

        fpoint tl = {0.0, 0.8};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_font(w, small_font);

        wid_set_no_shape(w);
    }

    {
        widp w =
            wid_new_container(wid_player_action, 
                              "wid player_stats container");

        fpoint tl = {0.2, 0.2};
        fpoint br = {0.3, 1.0};

        wid_set_tl_br_pct(w, tl, br);

        wid_set_tilename(w, "crystalball.1");
        wid_set_no_shape(w);
    }

    {
        widp w =
            wid_new_container(wid_player_action, 
                              "wid player_stats container");

        fpoint tl = {0.7, 0.2};
        fpoint br = {0.8, 1.0};

        wid_set_tl_br_pct(w, tl, br);

        wid_set_tilename(w, "crystalball_purple.1");
        wid_set_no_shape(w);
    }

    {
        widp w =
            wid_new_container(wid_player_action, 
                              "wid player_stats container");

        fpoint tl = {0.15, 0.2};
        fpoint br = {0.35, 1.0};

        wid_set_tl_br_pct(w, tl, br);

        wid_set_tex(w, 0, "squiggles");
        wid_raise(w);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);
        wid_set_square(w);
        wid_set_text(w, "Health");
        wid_set_text_bot(w, true);
        wid_set_font(w, small_font);
    }

    {
        widp w =
            wid_new_container(wid_player_action, 
                              "wid player_stats container");

        fpoint tl = {0.65, 0.2};
        fpoint br = {0.85, 1.0};

        wid_set_tl_br_pct(w, tl, br);

        wid_set_tex(w, 0, "squiggles");
        wid_raise(w);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);
        wid_set_square(w);
        wid_set_text(w, "ID");
        wid_set_text_bot(w, true);
        wid_set_font(w, small_font);
    }

    widp wid_item_bar;

    {
        widp w = wid_item_bar = 
            wid_new_container(wid_player_action, 
                              "wid player_stats container");

        fpoint tl = {0.3, 0.5};
        fpoint br = {0.8, 1.0};

        wid_set_tl_br_pct(w, tl, br);

        wid_set_tex(w, 0, "itembar");
        wid_raise(w);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);
        wid_set_square(w);
    }

    {
        double x;
        double max_across = 10.0;
        double border_left = 0.06;
        double border_right = 0.25;
        double dx = (1.0 - (border_left + border_right)) / max_across;

        double y;
        double max_down = 1.0;
        double border_top = 0.06;
        double border_bottom = 0.25;
        double dy = (1.0 - (border_top + border_bottom)) / max_down;

        int item = 0;
        
        y = 0;
        for (x = 0; x < max_across; x++) 
        {
            widp w = wid_new_square_button(wid_item_bar, 
                                        "wid player_stats inventory item");
            fpoint tl = {0.0, 0.0};
            fpoint br = {0.0, 0.8};

            tl.x = border_left + (x * dx);
            tl.y = border_top + (y * dy);

            br.x = tl.x + dx;
            br.y = tl.y + dy;

            wid_set_tl_br_pct(w, tl, br);

            int id = s->action_bar[item];

            wid_player_inventory_button_style(w, s, id);
            wid_set_rounded_small(w);

    //        wid_set_on_mouse_down(w, wid_intro_settings_col4_mouse_event);
            item++;
        }
    }

    wid_move_to_pct_centered(wid_player_action, 0.5, 2.0);
    wid_move_to_pct_centered_in(wid_player_action, 0.5, 0.905, 
                                5*wid_swipe_delay);

    wid_raise(wid_player_action);
    wid_update(wid_player_action);

    wid_raise(wid_player_action);
    wid_set_do_not_lower(wid_player_action, true);
}

static void wid_player_action_destroy (void)
{
    wid_destroy(&wid_player_action);
}
