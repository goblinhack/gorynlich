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
#include "wid_player_stats.h"
#include "color.h"
#include "glapi.h"
#include "thing.h"

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

void wid_player_inventory_button_style (widp w,
                                        player_stats_t *s,
                                        item_t item)
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

    if (!item.id) {
        return;
    }

    thing_templatep temp = id_to_thing_template(item.id);

    if (player && player->weapon && (temp == player->weapon)) {
        wid_set_color(w, WID_COLOR_TL, RED);
        wid_set_color(w, WID_COLOR_BR, RED);
    }

    wid_set_thing_template(w, temp);

    int quantity = item.quantity;
    int quality = item.quality;

    if (quantity > 1) {
        char tmp[20];
        snprintf(tmp, sizeof(tmp) - 1, "x%u", quantity);

        wid_set_text(w, tmp);
        wid_set_font(w, vsmall_font);
        wid_set_text_lhs(w, true);
        wid_set_text_top(w, true);
    }

    /*
     * Quality bar.
     */
    if (thing_template_is_degradable(temp)) {
        widp wid_bar = wid_new_square_button(w, "quality bar");

        wid_set_bevelled(wid_bar, true);
        wid_set_bevel(wid_bar, 2);

        fpoint tl = {0.1f, 0.8f};
        fpoint br = {0.9f, 0.9f};

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
    const char *tooltip = thing_template_get_tooltip(temp);
    if (!tooltip) {
        ERR("need a tooltip defined for %s", 
            thing_template_name(temp));
    } else {
        char *full_tooltip;
        char *tmp = strappend(tooltip, "\n\n%%fmt=left$");

        if (thing_template_is_weapon(temp)) {
            int damage = thing_template_get_damage(temp);
            if (!damage) {
                thing_templatep projectile = thing_template_fires(temp);
                if (projectile) {
                    damage = thing_template_get_damage(projectile);
                }
            }

            if (damage) {
                char *tmp2 = dynprintf("%%%%fmt=left$"
                                       "Damage\t\t1d%d\n", damage);

                char *old = tmp;
                tmp = strappend(old, tmp2);
                myfree(old);
                myfree(tmp2);
            }
        }

        /*
         * Current quality
         */
        if (thing_template_is_degradable(temp)) {
            const char *str;
            if (quality >= THING_ITEM_QUALITY_MAX) {
                str = "mint condition";
            } else if (quality >= THING_ITEM_QUALITY_MAX - 2) {
                str = "second hand condition";
            } else if (quality >= THING_ITEM_QUALITY_MAX / 2) {
                str = "damaged";
            } else {
                str = "heavily damaged";
            }

            char *tmp2 = dynprintf(
                        "%%%%fmt=left$"
                        "Quality\t\t%d%%%%, %s\n", 
                        (int) ((((double) quality) /
                        ((double) THING_ITEM_QUALITY_MAX)) * 100.0),
                        str);

            char *old = tmp;
            tmp = strappend(old, tmp2); 
            myfree(old);
            myfree(tmp2);
        }

        /*
         * HP bonus
         */
        if (thing_template_get_bonus_hp_on_use(temp)) {
            char *tmp2 = dynprintf(
                            "%%%%fmt=left$"
                            "HP gain on use\t\t%d\n", 
                            thing_template_get_bonus_hp_on_use(temp));

            char *old = tmp;
            tmp = strappend(old, tmp2);
            myfree(old);
            myfree(tmp2);
        }

        /*
         * ID bonus
         */
        if (thing_template_get_bonus_id_on_use(temp)) {
            char *tmp2 = dynprintf(
                            "%%%%fmt=left$"
                            "ID gain on use\t\t%d\n", 
                            thing_template_get_bonus_id_on_use(temp));

            char *old = tmp;
            tmp = strappend(old, tmp2);
            myfree(old);
            myfree(tmp2);
        }

        /*
         * Failure rate
         */
        uint32_t val = thing_template_get_failure_chance(temp);
        if (val) {
            const char *str;
            if (val >= 10000) {
                str = "enchanted, extremely low";
            } else if (val >= 5000) {
                str = "masterwork, very low";
            } else if (val >= 4000) {
                str = "low";
            } else if (val >= 3000) {
                str = "fair";
            } else if (val >= 2000) {
                str = "moderate";
            } else if (val >= 1000) {
                str = "high";
            } else if (val >= 500) {
                str = "very high";
            } else {
                str = "very very high, made by orcs?";
            }

            char *tmp2 = dynprintf("%%%%fmt=left$"
                                   "Failure rate\t%s\n", str);

            char *old = tmp;
            tmp = strappend(old, tmp2);
            myfree(old);
            myfree(tmp2);
        }

        /*
         * Rarity
         */
        val = thing_template_get_chance_of_appearing(temp);
        if (val) {
            const char *str;
            if (val >= 50) {
                str = "commonplace";
            } else if (val >= 20) {
                str = "common";
            } else if (val >= 10) {
                str = "uncommon";
            } else if (val >= 5) {
                str = "rare";
            } else if (val >= 1) {
                str = "very rare";
            } else {
                str = "unique";
            }

            char *tmp2 = dynprintf("%%%%fmt=left$"
                                   "Rarity\t\t%s\n", str);

            char *old = tmp;
            tmp = strappend(old, tmp2);
            myfree(old);
            myfree(tmp2);
        }

        full_tooltip = tmp;
        wid_set_tooltip(w, full_tooltip);
        myfree(full_tooltip);
    }
}

int mouse_item;

static uint8_t 
wid_player_inventory_button_style_mouse_down (widp w, 
                                              int32_t x, int32_t y,
                                              uint32_t button)

    static item_t item;
    thing_templatep thing_template;
    uint32_t id = (typeof(id)) (uintptr_t) wid_get_client_context(w);

    thing_template = wid_get_thing_template(w);

    if (!wid_mouse_template) {
        /*
         * Pick up an item.
         */
        widp w = wid_mouse_template = wid_new_square_window("moues");

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

        wid_set_thing_template(w, thing_template);

        wid_set_bevelled(w, true);
        wid_set_bevel(w, 2);

        wid_raise(w);
        wid_move_to_abs_centered_in(w, mouse_x, mouse_y, 10);
        wid_set_do_not_lower(w, true);

        wid_raise(w);
        wid_update(w);

        wid_set_client_context(w, (void*) &item);

        memcpy(&item, &player_stats->inventory[id], sizeof(item));
        memcpy(&player_stats->inventory[id], 0, sizeof(item));
    } else {
        /*
         * Drop the current item.
         */
        player_stats_item_add(0, player_stats, it, &item)

        if (player_stats->inventory[id]) {
            int i;

            for (i = 0; i < THING_INVENTORY_MAX; i++) {
                if (!player_stats->inventory[i]) {
                    player_stats->inventory[i] = player_stats->inventory[id];
                    player_stats->inventory[id] = 0;
                    break;
                }
            }
        }

        int mouse_item = (int) (uintptr_t) 
                        wid_get_client_context(wid_mouse_template);

        player_stats->inventory[id] = mouse_item;

        wid_destroy(&wid_mouse_template);
    }

    wid_player_stats_redraw();

    return (true);
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
    double border_bottom = 0.20;
    double dy = (1.0 - (border_top + border_bottom)) / max_down;
    
    int item = 0;

    for (y = 0; y < max_down; y++) 
    for (x = 0; x < max_across; x++) 
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

        int id = s->inventory[item];

        wid_player_inventory_button_style(w, s, id);

        wid_set_client_context(w, (void*) item);

        wid_set_on_mouse_down(w, 
                              wid_player_inventory_button_style_mouse_down);

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
