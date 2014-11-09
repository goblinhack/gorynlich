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
#include "wid_player_stats.h"
#include "thing_template.h"
#include "string.h"

static widp wid_player_action;
static uint8_t wid_player_action_init_done;

static void wid_player_action_create(thing_statsp , int fast);
static void wid_player_action_destroy(void);
static thing_statsp player_stats;

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

void wid_player_action_visible (thing_statsp s, int fast)
{
    wid_player_action_create(s, fast);
}

static uint8_t 
wid_player_action_button_style_mouse_down (widp w,
                                           int32_t x, int32_t y,
                                           uint32_t button)
{
    uint32_t id = (typeof(id)) (uintptr_t) wid_get_client_context(w);
    itemp over_item = &player_stats->action_bar[id];

    if (!wid_mouse_template) {
        wid_player_item_pick_up(w, over_item);
    } else {
        /*
         * Drop the current item.
         */
        int dropped = false;

        if (item_push(over_item, wid_item)) {
            /*
             * Success
             */
            dropped = true;
        }

        if (!dropped) {
            /*
             * If we failed to drop, can we move the item that we are over
             * somewhere else in the action bat?
             */
            int i;

            for (i = 0; i < THING_ACTION_BAR_MAX; i++) {
                itemp freeitem = &player_stats->action_bar[i];
                if (!freeitem->id) {
                    memcpy(freeitem, over_item, sizeof(item_t));
                    memset(over_item, 0, sizeof(item_t));

                    /*
                     * Try again
                     */
                    if (item_push(over_item, wid_item)) {
                        /*
                         * Success
                         */
                        dropped = true;
                    }
                    break;
                }
            }
        }

        if (!dropped) {
            /*
             * Still not dropped into the action bar. Can we move the 
             * conflicting item to the inventory?
             */
            int i;

            for (i = 0; i < THING_INVENTORY_MAX; i++) {
                itemp freeitem = &player_stats->inventory[i];
                if (!freeitem->id) {
                    memcpy(freeitem, over_item, sizeof(item_t));
                    memset(over_item, 0, sizeof(item_t));

                    /*
                     * Try again
                     */
                    if (item_push(over_item, wid_item)) {
                        /*
                         * Success
                         */
                        dropped = true;
                    }
                    break;
                }
            }
        }

        if (!dropped) {
            /*
             * Can we add this anywhere at all ?
             */
            if (thing_stats_item_add(0, player_stats, wid_item)) {
                dropped = true;
            }
        }

        if (dropped) {
            memset(&wid_item, 0, sizeof(item_t));

            wid_destroy(&wid_mouse_template);

            wid_set_client_context(w, 0);
        }
    }

    wid_player_stats_redraw();

    return (true);
}

static void wid_player_action_create (thing_statsp s, int fast)
{
    if (wid_player_action) {
        return;
    }

    player_stats = s;

    if (!wid_player_action) {
        widp w = wid_player_action = 
                        wid_new_square_window("wid player_stats");

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

        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);
        wid_set_no_shape(w);

        int which = (int)(((double)s->hp / (double)s->max_hp) * 16) + 1;
        if (which > 16) {
            which = 16;
        }
        if (which < 1) {
            which = 1;
        }

        char tmp[40];
        snprintf(tmp, sizeof(tmp)-1, "crystalball.%d", which);
        wid_set_tilename(w, tmp);
    }

    {
        widp w =
            wid_new_container(wid_player_action, 
                              "wid player_stats container");

        fpoint tl = {0.7, 0.2};
        fpoint br = {0.8, 1.0};

        wid_set_tl_br_pct(w, tl, br);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);
        wid_set_no_shape(w);

        int which = (int)(((double)s->id / (double)s->max_id) * 16) + 1;
        if (which > 16) {
            which = 16;
        }
        if (which < 1) {
            which = 1;
        }

        char tmp[40];
        snprintf(tmp, sizeof(tmp)-1, "crystalball_purple.%d", which);
        wid_set_tilename(w, tmp);
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
        wid_set_text_outline(w, true);
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
        wid_set_text_outline(w, true);
        wid_set_font(w, small_font);
    }

    if (player) {
        widp w =
            wid_new_container(wid_player_action, 
                              "wid player_stats container");

        fpoint tl = {0.55, 0.2};
        fpoint br = {0.65, 1.0};

        wid_set_tl_br_pct(w, tl, br);

        wid_set_tex(w, 0, "squiggles");
        wid_raise(w);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);
        wid_set_square(w);
        wid_set_text_bot(w, true);
        wid_set_text_outline(w, true);
        wid_set_font(w, small_font);

        char tmp[40];
        snprintf(tmp, sizeof(tmp) - 1, "%d", s->id);
        wid_set_text(w, tmp);
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
        double border_top = 0.01;
        double border_bottom = 0.25;
        double dy = (1.0 - (border_top + border_bottom)) / max_down;

        int i = 0;
        
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

            item_t item = s->action_bar[i];

            wid_player_inventory_button_style(w, s, item);

            wid_set_on_mouse_down(w, 
                                  wid_player_action_button_style_mouse_down);

            wid_set_client_context(w, (void*) (uintptr_t) i);

            i++;
        }
    }

    if (fast) {
        wid_move_to_pct_centered(wid_player_action, 0.5, 0.905);
    } else {
        wid_move_to_pct_centered(wid_player_action, 0.5, 2.0);
        wid_move_to_pct_centered_in(wid_player_action, 0.5, 0.905,
                                    5*wid_swipe_delay);
    }

    wid_raise(wid_player_action);
    wid_update(wid_player_action);

    wid_raise(wid_player_action);

    wid_set_movable(wid_player_action, false);
}

static void wid_player_action_destroy (void)
{
    wid_destroy(&wid_player_action);
}
