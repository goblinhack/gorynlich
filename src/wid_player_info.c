/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_player_info.h"
#include "wid_player_inventory.h"
#include "wid_player_stats.h"
#include "thing_stats.h"
#include "string_util.h"
#include "thing_template.h"
#include "time_util.h"
#include "client.h"
#include "name.h"

static widp wid_player_info;
static uint8_t wid_player_info_init_done;
int wid_player_info_set_name;

static void wid_player_info_create(thing_statsp, int fast);
static void wid_player_info_destroy(void);
static thing_statsp player_stats;

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

void wid_player_info_hide (int fast)
{
    if (wid_player_info) {
        if (fast) {
            wid_hide(wid_player_info, 0);
        } else {
            wid_move_to_pct_centered_in(wid_player_info, -1.15, 0.45, 200);
        }
    }

    wid_player_info_destroy();
}

void wid_player_info_visible (thing_statsp s, int fast)
{
    wid_player_info_create(s, fast);
}

int wid_player_info_is_visible (void)
{
    if (!wid_player_info) {
        return (false);
    }

    if (wid_is_hidden(wid_player_info)) {
        return (false);
    }

    return (true);
}

static void wid_player_info_buttons_tick (widp wid)
{
    if (!player_stats) {
        return;
    }

    int tick = time_get_time_ms() / 100;
    int which = tick % 8;

    const char *tn = player_stats->pclass;
    if (!*tn) {
        return;
    }

    char tilename[40];

    switch (which) {
    case 0: snprintf(tilename, sizeof(tilename) - 1, "%s-right", tn); break;
    case 1: snprintf(tilename, sizeof(tilename) - 1, "%s-br", tn); break;
    case 2: snprintf(tilename, sizeof(tilename) - 1, "%s-down", tn); break;
    case 3: snprintf(tilename, sizeof(tilename) - 1, "%s-bl", tn); break;
    case 4: snprintf(tilename, sizeof(tilename) - 1, "%s-left", tn); break;
    case 5: snprintf(tilename, sizeof(tilename) - 1, "%s-tl", tn); break;
    case 6: snprintf(tilename, sizeof(tilename) - 1, "%s-up", tn); break;
    case 7: snprintf(tilename, sizeof(tilename) - 1, "%s-tr", tn); break;
    }

    wid_set_tilename(wid, tilename);
    wid_set_animate(wid, false);
}

/*
 * Key down etc...
 */
static uint8_t wid_player_info_name_receive_input (widp w, 
                                                   const SDL_KEYSYM *key)
{
    int r;
    char *name = (char*) wid_get_text(w);

    switch (key->sym) {
        case SDLK_RETURN: {
            /*
             * Change name.
             */
            wid_set_show_cursor(w, false);
            wid_set_on_key_down(w, 0);

            if (!strlen(name)) {
                strlcpy(player_stats->pname, name_random(player_stats->pclass),
                        sizeof(player_stats->pname) - 1);

                wid_set_text(w, player_stats->pname);
                return (true);
            }
        }

        default:
            break;
    }

    /*
     * Feed to the general input handler
     */
    r = (wid_receive_input(w, key));

    wid_player_info_set_name = true;

    strlcpy(player_stats->pname, name, sizeof(player_stats->pname) - 1);

    return (r);
}

static uint8_t wid_player_info_select_name_event (widp w, int32_t x, int32_t y,
                                             uint32_t button)
{
    wid_set_on_key_down(w, wid_player_info_name_receive_input);

    wid_set_show_cursor(w, true);

    return (true);
}

static uint8_t 
wid_player_info_button_style_mouse_down (widp w,
                                         int32_t x, int32_t y,
                                         uint32_t button)
{
    uint32_t id = (typeof(id)) (uintptr_t) wid_get_client_context(w);

    itemp over_item = &player_stats->worn[id];

    if (!wid_mouse_template) {
        wid_player_item_pick_up(w, over_item);
    } else {
        int valid = true;

        tpp item_tp = id_to_tp(wid_item.id);
        if (!item_tp) {
            ERR("no item to place");
            return (false);
        }

        switch (id) {
        case THING_WORN_ARMOR:
            if (!tp_is_armor(item_tp)) {
                valid = false;
                MESG(WARNING, "This item wont work as armor");
                break;
            }
            break;

        case THING_WORN_HELMET:
            if (!tp_is_helmet(item_tp)) {
                valid = false;
                MESG(WARNING, "This item wont work as a helmet");
                break;
            }
            break;

        case THING_WORN_BOOTS:
            if (!tp_is_boots(item_tp)) {
                valid = false;
                MESG(WARNING, "This item wont work as a boots");
                break;
            }
            break;

        case THING_WORN_ARM_RIGHT:
        case THING_WORN_ARM_LEFT:
            if (!tp_is_hand_item(item_tp)) {
                valid = false;
                MESG(WARNING, "This item can't be worn on the hand. "
                     "This slot is for things like rings.");
                break;
            }
            break;
        }

        if (!valid) {
            return (true);
        }

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
             * somewhere else in the inventory?
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
                player_inventory_sort(player_stats);
            }
        }

        if (dropped) {
            memset(&wid_item, 0, sizeof(item_t));

            wid_destroy(&wid_mouse_template);

            wid_set_client_context(w, 0);
        }
    }

    stats_bump_version(player_stats);

    wid_player_stats_redraw(true /* fast */);

    return (true);
}

static uint8_t 
wid_player_info_button_style_joy_down (widp w, int32_t x, int32_t y)
{
    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
        wid_player_info_button_style_mouse_down(w, x, y, SDL_BUTTON_LEFT);
        return (true);
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

static void wid_player_info_create (thing_statsp s, int fast)
{
    if (wid_player_info) {
        return;
    }

    player_stats = s;

    if (!wid_player_info) {
        widp w = wid_player_info = 
                        wid_new_rounded_window("wid player info");

        fpoint tl = {0.0, 0.0};
        fpoint br = {0.3, 0.8};

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
        widp w = 
            wid_new_square_button(wid_player_info, 
                                  "wid player_stats player");

        fpoint tl = {0.3, 0.15};
        fpoint br = {0.7, 0.35};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_on_tick(w, wid_player_info_buttons_tick);

        wid_set_no_shape(w);

        if (player_stats->pclass[0]) {
            tpp tp = thing_stats_to_tp(player_stats);

            const char *tooltip = tp_get_tooltip(tp);

            if (!player) {
                wid_set_tooltip(w, tooltip, 0 /* font */);
            }
        }
    }

    {
        fpoint tl = {0.05, 0.4};
        fpoint br = {0.25, 0.45};

        widp w = wid_new_container(wid_player_info, 
                                   "wid intro name container");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Title");
        wid_set_font(w, small_font);
        wid_set_no_shape(w);

        wid_set_color(w, WID_COLOR_BG, BLACK);

        if (player) {
            wid_set_color(w, WID_COLOR_TL, BLACK);
            wid_set_color(w, WID_COLOR_BR, BLACK);
        } else {
            wid_set_color(w, WID_COLOR_TL, STEELBLUE);
            wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        }

        wid_set_text_outline(w, true);
        wid_raise(w);

        /*
         * Allow over-sized items
         */
        wid_set_disable_scissors(w, 1);
    }

    {
        fpoint tl = {0.26, 0.4};
        fpoint br = {0.9, 0.45};

        widp w = wid_new_container(wid_player_info, 
                                   "wid intro name value");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, player_stats->pname);
        wid_set_font(w, small_font);

        wid_set_color(w, WID_COLOR_BG, BLACK);

        if (player) {
            wid_set_color(w, WID_COLOR_TL, BLACK);
            wid_set_color(w, WID_COLOR_BR, BLACK);
        } else {
            wid_set_color(w, WID_COLOR_TL, STEELBLUE);
            wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        }

        wid_set_square(w);
        wid_set_bevelled(w, true);
        wid_set_bevel(w, 2);
        wid_set_text_outline(w, true);

        if (!player) {
            wid_set_on_mouse_down(w, wid_player_info_select_name_event);
        }

        wid_raise(w);
    }

    {
        fpoint tl = {0.05, 0.46};
        fpoint br = {0.24, 0.51};

        widp w = wid_new_container(wid_player_info, 
                                   "wid intro pclass container");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Pedigree");
        wid_set_font(w, small_font);
        wid_set_no_shape(w);

        wid_set_color(w, WID_COLOR_BG, BLACK);
        wid_set_color(w, WID_COLOR_TL, STEELBLUE);
        wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        wid_set_text_outline(w, true);
        wid_raise(w);
    }

    {
        fpoint tl = {0.26, 0.46};
        fpoint br = {0.9, 0.51};

        widp w = wid_new_container(wid_player_info, 
                                   "wid intro pclass value");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, player_stats->pclass);
        wid_set_font(w, small_font);
        wid_set_no_shape(w);

        wid_set_color(w, WID_COLOR_BG, BLACK);
        wid_set_color(w, WID_COLOR_TL, STEELBLUE);
        wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        wid_set_text_outline(w, true);
        wid_raise(w);
    }

    {
        widp w = wid_new_square_button(wid_player_info, 
                                       "wid intro item helmet");

        fpoint tl = {0.8, 0.55};
        fpoint br = {0.95, 0.63};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_tooltip(w, "Your helmet", 0 /* font */);

        item_t item = s->worn[THING_WORN_ARMOR];

        wid_set_client_context(w, (void*) (uintptr_t) THING_WORN_ARMOR);

        wid_player_inventory_button_style(w, s, item,
                                          false, /* action bar item */
                                          true, /* worn item */
                                          false, /* inventory item */
                                          THING_WORN_ARMOR);

        wid_set_on_mouse_down(w, wid_player_info_button_style_mouse_down);

        wid_set_on_joy_down(w, wid_player_info_button_style_joy_down);
    }

    {
        widp w = wid_new_square_button(wid_player_info, 
                                       "wid intro item armor");

        fpoint tl = {0.3, 0.55};
        fpoint br = {0.45, 0.63};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_tooltip(w, "Your armor", 0 /* font */);

        item_t item = s->worn[THING_WORN_HELMET];

        wid_set_client_context(w, (void*) (uintptr_t) THING_WORN_HELMET);

        wid_player_inventory_button_style(w, s, item,
                                          false, /* action bar item */
                                          true, /* worn item */
                                          false, /* inventory item */
                                          THING_WORN_HELMET);

        wid_set_on_mouse_down(w, wid_player_info_button_style_mouse_down);

        wid_set_on_joy_down(w, wid_player_info_button_style_joy_down);
    }

    {
        widp w = wid_new_square_button(wid_player_info, 
                                       "wid intro item boots");

        fpoint tl = {0.3, 0.71};
        fpoint br = {0.45, 0.79};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_tooltip(w, "Your boots", 0 /* font */);

        item_t item = s->worn[THING_WORN_BOOTS];

        wid_set_client_context(w, (void*) (uintptr_t) THING_WORN_BOOTS);

        wid_player_inventory_button_style(w, s, item,
                                          false, /* action bar item */
                                          true, /* worn item */
                                          false, /* inventory item */
                                          THING_WORN_BOOTS);

        wid_set_on_mouse_down(w, wid_player_info_button_style_mouse_down);

        wid_set_on_joy_down(w, wid_player_info_button_style_joy_down);
    }

    {
        widp w = wid_new_square_button(wid_player_info, 
                                       "wid intro item left arm");

        fpoint tl = {0.8, 0.71};
        fpoint br = {0.95, 0.79};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_tooltip(w, "Left hand", 0 /* font */);

        item_t item = s->worn[THING_WORN_ARM_LEFT];

        wid_set_client_context(w, (void*) (uintptr_t) THING_WORN_ARM_LEFT);

        wid_player_inventory_button_style(w, s, item,
                                          false, /* action bar item */
                                          true, /* worn item */
                                          false, /* inventory item */
                                          THING_WORN_ARM_LEFT);

        wid_set_on_mouse_down(w, wid_player_info_button_style_mouse_down);

        wid_set_on_joy_down(w, wid_player_info_button_style_joy_down);
    }

    {
        widp w = wid_new_square_button(wid_player_info, 
                                       "wid intro item right arm");

        fpoint tl = {0.8, 0.80};
        fpoint br = {0.95, 0.88};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_tooltip(w, "Right hand", 0 /* font */);

        item_t item = s->worn[THING_WORN_ARM_RIGHT];

        wid_set_client_context(w, (void*) (uintptr_t) THING_WORN_ARM_RIGHT);

        wid_player_inventory_button_style(w, s, item,
                                          false, /* action bar item */
                                          true, /* worn item */
                                          false, /* inventory item */
                                          THING_WORN_ARM_RIGHT);

        wid_set_on_mouse_down(w, wid_player_info_button_style_mouse_down);

        wid_set_on_joy_down(w, wid_player_info_button_style_joy_down);
    }

    if (fast) {
        wid_move_to_pct_centered(wid_player_info, 0.15, 0.45);
    } else {
        wid_move_to_pct_centered(wid_player_info, -1.15, 0.45);
        wid_move_to_pct_centered_in(wid_player_info, 0.15, 0.45, 200);
    }

    wid_raise(wid_player_info);
    wid_update(wid_player_info);

    wid_set_movable(wid_player_info, false);
}

static void wid_player_info_destroy (void)
{
    wid_destroy(&wid_player_info);
}
