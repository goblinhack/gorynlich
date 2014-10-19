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
                                        int id)
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

    if (!id) {
        return;
    }

    thing_templatep temp = id_to_thing_template(id);

    if (player && player->weapon && (temp == player->weapon)) {
        wid_set_color(w, WID_COLOR_TL, RED);
        wid_set_color(w, WID_COLOR_BR, RED);
    }

    wid_set_thing_template(w, temp);

    uint8_t quantity = s->carrying[id].quantity;
//        uint8_t cursed = s->carrying[id].cursed;
    uint8_t quality = s->carrying[id].quality;

    if (quantity > 1) {
        char tmp[20];
        snprintf(tmp, sizeof(tmp) - 1, "%u", quantity);

        wid_set_text(w, tmp);
        wid_set_font(w, small_font);
        wid_set_text_rhs(w, true);
        wid_set_text_bot(w, true);
    }

    /*
     * Quality bar.
     */
    if (thing_template_is_degradable(temp)) {
        quality = rand() % 8;

        widp wid_bar = wid_new_square_button(w, "quality bar");

        wid_set_bevelled(wid_bar, true);
        wid_set_bevel(wid_bar, 2);

        fpoint tl = {0.2f, 0.8f};
        fpoint br = {0.8f, 0.9f};

        br.x = (0.6 / ((float)THING_ITEM_QUALITY_MAX + 1) )
                    * ((double)quality);

        wid_set_tl_br_pct(wid_bar, tl, br);

        color col;

        switch (quality) {
            case 7: col = GREEN; break;
            case 6: col = YELLOW; break;
            case 5: col = ORANGE; break;
            case 4: col = ORANGE; break;
            case 3: col = BROWN; break;
            case 2: col = BROWN; break;
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
        char *tmp = strappend(tooltip, "\n\n");

        if (thing_template_is_weapon(temp)) {
            char *old = tmp;
            tmp = strappend(
                    old,
                    dynprintf("%d damage\n",
                              thing_template_get_damage(temp)));
            myfree(old);
        }

        full_tooltip = tmp;
        wid_set_tooltip(w, full_tooltip);
        myfree(full_tooltip);
    }
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

//        wid_set_on_mouse_down(w, wid_intro_settings_col4_mouse_event);
//

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
