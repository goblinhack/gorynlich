/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_player_inventory.h"
#include "string.h"
#include "thing_template.h"

static widp wid_player_inventory;
static widp wid_player_inventory_container;
static uint8_t wid_player_inventory_init_done;

static void wid_player_inventory_create(player_stats_t *);
static void wid_player_inventory_destroy(void);
static player_stats_t *player_stats;

uint8_t wid_player_inventory_init (void)
{
    if (!wid_player_inventory_init_done) {
    }

    wid_player_inventory_init_done = true;

    return (true);
}

void wid_player_inventory_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_player_inventory_init_done) {
        wid_player_inventory_init_done = false;

        wid_player_inventory_destroy();
    }
}

void wid_player_inventory_hide (void)
{
    wid_player_inventory_destroy();
}

void wid_player_inventory_visible (player_stats_t *s)
{
    wid_player_inventory_create(s);
}

void wid_player_inventory_button_style (widp w)
{
    color c;

    wid_set_mode(w, WID_MODE_NORMAL);

    c = BLACK;
    c.a = 0;
    wid_set_color(w, WID_COLOR_BG, c);

    c = STEELBLUE;
    c.a = 30;
    wid_set_color(w, WID_COLOR_TL, c);

    c = GRAY;
    c.a = 30;
    wid_set_color(w, WID_COLOR_BR, c);

    wid_set_mode(w, WID_MODE_OVER);

    c = WHITE;
    c.a = 50;
    wid_set_color(w, WID_COLOR_BG, c);

    c = STEELBLUE;
    c.a = 30;
    wid_set_color(w, WID_COLOR_TL, c);

    c = GRAY;
    c.a = 30;
    wid_set_color(w, WID_COLOR_BR, c);

    wid_set_mode(w, WID_MODE_NORMAL);
}

static void wid_player_inventory_create (player_stats_t *s)
{
    player_stats = s;

    if (!wid_player_inventory) {
        widp w = wid_player_inventory = 
                        wid_new_rounded_window("wid player_stats");

        fpoint tl = {0.0, 0.0};
        fpoint br = {0.3, 0.9};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_font(w, small_font);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);

        wid_set_tex(wid_player_inventory, 0, "gothic_tall_chest");
        wid_set_square(wid_player_inventory);
    }

    {
        widp w = wid_player_inventory_container =
            wid_new_container(wid_player_inventory, 
                              "wid player_stats container");

        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(w, tl, br);
    }

    double x;
    double max_across = 10.0;
    double border_left = 0.05;
    double border_right = 0.05;
    double dx = (1.0 - (border_left + border_right)) / max_across;

    double y;
    double max_down = 10.0;
    double border_top = 0.15;
    double border_bottom = 0.15;
    double dy = (1.0 - (border_top + border_bottom)) / max_down;
    
    int item = 0;
    for (x = 0; x < max_across; x++) 
    for (y = 0; y < max_down; y++) 
    {
        widp w = wid_new_square_button(wid_player_inventory_container, 
                                       "wid player_stats inventory item");
        fpoint tl = {0.0, 0.0};
        fpoint br = {0.0, 0.8};

        tl.x = border_left + (x * dx);
        tl.y = border_top + (y * dy);

        br.x = tl.x + dx;
        br.y = tl.y + dy;

        wid_set_tl_br_pct(w, tl, br);

        wid_player_inventory_button_style(w);

//        wid_set_on_mouse_down(w, wid_intro_settings_col4_mouse_event);
//
        int id = s->inventory[item];
        if (id) {
            thing_templatep t = id_to_thing_template(id);
            wid_set_thing_template(w, t);
        }

        item++;
    }

    wid_move_to_pct_centered(wid_player_inventory, 0.8, 0.45);

    wid_raise(wid_player_inventory);
    wid_update(wid_player_inventory);
}

static void wid_player_inventory_destroy (void)
{
    wid_destroy(&wid_player_inventory);
}
