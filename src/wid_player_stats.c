/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_player_stats.h"
#include "wid_player_info.h"
#include "wid_player_inventory.h"
#include "wid_intro3.h"
#include "timer.h"
#include "string.h"
#include "client.h"

static widp wid_player_stats;
static widp wid_player_stats_container;
static uint8_t wid_player_stats_init_done;

static void wid_player_stats_create(player_stats_t *);
static void wid_player_stats_destroy(void);
static player_stats_t *player_stats;

typedef struct row_ {
    const char *col1;
    const char *col2;
    uint32_t increment;
    const char *tooltip;
} row;

enum {
    STAT_SPENDING_POINTS,
    STAT_EXPERIENCE,
    STAT_LEVEL,
    STAT_MAX_HP,
    STAT_MAX_ID,
    STAT_ATTACK_MELEE,
    STAT_ATTACK_RANGED,
    STAT_ATTACK_MAGICAL,
    STAT_DEFENSE,
    STAT_SPEED,
    STAT_VISION,
    STAT_HEALING,
};

#define WID_INTRO_MAX_SETTINGS 12 

static row rows[WID_INTRO_MAX_SETTINGS] = {
      /*                         Column 1     Column 2 Increm */
    { /* STAT_SPENDING_POINTS */ "Spending points", 0,   0,
    "Click on a statistic below to spend your hard earned points." },

    { /* STAT_EXPERIENCE      */ "Experience",      0,   0,
    "Experience points earned on defeating Bad Guys." },

    { /* STAT_LEVEL           */ "Level",           0,   0,
    "Your current experience level" },

    { /* STAT_MAX_HP          */ "Max Health",      "+", 5,
    "Health points. This is the max you can recover to with your healing ability." },

    { /* STAT_MAX_ID          */ "Max ID",          "+", 5,
    "ID is your life force used for magic and life changing events." },

    { /* STAT_ATTACK_MELEE    */ "Attack, Melee",   "+", 1,
    "This modifier is the percentage damage gain you get in physical attacks." },

    { /* STAT_ATTACK_RANGED   */ "Attack, Ranged",  "+", 1,
    "This modifier is the percentage damage gain you get in missile attacks." },

    { /* STAT_ATTACK_MAGIC   */ "Attack, Magical",  "+", 1,
    "This modifier is the percentage damage gain you get in magical attacks." },

    { /* STAT_DEFENSE         */ "Defense",         "+", 1,
    "This modifier is the percentage gain you get in defense against attacks" },

    { /* STAT_SPEED           */ "Speed",           "+", 1,
    "This modifier is the percentage gain you get in running around like a nutter." },

    { /* STAT_VISION          */ "Vision",          "+", 1,
    "This modifier is the percentage gain you get in seeing bad stuffs." },

    { /* STAT_HEALING         */ "Healing",         "+", 1,
    "This modifier is the percentage gain you get in recovering health points." },
};

uint8_t wid_player_stats_init (void)
{
    if (!wid_player_stats_init_done) {
    }

    wid_player_stats_init_done = true;

    return (true);
}

void wid_player_stats_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_player_stats_init_done) {
        wid_player_stats_init_done = false;

        wid_player_stats_destroy();
    }
}

void wid_player_stats_hide (void)
{
    wid_player_stats_destroy();

    /*
     * May not be set yet if first time making the windows visible we also 
     * call hide.
     */
    if (player_stats) {
        client_socket_set_name(player_stats->pname);
        client_socket_set_pclass(player_stats->pclass);
    }
}

void wid_player_stats_visible (player_stats_t *s)
{
    wid_player_stats_create(s);
}

static void wid_player_stats_redraw (void)
{
    wid_player_stats_hide();
    wid_player_stats_create(player_stats);

    wid_player_info_hide();
    wid_player_info_visible(player_stats);

    wid_player_inventory_hide();
    wid_player_inventory_visible(player_stats);
}

static void wid_player_stats_reroll (void)
{
    player_stats_generate_random(player_stats);
    wid_player_stats_redraw();
}

static uint8_t wid_player_stats_reroll_mouse_event (widp w, 
                                                    int32_t x, int32_t y,
                                                    uint32_t button)
{
    wid_player_stats_reroll();

    return (true);
}

static uint8_t wid_player_stats_reroll_key_event (widp w, 
                                                  const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'q':
        case SDLK_ESCAPE:
            wid_player_stats_hide();
            return (true);

        default:
            break;
    }

    wid_player_stats_reroll();

    return (false);
}

static uint8_t wid_player_stats_col1_name_mouse_event (widp w,
                                                       int32_t x, int32_t y,
                                                       uint32_t button)
{
    /*
     * Increment.
     */
    int32_t row = (typeof(row)) (intptr_t) wid_get_client_context(w);

    if (!player_stats->spending_points) {
        return (true);
    }

    switch (row) {
    case STAT_EXPERIENCE:
        break;
    case STAT_LEVEL:
        break;
    case STAT_SPENDING_POINTS:
        break;
    case STAT_MAX_HP:
        player_stats->max_hp += rows[row].increment;
        player_stats->hp = player_stats->max_hp;
        player_stats->spending_points--;
        break;
    case STAT_MAX_ID:
        player_stats->max_id += rows[row].increment;
        player_stats->id = player_stats->max_id;
        player_stats->spending_points--;
        break;
    case STAT_ATTACK_MELEE:
        player_stats->attack_melee += rows[row].increment;
        player_stats->spending_points--;
        break;
    case STAT_ATTACK_RANGED:
        player_stats->attack_ranged += rows[row].increment;
        player_stats->spending_points--;
        break;
    case STAT_ATTACK_MAGICAL:
        player_stats->attack_magical += rows[row].increment;
        player_stats->spending_points--;
        break;
    case STAT_DEFENSE:
        player_stats->defense += rows[row].increment;
        player_stats->spending_points--;
        break;
    case STAT_SPEED:
        player_stats->speed += rows[row].increment;
        player_stats->spending_points--;
        break;
    case STAT_VISION:
        player_stats->vision += rows[row].increment;
        player_stats->spending_points--;
        break;
    case STAT_HEALING:
        player_stats->healing += rows[row].increment;
        player_stats->spending_points--;
        break;
    }

    wid_player_stats_redraw();

    return (true);
}

static uint8_t wid_player_stats_col2_mouse_event (widp w,
                                                  int32_t x, int32_t y,
                                                  uint32_t button)
{
    wid_player_stats_col1_name_mouse_event(w, x, y, button);

    return (true);
}

static void wid_player_stats_create (player_stats_t *s)
{
    player_stats = s;

    if (!wid_player_stats) {
        widp w = wid_player_stats = wid_new_rounded_window("wid player_stats");

        fpoint tl = {0.0, 0.0};
        fpoint br = {0.3, 0.9};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_font(w, small_font);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);
        wid_set_color(w, WID_COLOR_BG, WHITE);
        wid_set_color(w, WID_COLOR_TL, WHITE);
        wid_set_color(w, WID_COLOR_BR, WHITE);

        wid_set_tex(wid_player_stats, 0, "gothic_tall_axe");
        wid_set_square(wid_player_stats);
    }

    {
        widp w = wid_player_stats_container =
            wid_new_container(wid_player_stats, "wid player_stats container");

        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(w, tl, br);
    }

    static const double ROW_HEIGHT = 0.05;

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(rows); i++)
        {
            widp w = wid_new_square_button(wid_player_stats_container,
                                           rows[i].col1);
            if (rows[i].tooltip) {
                wid_set_tooltip(w, rows[i].tooltip);
            }

            fpoint tl = {0.05, 0.2};
            fpoint br = {0.48, 0.2};

            br.y += (double)i * ROW_HEIGHT;
            tl.y += (double)i * ROW_HEIGHT;
            br.y += ROW_HEIGHT;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, rows[i].col1);

            wid_set_color(w, WID_COLOR_TEXT, WHITE);
            wid_set_font(w, small_font);

            switch (i) {
            case STAT_EXPERIENCE:
                break;
            case STAT_LEVEL:
                break;
            case STAT_SPENDING_POINTS:
                wid_set_font(w, small_font);

                if (s->spending_points > 0) {
                    wid_set_color(w, WID_COLOR_TEXT, RED);
                }
                break;
            case STAT_MAX_HP:
                break;
            case STAT_ATTACK_MELEE:
                break;
            case STAT_ATTACK_RANGED:
                break;
            case STAT_DEFENSE:
                break;
            case STAT_SPEED:
                break;
            case STAT_VISION:
                break;
            case STAT_HEALING:
                break;
            }

            color c = WHITE;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            c.a = 255;
            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_on_mouse_down(w, wid_player_stats_col1_name_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);

            wid_set_tex(w, 0, "button_black");

            if (s->spending_points > 0) {
                if (i < STAT_MAX_HP) {
                    wid_set_no_shape(w);
                } else {
                    wid_set_square(w);
                }
            } else {
                wid_set_no_shape(w);
            }
        }
    }

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(rows); i++) {

            if (!rows[i].col2) {
                continue;
            }

            widp w = wid_new_square_button(wid_player_stats_container,
                                           rows[i].col2);
            if (rows[i].tooltip) {
                wid_set_tooltip(w, rows[i].tooltip);
            }

            fpoint tl = {0.48, 0.2};
            fpoint br = {0.595, 0.2};

            br.y += (double)i * ROW_HEIGHT;
            tl.y += (double)i * ROW_HEIGHT;
            br.y += ROW_HEIGHT;

            wid_set_tl_br_pct(w, tl, br);

            if (s->spending_points > 0) {
                wid_set_text(w, rows[i].col2);
            }

            wid_set_font(w, small_font);

            color c = WHITE;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            c.a = 255;
            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_on_mouse_down(w, wid_player_stats_col2_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);

            wid_set_tex(w, 0, "button_black");

            if (s->spending_points > 0) {
                if (i < STAT_MAX_HP) {
                    wid_set_no_shape(w);
                } else {
                    wid_set_square(w);
                }
            } else {
                wid_set_no_shape(w);
            }
        }
    }

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(rows); i++) {

            if (!rows[i].col1) {
                continue;
            }

            widp w = wid_new_square_button(wid_player_stats_container,
                                           rows[i].col1);
            if (rows[i].tooltip) {
                wid_set_tooltip(w, rows[i].tooltip);
            }

            fpoint tl = {0.82, 0.2};
            fpoint br = {0.95, 0.2};

            br.y += (double)i * ROW_HEIGHT;
            tl.y += (double)i * ROW_HEIGHT;
            br.y += ROW_HEIGHT;

            wid_set_tl_br_pct(w, tl, br);

            char *text = 0;
            int stat;

            wid_set_color(w, WID_COLOR_TEXT, WHITE);

            switch (i) {
            case STAT_EXPERIENCE:
                stat = s->experience;
                text = dynprintf("%u", stat);
                break;
            case STAT_LEVEL:
                stat = s->experience / 1000;
                text = dynprintf("%u", stat);
                break;
            case STAT_SPENDING_POINTS:
                stat = s->spending_points;
                text = dynprintf("%u", stat);

                if (s->spending_points > 0) {
                    wid_set_color(w, WID_COLOR_TEXT, RED);
                }
                break;
            case STAT_MAX_HP:
                stat = s->hp;
                if (s->hp != s->max_hp) {
                    text = dynprintf("%u (%u)", s->hp, s->max_hp);
                } else {
                    text = dynprintf("%u", s->max_hp);
                }
                break;
            case STAT_MAX_ID:
                stat = s->id;
                if (s->id != s->max_id) {
                    text = dynprintf("%u (%u)", s->id, s->max_id);
                } else {
                    text = dynprintf("%u", s->max_id);
                }
                break;
            case STAT_ATTACK_MELEE:
                stat = s->attack_melee;
                break;
            case STAT_ATTACK_RANGED:
                stat = s->attack_ranged;
                break;
            case STAT_ATTACK_MAGICAL:
                stat = s->attack_magical;
                break;
            case STAT_DEFENSE:
                stat = s->defense;
                break;
            case STAT_SPEED:
                stat = s->speed;
                break;
            case STAT_VISION:
                stat = s->vision;
                break;
            case STAT_HEALING:
                stat = s->healing;
                break;
            }

            int modifier = player_stats_get_modifier(stat);

            if (rows[i].increment == 1) {
                if (modifier <= -3) {
                    wid_set_color(w, WID_COLOR_TEXT, DARKGRAY);
                } else if (modifier <= -2) {
                    wid_set_color(w, WID_COLOR_TEXT, GRAY);
                } else if (modifier >= 3) {
                    wid_set_color(w, WID_COLOR_TEXT, GOLD);
                } else if (modifier >= 2) {
                    wid_set_color(w, WID_COLOR_TEXT, GREEN);
                } else {
                    wid_set_color(w, WID_COLOR_TEXT, WHITE);
                }
            }

            if (!text) {
                if (modifier > 0) {
                    text = dynprintf("%u / +%d", stat, modifier);
                } else if (modifier < 0) {
                    text = dynprintf("%u", stat);
                } else {
                    text = dynprintf("%u / %d", stat, modifier);
                }
            }

            wid_set_text(w, text);

            myfree(text);

            wid_set_font(w, small_font);
            wid_set_no_shape(w);
        }
    }

    {
        const char *button_name = "Re-roll";

        widp w = wid_new_rounded_small_button(wid_player_stats_container,
                                              button_name);

        fpoint tl = {0.1, 0.85};
        fpoint br = {0.9, 0.90};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_text(w, button_name);
        wid_set_font(w, small_font);

        color c = WHITE;

        c.a = 200;
        wid_set_mode(w, WID_MODE_NORMAL);
        wid_set_color(w, WID_COLOR_BG, c);

        c.a = 255;
        wid_set_mode(w, WID_MODE_OVER);
        wid_set_color(w, WID_COLOR_BG, c);

        wid_set_mode(w, WID_MODE_NORMAL);

        wid_set_on_mouse_down(w, wid_player_stats_reroll_mouse_event);
        wid_set_on_key_down(w, wid_player_stats_reroll_key_event);

        wid_set_tooltip(w, "Select this to try a differnet character. "
                        "But be certain, there is no undo...");

        wid_set_tex(w, 0, "button_red");
        wid_set_square(w);
    }

    wid_move_to_pct_centered(wid_player_stats, 0.2, 0.45);

    wid_raise(wid_player_stats);
    wid_update(wid_player_stats);
}

static void wid_player_stats_destroy (void)
{
    wid_destroy(&wid_player_stats);
}
