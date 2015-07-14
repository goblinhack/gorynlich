/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
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
#include "wid_game_map_client.h"
#include "thing_template.h"
#include "string_util.h"
#include "socket_util.h"
#include "client.h"
#include "thing.h"
#include "time_util.h"
#include "sound.h"

static widp wid_player_action;
static uint8_t wid_player_action_init_done;

static void wid_player_action_create(thing_statsp , int fast);
static void wid_player_action_destroy(void);
static thing_statsp player_stats;
static int last_hp;
static int last_magic;

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

void wid_player_action_hide (int fast, int player_quit)
{
    wid_player_action_destroy();

    if (player_quit) {
        last_hp = 0;
        last_magic = 0;
    }
}

void wid_player_action_visible (thing_statsp s, int fast)
{
    wid_player_action_create(s, fast);
}

static uint8_t 
wid_player_action_button_mouse_down (widp w,
                                     int32_t x, int32_t y,
                                     uint32_t button)
{
    uint32_t id = (typeof(id)) (uintptr_t) wid_get_client_context(w);
    int32_t action_bar_index = (typeof(action_bar_index))
                    (uintptr_t) wid_get_client_context2(w);

    if (player &&
        !wid_player_inventory_is_visible() && 
        !wid_mouse_template) {
        /*
         * Inventory window is hidden, we've clicked on a button and
         * the mouse has no item. Use the item we've clicked on.
         */
        uint32_t id = player->stats.action_bar[action_bar_index].id;
        if (!id) {
            MSG(WARNING, "Nothing in that slot");
            return (true);
        }

        if (!client_joined_server) {
            MSG(WARNING, "Not connected to server");
            return (true);
        }

        player_action_bar_changed_at = time_get_time_ms();

        socket_tx_player_action(client_joined_server, player, 
                                PLAYER_ACTION_USE,
                                action_bar_index,
                                false /* change_selection_only */);
        return (true);
    }

    itemp over_item = &player_stats->action_bar[id];

    if (!wid_mouse_template) {
        /*
         * Inventory window is visible, we've clicked on a button and
         * the mouse has no item. Pick up the item we've clicked on.
         */
        int wield = false;

        if (player) {
            int current_action_bar_index = 
                thing_stats_get_action_bar_index(player);

            /*
             * If we just picked up the current weapon (to move it elsewhere) 
             * then stop using it.
             */
            if (action_bar_index == current_action_bar_index) {
                uint32_t current_id = player->stats.action_bar[action_bar_index].id;

                tpp weapon = id_to_tp(current_id);
                if (tp_is_weapon(weapon)) {
                    thing_unwield(player);

                    /*
                     * Need to wield the next weapon.
                     */
                    wield = true;

                    socket_tx_player_action(client_joined_server, player, 
                                            PLAYER_ACTION_STOP_USE,
                                            current_action_bar_index,
                                            false /* change_selection_only */);
                }
            }
        }

        wid_player_item_pick_up(w, over_item);

        /*
         * If we just moved a weapon into the inventory we need to auto use 
         * the next weapon we have.
         */
        if (player && wield) {
            thing_wield_next_weapon(player);

            int current_action_bar_index = 
                thing_stats_get_action_bar_index(player);

            uint32_t current_id = 
                player->stats.action_bar[current_action_bar_index].id;

            tpp weapon = id_to_tp(current_id);
            if (tp_is_weapon(weapon)) {
                socket_tx_player_action(client_joined_server, player, 
                                        PLAYER_ACTION_USE,
                                        current_action_bar_index,
                                        false /* change_selection_only */);
            }
        }
    } else {
        /*
         * Inventory window is visible, we've clicked on a button and
         * the mouse has an item. Place the item we've clicked on onto
         * the action bar.
         */
        int dropped = false;

        tpp weapon = id_to_tp(wid_item.id);

        /*
         * Weapons have quality and we can't stack them. If placing a weapon 
         * on a weapon then move the current weapon back into the inventory.
         */
        if (!dropped) {
            if (tp_is_weapon(weapon)) {
                int i;
                for (i = 0; i < THING_ACTION_BAR_MAX; i++) {
                    itemp same_item = &player_stats->action_bar[i];

                    if (same_item->id == wid_item.id) {
                        /*
                         * Move the conflicting item into the inventory to 
                         * make space.
                         */
                        int j;
                        for (j = 0; j < THING_INVENTORY_MAX; j++) {
                            itemp free_slot = &player_stats->inventory[j];
                            if (!free_slot->id) {
                                memcpy(free_slot, same_item, sizeof(item_t));
                                memset(same_item, 0, sizeof(item_t));
                                break;
                            }
                        }

                        break;
                    }
                }
            }
        }

        if (!dropped) {
            /*
             * Is this item in the bar already and can it be merged with?
             */
            int i;

            for (i = 0; i < THING_ACTION_BAR_MAX; i++) {
                itemp same_item = &player_stats->action_bar[i];
                if (same_item->id == wid_item.id) {
                    /*
                     * Try again
                     */
                    if (item_push(same_item, wid_item)) {
                        /*
                         * Success
                         */
                        dropped = true;
                        player_inventory_sort(player_stats);
                    }
                    break;
                }
            }
        }

        if (!dropped) {
            if (item_push(over_item, wid_item)) {
                /*
                 * Success
                 */
                dropped = true;
                player_inventory_sort(player_stats);
            }
        }

        if (!dropped) {
            /*
             * If we failed to drop, can we move the item that we are over
             * somewhere else in the action bar?
             */
            int i;

            for (i = 0; i < THING_ACTION_BAR_MAX; i++) {
                itemp free_slot = &player_stats->action_bar[i];
                if (!free_slot->id) {
                    memcpy(free_slot, over_item, sizeof(item_t));
                    memset(over_item, 0, sizeof(item_t));

                    /*
                     * Try again
                     */
                    if (item_push(over_item, wid_item)) {
                        /*
                         * Success
                         */
                        dropped = true;
                        player_inventory_sort(player_stats);
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
                itemp free_slot = &player_stats->inventory[i];
                if (!free_slot->id) {
                    memcpy(free_slot, over_item, sizeof(item_t));
                    memset(over_item, 0, sizeof(item_t));

                    /*
                     * Try again
                     */
                    if (item_push(over_item, wid_item)) {
                        /*
                         * Success
                         */
                        dropped = true;
                        player_inventory_sort(player_stats);
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
                player_inventory_sort(player_stats);
            }
        }

        if (dropped) {
            memset(&wid_item, 0, sizeof(item_t));

            wid_destroy(&wid_mouse_template);

            wid_set_client_context(w, 0);
        }
    }

    LOG("Client: stats manually");
    stats_bump_version(player_stats);

    /*
     * Send an update now.
     */
    thing_stats_client_modified(player_stats);


    /*
     * If we've placed a weapon and we had none, then start using this one 
     * now.
     */
    if (player && !player->weapon) {
        thing_wield_next_weapon(player);

        int current_action_bar_index = 
            thing_stats_get_action_bar_index(player);

        uint32_t current_id = 
            player->stats.action_bar[current_action_bar_index].id;

        tpp weapon = id_to_tp(current_id);
        if (tp_is_weapon(weapon)) {
            socket_tx_player_action(client_joined_server, player, 
                                    PLAYER_ACTION_USE,
                                    current_action_bar_index,
                                    false /* change_selection_only */);
        }
    }

    wid_player_stats_redraw(true /* fast */);

    return (true);
}

static uint8_t 
wid_player_action_button_joy_down (widp w,
                                   int32_t x, int32_t y)
{
    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
        return (wid_player_action_button_mouse_down(w, x, y, 1));
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_B]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_X]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_Y]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_LEFT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_RIGHT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_STICK_DOWN]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_STICK_DOWN]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_START]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_XBOX]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_BACK]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_UP]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_DOWN]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_FIRE]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_FIRE]) {
    }

    return (false);
}

static void wid_player_action_tooltip (widp w, widp tooltip)
{
    double px, py;

    wid_get_pct(w, &px, &py);

    wid_move_end(tooltip);
    wid_move_to_pct_centered(tooltip, px + 0.1, py - 0.2);

    color c = CYAN;
    c.a = 50;
    wid_set_color(tooltip, WID_COLOR_BG, c);
}

static void wid_player_action_create (thing_statsp s, int fast)
{
    double left_ball_offset = -0.07;
    double right_ball_offset = 0.045;

    if (wid_player_action) {
        return;
    }

    player_stats = s;

    if (!wid_player_action) {
        widp w = wid_player_action = 
                        wid_new_square_window("wid player action");

        fpoint tl = {0.0, 0.82};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_font(w, med_font);

        wid_set_no_shape(w);
    }

    widp left_ball;
    {
        widp w = left_ball = 
            wid_new_container(wid_player_action, 
                              "wid player_stats container");

        fpoint tl = {0.2 + left_ball_offset, 0.2};
        fpoint br = {0.3 + left_ball_offset, 1.0};

        wid_set_tl_br_pct(w, tl, br);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);
        wid_set_no_shape(w);

        int which = (int)(((double)stats_get_hp(s) / (double)s->max_hp) * 16) + 1;
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

        fpoint tl = {0.7 + right_ball_offset, 0.2};
        fpoint br = {0.8 + right_ball_offset, 1.0};

        wid_set_tl_br_pct(w, tl, br);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);
        wid_set_no_shape(w);

        int which = (int)(((double)s->magic / (double)s->max_magic) * 16) + 1;
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

    widp left_ball_squiggles;
    {
        widp w = left_ball_squiggles = 
            wid_new_container(wid_player_action, 
                              "wid player_stats container");

        fpoint tl = {0.15 + left_ball_offset, 0.2};
        fpoint br = {0.35 + left_ball_offset, 1.0};

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
        wid_set_font(w, med_font);
    }

    {
        widp w =
            wid_new_container(wid_player_action, 
                              "wid player_stats container");

        fpoint tl = {0.65 + right_ball_offset, 0.2};
        fpoint br = {0.85 + right_ball_offset, 1.0};

        wid_set_tl_br_pct(w, tl, br);

        wid_set_tex(w, 0, "squiggles");
        wid_raise(w);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);
        wid_set_square(w);
        wid_set_text(w, "Magic");
        wid_set_text_bot(w, true);
        wid_set_text_outline(w, true);
        wid_set_font(w, med_font);
    }

    {
        widp w =
            wid_new_square_button(wid_player_action,
                                  "wid player_stats container");

        fpoint tl = {0.65 + right_ball_offset, 0.2};
        fpoint br = {0.85 + right_ball_offset, 0.88};

        wid_set_tl_br_pct(w, tl, br);

        wid_raise(w);

        wid_set_mode(w, WID_MODE_NORMAL);
        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);
        wid_set_text_outline(w, true);
        wid_set_font(w, large_font);
        wid_set_no_shape(w);

        char tmp[40];

        snprintf(tmp, sizeof(tmp) - 1, "%d", s->magic);
        wid_set_text(w, tmp);

        if (last_magic != s->magic) {
            if (last_magic > s->magic) {
                if (stats_get_magic(s) >= stats_get_max_magic(s)) {
                    /*
                     * Don't flash as this is just a magical effect winding 
                     * down.
                     */
                } else {
                    /*
                     * Warn the user about the decrease.
                     */
                    if (stats_get_magic(s) > 0) {
                        wid_effect_pulses(w);
                    }

                    wid_set_mode(w, WID_MODE_ACTIVE);
                    wid_set_color(w, WID_COLOR_TEXT, LIGHT_GREEN);
                }
            }
        }

        last_magic = s->magic;

        if (s->magic < s->max_magic / 10) {
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_TEXT, LIME);
        }
    }

    if (player && player->shield) {
        widp w =
            wid_new_square_button(wid_player_action,
                                  "wid player_stats container");

        fpoint tl = {0.85, 0.1};
        fpoint br = {0.95, 0.9};

        wid_set_tl_br_pct(w, tl, br);

        wid_raise(w);

        wid_set_mode(w, WID_MODE_NORMAL);
        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);
        wid_set_text_outline(w, true);
        wid_set_font(w, large_font);
        wid_set_no_shape(w);

        char tmp[40];

        snprintf(tmp, sizeof(tmp) - 1, "%d", s->shield);
        wid_set_text(w, tmp);
        wid_set_thing_template(w, player->shield);
    }

    widp left_ball_stats;

    {
        widp w = left_ball_stats = 
            wid_new_square_button(wid_player_action, 
                              "wid player_stats container");

        fpoint tl = {0.15 + left_ball_offset, 0.2};
        fpoint br = {0.35 + left_ball_offset, 0.88};

        wid_set_tl_br_pct(w, tl, br);

        wid_raise(w);

        wid_set_mode(w, WID_MODE_NORMAL);
        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);
        wid_set_text_outline(w, true);
        wid_set_font(w, large_font);
        wid_set_no_shape(w);

        char tmp[40];

        snprintf(tmp, sizeof(tmp) - 1, "%d", stats_get_hp(s));
        wid_set_text(w, tmp);

        if (stats_get_hp(s) <= THING_MIN_HEALTH) {
            wid_hide(left_ball, 0);
            wid_hide(left_ball_stats, 0);
            wid_hide(left_ball_squiggles, 0);
        }

        if (last_hp != stats_get_hp(s)) {
            int delta = stats_get_hp(s) - last_hp;

            if (last_hp > stats_get_hp(s)) {
                if (stats_get_hp(s) >= stats_get_max_hp(s)) {
                    /*
                     * Don't flash as this is just a magical effect winding 
                     * down.
                     */
                } else {
                    /*
                     * Warn the user about the decrease.
                     */
                    if (stats_get_hp(s) > 0) {
                        wid_effect_pulses(w);
                    }

                    wid_set_mode(w, WID_MODE_ACTIVE);
                    wid_set_color(w, WID_COLOR_TEXT, RED);
                }

                if (delta < -1) {
                    if (!strcmp("valkyrie", global_config.stats.pclass)) {
                        sound_play("female_player_hit");
                    } else {
                        sound_play("player_hit");
                    }
                }

            } else if (player && last_hp) {
                int count = (rand() % delta) + 1;

                if (count > 10) {
                    count = 10;
                }

                int i = count;
                while (i--) {
                    tpp what = tp_find("data/things/heart");
                    if (!what) {
                        ERR("cannot place heart");
                    }

                    widp heart =
                        wid_new_square_window("wid player_stats container");

                    fpoint tl = {0.1, 0.78};
                    fpoint br = {0.15, 0.85};

                    double dx = (myrand() % 100) / 1500.0;
                    double dy = (myrand() % 100) / 1000.0;

                    tl.x += dx;
                    tl.y += dy;
                    br.x += dx;
                    br.y += dy;

                    wid_set_tl_br_pct(heart, tl, br);
                    double px, py;
                    wid_get_pct(heart, &px, &py);

                    wid_set_thing_template(heart, what);

                    uint32_t delay = 2000;
                    wid_move_to_pct_centered_in(heart, 
                                                px - 0.1, py - 0.1, delay);
                    wid_fade_out(heart, delay + 100);
                    wid_destroy_in(heart, delay - 100);

                    wid_raise(heart);
                    wid_set_mode(heart, WID_MODE_NORMAL);
                    wid_set_color(heart, WID_COLOR_TEXT, WHITE);
                    wid_set_color(heart, WID_COLOR_BG, WHITE);
                    wid_set_color(heart, WID_COLOR_TL, WHITE);
                    wid_set_color(heart, WID_COLOR_BR, WHITE);
                    wid_set_no_shape(heart);
                    wid_set_do_not_lower(heart, 1);

                    double scale = delta / (double) count;
                    if (scale < 1.0) {
                        scale = 1.0;
                    }

                    if (scale > 2.0) {
                        scale = 2.0;
                    }

                    wid_scaling_to_pct_in(heart, scale, 0.5, delay + 100, 0);
                }
            }

            if ((stats_get_hp(s) < s->max_hp / 10) && (stats_get_hp(s) > THING_MIN_HEALTH)) {
                wid_scaling_to_pct_in(w, 1.0, 3.15, wid_scaling_forever_delay, 1);
                wid_set_mode(w, WID_MODE_NORMAL);
                wid_set_color(w, WID_COLOR_TEXT, RED);
            }
        }

        last_hp = stats_get_hp(s);
    }

    widp wid_item_bar;

    {
        widp w = wid_item_bar = 
            wid_new_container(wid_player_action, 
                              "wid player_stats container");

        fpoint tl = {0.2, 0.4};
        fpoint br = {0.9, 1.0};

        wid_set_tl_br_pct(w, tl, br);

        wid_set_tex(w, 0, "itembar");
        wid_raise(w);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);
        wid_set_square(w);

        /*
         * Allow over-sized items
         */
        wid_set_disable_scissors(wid_item_bar, 1);
    }

    {
        double x;
        double max_across = 10.0;
        double border_left = 0.06;
        double border_right = 0.24;
        double dx = (1.0 - (border_left + border_right)) / max_across;

        double y;
        double max_down = 1.0;
        double border_top = 0.011;
        double border_bottom = 0.26;
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

            wid_set_client_context(w, (void*) (uintptr_t) i);
            wid_set_client_context2(w, (void*) (uintptr_t) x);

            wid_player_inventory_button_style(w, s, item,
                                              true, /* action bar item */
                                              false, /* worn item */
                                              false, /* inventory item */
                                              x);

            wid_set_on_tooltip(w, wid_player_action_tooltip);
            wid_set_on_mouse_down(w, wid_player_action_button_mouse_down);

            wid_set_on_joy_down(w, wid_player_action_button_joy_down);

            {
                widp w = 
                    wid_new_container(wid_item_bar, "inventory shortcut");

                tl.y += 0.25;
                br.y += 0.25;

                wid_set_tl_br_pct(w, tl, br);

                wid_raise(w);

                color c = WHITE;
                c.a = 0;

                wid_set_color(w, WID_COLOR_TEXT, WHITE);
                wid_set_color(w, WID_COLOR_BG, c);
                wid_set_color(w, WID_COLOR_TL, BLACK);
                wid_set_color(w, WID_COLOR_BR, BLACK);
                wid_set_no_shape(w);

                char tmp[40];
                snprintf(tmp, sizeof(tmp)-1, "%d", (i + 1) % 10);
                wid_set_text(w, tmp);

                wid_set_text_bot(w, true);
                wid_set_text_outline(w, true);
                wid_set_font(w, med_font);
            }
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
    wid_destroy_nodelay(&wid_player_action);
}
