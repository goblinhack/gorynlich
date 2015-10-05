/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_player_inventory.h"
#include "string_util.h"
#include "thing_template.h"
#include "wid_player_stats.h"
#include "color.h"
#include "glapi.h"
#include "thing.h"

static widp wid_player_inventory;
static widp wid_player_inventory_container;
static uint8_t wid_player_inventory_init_done;

static void wid_player_inventory_create(thing_statsp, int fast);
static void wid_player_inventory_destroy(void);
static thing_statsp player_stats;

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

void wid_player_inventory_hide (int fast)
{
    if (wid_player_inventory) {
        if (fast) {
            wid_hide(wid_player_inventory, 0);
        } else {
            wid_move_to_pct_centered_in(wid_player_inventory, 1.85, 0.45, 200);
        }
    }

    wid_player_inventory_destroy();
}

void wid_player_inventory_visible (thing_statsp s, int fast)
{
    wid_player_inventory_create(s, fast);
}

int wid_player_inventory_is_visible (void)
{
    if (wid_player_inventory) {
        return (1);
    } else {
        return (0);
    }
}

void wid_player_inventory_button_style (widp w,
                                        thing_statsp s,
                                        const item_t item,
                                        const int action_bar_item,
                                        const int worn_item,
                                        const int inventory_item,
                                        const int index)
{
    color c;

#ifdef SEEMS_TO_ADD_NOTHING
    wid_set_blit_outline(w, 1.5);
    c = BLACK;
    c.a = 30;
    wid_set_color(w, WID_COLOR_BLIT_OUTLINE, c);
#endif

    wid_set_mode(w, WID_MODE_NORMAL);

    if (action_bar_item) {
        c = WHITE;
        c.a = 0;
        wid_set_color(w, WID_COLOR_TL, c);

        c = GRAY10;
        c.a = 0;
        wid_set_color(w, WID_COLOR_BR, c);

        c = WHITE;
        c.a = 20;
        wid_set_color(w, WID_COLOR_BG, c);

        wid_set_rounded_small(w);
    } else {
        c = STEELBLUE;
        c.a = 30;
        wid_set_color(w, WID_COLOR_TL, c);

        c = GRAY;
        c.a = 30;
        wid_set_color(w, WID_COLOR_BR, c);

        c = WHITE;
        c.a = 100;
        wid_set_color(w, WID_COLOR_BG, c);
    }

    wid_set_blit_center(w, true);

    wid_set_mode(w, WID_MODE_OVER);

    c = RED;
    c.a = 50;
    wid_set_color(w, WID_COLOR_BG, c);

    c = STEELBLUE;
    c.a = 30;
    wid_set_color(w, WID_COLOR_TL, c);

    c = GRAY;
    c.a = 30;

    wid_set_color(w, WID_COLOR_BR, c);

    wid_set_mode(w, WID_MODE_NORMAL);

    if (!item.id) {
        return;
    }

    c = WHITE;
    c.a = 50;
    wid_set_color(w, WID_COLOR_BG, c);

    tpp tp = id_to_tp(item.id);

    if (action_bar_item) {
        if (player && 
            (index == stats_get_action_bar_index(s))) {
            color c = YELLOW;
            wid_set_color(w, WID_COLOR_TL, c);
            wid_set_color(w, WID_COLOR_BR, c);
            wid_set_rounded_small(w);
            wid_set_bevel(w, 4);
        }

        if (tp == player->weapon) {
            color c = RED;
            wid_set_color(w, WID_COLOR_TL, c);
            wid_set_color(w, WID_COLOR_BR, c);
            wid_set_rounded_small(w);
            wid_set_bevel(w, 4);
        }
    }

    wid_set_thing_template(w, tp);

    /*
     * To have the item appear a bit oversized but it obscure item counts.
     */
//    wid_blit_scale_immediate(w, 1.5);

    int quantity = item.quantity;
    int quality = item.quality;

    if (quantity > 1) {
        char tmp[20];
        snprintf(tmp, sizeof(tmp) - 1, "x%u", quantity);

        wid_set_text(w, tmp);
        wid_set_font(w, vsmall_font);
        wid_set_text_lhs(w, true);
        wid_set_text_top(w, true);
        wid_set_color(w, WID_COLOR_TEXT, GREEN);
        wid_set_text_outline(w, true);
    }

    if (item.enchanted) {
        char tmp[20];
        snprintf(tmp, sizeof(tmp) - 1, "+%u", item.enchanted);

        wid_set_text(w, tmp);
        wid_set_font(w, vsmall_font);
        wid_set_text_rhs(w, true);
        wid_set_text_top(w, true);
        wid_set_color(w, WID_COLOR_TEXT, YELLOW);
        wid_set_text_outline(w, true);
    }

    /*
     * Quality bar.
     */
    if (tp_is_degradable(tp)) {
        widp wid_bar = wid_new_square_button(w, "quality bar");

        wid_set_bevelled(wid_bar, true);
        wid_set_bevel(wid_bar, 2);

        fpoint tl = {0.1f, 0.85f};
        fpoint br = {0.9f, 1.0f};

        br.x = (0.9 / ((float)THING_ITEM_QUALITY_MAX) )
                    * ((double)quality);

        wid_set_tl_br_pct(wid_bar, tl, br);

        color col;

        switch (quality) {
            case 7: col = GREEN; break;
            case 6: col = YELLOW; break;
            case 5: col = ORANGE; break;
            case 4: col = ORANGE; break;
            case 3: col = RED; break;
            case 2: col = RED; break;
            case 1: col = RED; break;
            default:
            case 0: col = RED; break;
        }

        glcolor(col);

        wid_set_mode(wid_bar, WID_MODE_NORMAL);
        wid_set_color(wid_bar, WID_COLOR_BG, col);
    }

    /*
     * Tooltip
     */
    const char *tooltip = tp_get_tooltip(tp);
    if (!tooltip) {
        ERR("need a tooltip defined for %s", 
            tp_name(tp));
    } else {
        char *full_tooltip;
        char *tmp = strappend(tooltip, "\n\n%%fmt=left$");

        if (tp_is_weapon(tp)) {
            int damage = tp_get_damage(tp);
            if (!damage) {
                tpp projectile = tp_fires(tp);
                if (projectile) {
                    damage = tp_get_damage(projectile);
                }
            }

            if (damage) {
                char *tmp2;

                if (item.enchanted) {
                    tmp2 = dynprintf("%%%%fmt=left$Damage: 1d%d (+%d)\n",
                                     damage, 
                                     item.enchanted);
                } else {
                    tmp2 = dynprintf("%%%%fmt=left$Damage: 1d%d\n",
                                     damage);
                }

                char *old = tmp;
                tmp = strappend(old, tmp2);
                myfree(old);
                myfree(tmp2);
            }
        }

        /*
         * Current quality
         */
        if (tp_is_degradable(tp)) {
            const char *str;
            if (quality >= THING_ITEM_QUALITY_MAX) {
                str = "%%fg=green$mint condition";
            } else if (quality >= THING_ITEM_QUALITY_MAX - 2) {
                str = "%%fg=green$second hand condition";
            } else if (quality >= THING_ITEM_QUALITY_MAX / 2) {
                str = "%%fg=red$damaged";
            } else {
                str = "%%fg=red$heavily damaged";
            }

            char *tmp2 = dynprintf(
                        "%%%%fmt=left$"
                        "Quality: %d%%%%, %s\n", 
                        (int) ((((double) quality) /
                        ((double) THING_ITEM_QUALITY_MAX)) * 100.0),
                        str);

            char *old = tmp;
            tmp = strappend(old, tmp2); 
            myfree(old);
            myfree(tmp2);
        }

        /*
         * HP bonus. Don't show negative gains, i.e. curses
         */
        if (tp_get_bonus_hp_on_use(tp) > 0) {
            /*
             * Let the player guess/learn about food.
             */
            if (!tp_is_food(tp)) {
                char *tmp2 = dynprintf(
                                "%%%%fmt=left$"
                                "HP gain on use: %d\n", 
                                tp_get_bonus_hp_on_use(tp));

                char *old = tmp;
                tmp = strappend(old, tmp2);
                myfree(old);
                myfree(tmp2);
            }
        }

        /*
         * ID bonus
         */
        if (tp_get_bonus_magic_on_use(tp)) {
            char *tmp2 = dynprintf(
                            "%%%%fmt=left$"
                            "ID gain on use: %d\n", 
                            tp_get_bonus_magic_on_use(tp));

            char *old = tmp;
            tmp = strappend(old, tmp2);
            myfree(old);
            myfree(tmp2);
        }

        /*
         * Failure rate
         */
        uint32_t val = tp_get_d10000_chance_of_breaking(tp);
        if (val) {
            const char *str;
            if (val == 0) {
                str = "%%fg=green$near indestructible item";
            } else if (val <= 3) {
                str = "%%fg=green$very low, masterwork item";
            } else if (val <= 5) {
                str = "%%fg=green$very low, high quality";
            } else if (val <= 10) {
                str = "%%fg=green$fair chance of failing";
            } else if (val <= 20) {
                str = "%%fg=red$high chance of failing";
            } else if (val <= 50) {
                str = "%%fg=red$high chance of failing";
            } else if (val <= 100) {
                str = "%%fg=red$very high, poor quality item";
            } else {
                str = "this item is rubbish";
            }

            char *tmp2 = dynprintf("%%%%fmt=left$"
                                   "Failure rate: %s\n", str);

            char *old = tmp;
            tmp = strappend(old, tmp2);
            myfree(old);
            myfree(tmp2);
        }

        /*
         * Rarity
         */
        val = tp_get_d10000_chance_of_appearing(tp);
        if (val) {
            const char *str = 0;
            if (val >= 20) {
                str = 0;
            } else if (val >= 10) {
                str = "Uncommon item";
            } else if (val >= 5) {
                str = "Rare item";
            } else if (val >= 1) {
                str = "Very rare";
            } else {
                str = "Very very rare item";
            }

            if (str) {
                char *tmp2 = dynprintf("%%%%fmt=left$%s\n", str);

                char *old = tmp;
                tmp = strappend(old, tmp2);
                myfree(old);
                myfree(tmp2);
            }
        }

        if (tp_is_life_saving(tp)) {
            int val = thing_stats_val_to_modifier(tp_get_stats_attack_melee(tp));
            char *tmp2;

            tmp2 = dynprintf("%%%%fmt=left$This item might just save your life, if you wear it...\n", val);

            if (tmp2) {
                char *old = tmp;
                tmp = strappend(old, tmp2);
                myfree(old);
                myfree(tmp2);
            }
        }

        if (tp_get_stats_attack_melee(tp)) {
            int val = thing_stats_val_to_modifier(tp_get_stats_attack_melee(tp));
            char *tmp2;

            if (val > 0) {
                tmp2 = dynprintf("%%%%fmt=left$This awesome item gives +%d melee attack modifier\n", val);
            } else if (val < 0) {
                tmp2 = dynprintf("%%%%fmt=left$This awful item gives %d melee attack modifier\n", val);
            } else {
                tmp2 = 0;
            }

            if (tmp2) {
                char *old = tmp;
                tmp = strappend(old, tmp2);
                myfree(old);
                myfree(tmp2);
            }
        }

        if (tp_get_stats_attack_ranged(tp)) {
            int val = thing_stats_val_to_modifier(tp_get_stats_attack_ranged(tp));
            char *tmp2;

            if (val > 0) {
                tmp2 = dynprintf("%%%%fmt=left$This amazing item gives +%d ranged attack modifier\n", val);
            } else if (val < 0) {
                tmp2 = dynprintf("%%%%fmt=left$This dark item gives %d ranged attack modifier\n", val);
            } else {
                tmp2 = 0;
            }

            if (tmp2) {
                char *old = tmp;
                tmp = strappend(old, tmp2);
                myfree(old);
                myfree(tmp2);
            }
        }

        if (tp_get_stats_defense(tp)) {
            int val = thing_stats_val_to_modifier(tp_get_stats_defense(tp));
            char *tmp2;

            if (val > 0) {
                tmp2 = dynprintf("%%%%fmt=left$This stunning item gives +%d defense modifier\n", val);
            } else if (val < 0) {
                tmp2 = dynprintf("%%%%fmt=left$This evil item gives %d defense modifier\n", val);
            } else {
                tmp2 = 0;
            }

            if (tmp2) {
                char *old = tmp;
                tmp = strappend(old, tmp2);
                myfree(old);
                myfree(tmp2);
            }
        }

        if (tp_get_stats_speed(tp)) {
            int val = thing_stats_val_to_modifier(tp_get_stats_speed(tp));
            char *tmp2;

            if (val > 0) {
                tmp2 = dynprintf("%%%%fmt=left$This shiny item gives +%d speed modifier\n", val);
            } else if (val < 0) {
                tmp2 = dynprintf("%%%%fmt=left$This nasty item gives %d speed modifier\n", val);
            } else {
                tmp2 = 0;
            }

            if (tmp2) {
                char *old = tmp;
                tmp = strappend(old, tmp2);
                myfree(old);
                myfree(tmp2);
            }
        }

        if (tp_get_stats_toughness(tp)) {
            int val = thing_stats_val_to_modifier(tp_get_stats_toughness(tp));
            char *tmp2;

            if (val > 0) {
                tmp2 = dynprintf("%%%%fmt=left$This beautiful item gives +%d toughness modifier\n", val);
            } else if (val < 0) {
                tmp2 = dynprintf("%%%%fmt=left$This diabolic item gives %d toughness modifier\n", val);
            } else {
                tmp2 = 0;
            }

            if (tmp2) {
                char *old = tmp;
                tmp = strappend(old, tmp2);
                myfree(old);
                myfree(tmp2);
            }
        }

        if (tp_get_stats_vision(tp)) {
            int val = thing_stats_val_to_modifier(tp_get_stats_vision(tp));
            char *tmp2;

            if (val > 0) {
                tmp2 = dynprintf("%%%%fmt=left$This glistening item gives +%d vision modifier\n", val);
            } else if (val < 0) {
                tmp2 = dynprintf("%%%%fmt=left$This blinding item gives %d vision modifier\n", val);
            } else {
                tmp2 = 0;
            }

            if (tmp2) {
                char *old = tmp;
                tmp = strappend(old, tmp2);
                myfree(old);
                myfree(tmp2);
            }
        }

        full_tooltip = tmp;
        wid_set_tooltip(w, full_tooltip, med_font);
        myfree(full_tooltip);
    }
}

item_t wid_item;

uint8_t 
wid_player_item_pick_up (widp w, itemp over_item)
{
    tpp tp;
    uint32_t id = (typeof(id)) (uintptr_t) wid_get_client_context(w);

    tp = wid_get_thing_template(w);

    if (wid_mouse_template) {
        ERR("already carrying an item");
    }

    /*
     * Pick up an item.
     */
    if (!item_pop_all(over_item, &wid_item)) {
        /*
         * Failed.
         */
        return (true);
    }

    {
        widp w = wid_mouse_template = wid_new_square_window("mouse");

        fpoint tl = {0.0, 0.0};
        fpoint br = {0.04, 0.06};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_font(w, small_font);

        color c;
        c = WHITE;
        c.a = 0;
        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, c);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);

        wid_set_thing_template(w, tp);

        wid_set_bevelled(w, true);
        wid_set_bevel(w, 2);

        wid_raise(w);
        wid_move_to_abs_centered_in(w, mouse_x, mouse_y, 10);
        wid_set_do_not_lower(w, true);

        wid_raise(w);
        wid_update(w);

        wid_set_client_context(w, (void*) &wid_item);
    }

    return (true);
}

static uint8_t 
wid_player_inventory_button_style_mouse_down (widp w, 
                                              int32_t x, int32_t y,
                                              uint32_t button)
{
    uint32_t id = (typeof(id)) (uintptr_t) wid_get_client_context(w);
    itemp over_item = &player_stats->inventory[id];

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

    /*
     * Send an update now.
     */
    thing_stats_client_modified(player_stats);

    wid_player_stats_redraw(true /* fast */);

    return (true);
}

static uint8_t 
wid_player_inventory_button_style_joy_down (widp w, int32_t x, int32_t y)
{
    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
        wid_player_inventory_button_style_mouse_down(w, x, y, SDL_BUTTON_LEFT);
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

static void wid_player_inventory_create (thing_statsp s, int fast)
{
    if (wid_player_inventory) {
        return;
    }

    player_stats = s;

    if (!wid_player_inventory) {
        widp w = wid_player_inventory = 
                        wid_new_rounded_window("wid player inv");

        fpoint tl = {0.0, 0.0};
        fpoint br = {0.3, 0.8};

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
    double border_bottom = 0.20;
    double dy = (1.0 - (border_top + border_bottom)) / max_down;
    
    int item = 0;
    int which = 0;
    double dy2 = 0.0;

    static const uint32_t N_PER_CLASS = 
                    THING_INVENTORY_MAX / THING_INVENTORY_CLASSES;

    for (y = 0; y < max_down; y++) 
    for (x = 0; x < max_across; x++) 
    {
        if (!(item % N_PER_CLASS)) {
            widp w = wid_new_square_button(wid_player_inventory_container, 
                                           "wid player_stats inventory item");

            fpoint tl = {0.0, 0.0};
            fpoint br = {1.0, 0.0};

            tl.x = border_left;
            tl.y = border_top + (y * dy) + dy2;
            br.x = 1.0 - border_right;
            br.y = tl.y + 0.02;

            dy2 += 0.02;

            switch (which) {
            case 0:
                wid_set_text(w, "Food");
                break;
            case 1:
                wid_set_text(w, "Weapons");
                break;
            case 2:
                wid_set_text(w, "Magical Items");
                break;
            case 3:
                wid_set_text(w, "Spells");
                break;
            case 4:
                wid_set_text(w, "Other stuffs");
                break;
            }

            color c = WHITE;
            c.a = 0;

            wid_set_tl_br_pct(w, tl, br);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_TEXT, RED);
            wid_set_color(w, WID_COLOR_BG, c);
            wid_set_color(w, WID_COLOR_TL, c);
            wid_set_color(w, WID_COLOR_BR, c);

            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_TEXT, WHITE);
            wid_set_color(w, WID_COLOR_BG, c);
            wid_set_color(w, WID_COLOR_TL, c);
            wid_set_color(w, WID_COLOR_BR, c);

            wid_set_font(w, vsmall_font);
            wid_set_text_lhs(w, true);
            wid_set_text_top(w, true);

            which++;
        }

        widp w = wid_new_square_button(wid_player_inventory_container, 
                                       "wid player_stats inventory item");
        /*
         * Allow over-sized items
         */
        wid_set_disable_scissors(w, 1);
        fpoint tl = {0.0, 0.0};
        fpoint br = {0.0, 0.0};

        tl.x = border_left + (x * dx);
        tl.y = border_top + (y * dy) + dy2;
        br.x = tl.x + dx;
        br.y = tl.y + dy;

        wid_set_tl_br_pct(w, tl, br);

        wid_set_client_context(w, (void*) (uintptr_t) item);

        wid_player_inventory_button_style(w, s, s->inventory[item],
                                          false, /* action bar item */
                                          false, /* worn item */
                                          true, /* inventory item */
                                          item);

        wid_set_on_mouse_down(w, 
                              wid_player_inventory_button_style_mouse_down);

        wid_set_on_joy_down(w, 
                              wid_player_inventory_button_style_joy_down);

        item++;
    }

    if (fast) {
        wid_move_to_pct_centered(wid_player_inventory, 0.85, 0.45);
    } else {
        wid_move_to_pct_centered(wid_player_inventory, 1.85, 0.45);
        wid_move_to_pct_centered_in(wid_player_inventory, 0.85, 0.45, 200);
    }

    wid_raise(wid_player_inventory);
    wid_update(wid_player_inventory);

    wid_set_movable(wid_player_inventory, false);
}

static void wid_player_inventory_destroy (void)
{
    wid_destroy(&wid_player_inventory);
}
