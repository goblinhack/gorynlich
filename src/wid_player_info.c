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

typedef struct row_ {
    const char *col1;
    const char *col2;
    uint32_t increment;
} row;

enum {
    STAT_SPENDING_POINTS,
    STAT_GAP1,
    STAT_EXPERIENCE,
    STAT_LEVEL,
    STAT_GAP2,
    STAT_MAX_HP,
    STAT_ATTACK_MELEE,
    STAT_ATTACK_RANGED,
    STAT_DEFENSE,
    STAT_SPEED,
    STAT_VISION,
    STAT_HEALING,
};

#define WID_INTRO_MAX_SETTINGS 12 

static row rows[WID_INTRO_MAX_SETTINGS] = {
      /*                         Column 1     Column 2 Increm */
    { /* STAT_SPENDING_POINTS */ "Spending points", 0,   0 },
    { /* STAT_GAP1,           */ "",                0,   0 },
    { /* STAT_EXPERIENCE      */ "Experience",      0,   0 },
    { /* STAT_LEVEL           */ "Level",           0,   0 },
    { /* STAT_GAP2            */ "",                0,   0 },
    { /* STAT_MAX_HP          */ "Max Health",      "+", 5 },
    { /* STAT_ATTACK_MELEE    */ "Attack, Melee",   "+", 1 },
    { /* STAT_ATTACK_RANGED   */ "Attack, Ranged",  "+", 1 },
    { /* STAT_DEFENSE         */ "Defense",         "+", 1 },
    { /* STAT_SPEED           */ "Speed",           "+", 1 },
    { /* STAT_VISION          */ "Vision",          "+", 1 },
    { /* STAT_HEALING         */ "Healing",         "+", 1 },
};

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

static void wid_player_info_redraw (void)
{
    wid_destroy_nodelay(&wid_player_info_container);
    wid_player_info_create(player_stats);
}

static void wid_player_info_reroll (void)
{
    player_stats_generate_random(player_stats);
    wid_player_info_redraw();
}

static uint8_t wid_player_info_all_done_mouse_event (widp w, 
                                                      int32_t x, int32_t y,
                                                      uint32_t button)
{
    wid_player_info_hide();

    return (true);
}

static uint8_t wid_player_info_all_done_key_event (widp w, 
                                                    const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'q':
        case SDLK_ESCAPE:
            wid_player_info_hide();
            return (true);

        default:
            break;
    }

    return (false);
}

static uint8_t wid_player_info_reroll_mouse_event (widp w, 
                                                    int32_t x, int32_t y,
                                                    uint32_t button)
{
    wid_player_info_reroll();

    return (true);
}

static uint8_t wid_player_info_reroll_key_event (widp w, 
                                                  const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'q':
        case SDLK_ESCAPE:
            wid_player_info_hide();
            return (true);

        default:
            break;
    }

    wid_player_info_reroll();

    return (false);
}

static uint8_t wid_player_info_col1_name_mouse_event (widp w,
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
        player_stats->hp +=
        player_stats->max_hp += rows[row].increment;
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

    wid_player_info_redraw();

    return (true);
}

static uint8_t wid_player_info_col2_mouse_event (widp w,
                                                  int32_t x, int32_t y,
                                                  uint32_t button)
{
    wid_player_info_col1_name_mouse_event(w, x, y, button);

    return (true);
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

    static const double ROW_HEIGHT = 0.05;

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(rows); i++)
        {
            if ((i == STAT_GAP1) ||
                (i == STAT_GAP2)) {
                continue;
            }

            widp w = wid_new_square_button(wid_player_info_container,
                                           rows[i].col1);

            fpoint tl = {0.05, 0.2};
            fpoint br = {0.48, 0.2};

            br.y += (double)i * ROW_HEIGHT;
            tl.y += (double)i * ROW_HEIGHT;
            br.y += ROW_HEIGHT;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, rows[i].col1);
            wid_set_font(w, small_font);

            wid_set_color(w, WID_COLOR_TEXT, WHITE);

            switch (i) {
            case STAT_EXPERIENCE:
                break;
            case STAT_LEVEL:
                break;
            case STAT_SPENDING_POINTS:
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

            wid_set_on_mouse_down(w, wid_player_info_col1_name_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);

            wid_set_tex(w, 0, "button_black");

            if (s->spending_points > 0) {
                if (i < STAT_GAP2) {
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

            if ((i == STAT_GAP1) ||
                (i == STAT_GAP2)) {
                continue;
            }

            widp w = wid_new_square_button(wid_player_info_container,
                                           rows[i].col2);

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

            wid_set_on_mouse_down(w, wid_player_info_col2_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);

            wid_set_tex(w, 0, "button_black");

            if (s->spending_points > 0) {
                if (i < STAT_GAP2) {
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

            if ((i == STAT_GAP1) ||
                (i == STAT_GAP2)) {
                continue;
            }

            widp w = wid_new_square_button(wid_player_info_container,
                                           rows[i].col1);

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
                text = dynprintf("%u (%u)", s->hp, s->max_hp);
                break;
            case STAT_ATTACK_MELEE:
                stat = s->attack_melee;
                break;
            case STAT_ATTACK_RANGED:
                stat = s->attack_ranged;
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

            if (!text) {
                int modifier = player_stats_get_modifier(stat);

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
        const char *button_name = "All Done";

        widp w = wid_new_rounded_small_button(wid_player_info_container,
                                              button_name);

        fpoint tl = {0.7, 0.85};
        fpoint br = {0.95, 0.90};

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

        wid_set_on_mouse_down(w, wid_player_info_all_done_mouse_event);
        wid_set_on_key_down(w, wid_player_info_all_done_key_event);

        wid_set_tex(w, 0, "button_black");
        wid_set_square(w);
    }

    {
        const char *button_name = "Re-roll";

        widp w = wid_new_rounded_small_button(wid_player_info_container,
                                              button_name);

        fpoint tl = {0.1, 0.85};
        fpoint br = {0.5, 0.90};

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

        wid_set_on_mouse_down(w, wid_player_info_reroll_mouse_event);
        wid_set_on_key_down(w, wid_player_info_reroll_key_event);

        wid_set_tex(w, 0, "button_black");
        wid_set_square(w);
    }

    wid_raise(wid_player_info);
    wid_update(wid_player_info);
}

static void wid_player_info_destroy (void)
{
    wid_destroy(&wid_player_info);
}