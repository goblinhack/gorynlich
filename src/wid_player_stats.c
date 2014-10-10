/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_popup.h"
#include "wid_player_stats.h"
#include "string.h"
#include "music.h"

static widp wid_player_stats;
static widp wid_player_stats_container;
static uint8_t wid_player_stats_init_done;

static void wid_player_stats_create(player_stats_t *);
static void wid_player_stats_destroy(void);
static player_stats_t *player_stats;

#define WID_INTRO_MAX_SETTINGS 10 
#define WID_INTRO_MAX_VAL      30 

enum {
    WID_INTRO_SETTINGS_ROW_EXPERIENCE,
    WID_INTRO_SETTINGS_ROW_LEVEL,
    WID_INTRO_SETTINGS_ROW_SPENDING_POINTS,
    WID_INTRO_SETTINGS_ROW_MAX_HEALTH,
    WID_INTRO_SETTINGS_ROW_MELEE,
    WID_INTRO_SETTINGS_ROW_RANGED,
    WID_INTRO_SETTINGS_ROW_DEFENSE,
    WID_INTRO_SETTINGS_ROW_SPEED,
    WID_INTRO_SETTINGS_ROW_VISION,
    WID_INTRO_SETTINGS_ROW_HEALING,
};

static const char *wid_player_stats_col1[WID_INTRO_MAX_SETTINGS] = {
    "Experience",
    "Level",
    "Spending points",
    "Max Health",
    "Attack, Melee",
    "Attack, Ranged",
    "Defense",
    "Speed",
    "Vision",
    "Healing",
};

static const char *wid_player_stats_col2[WID_INTRO_MAX_SETTINGS] = {
    0,
    0,
    0,
    "+",
    "+",
    "+",
    "+",
    "+",
    "+",
    "+",
};

static const char *wid_player_stats_col3
                        [WID_INTRO_MAX_SETTINGS][WID_INTRO_MAX_VAL] = {
};

static uint32_t wid_player_stats_val[WID_INTRO_MAX_SETTINGS];

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
}

void wid_player_stats_visible (player_stats_t *s)
{
    wid_player_stats_create(s);
}

static uint8_t wid_player_stats_all_done_mouse_event (widp w, int32_t x, int32_t y,
                                                      uint32_t button)
{
    wid_player_stats_hide();

    return (true);
}

static uint8_t wid_player_stats_col1_name_mouse_event (widp w,
                                                       int32_t x, int32_t y,
                                                       uint32_t button)
{
    /*
     * Increment.
     */
    int32_t row = (typeof(row)) (intptr_t) wid_get_client_context(w);
    int32_t val = wid_player_stats_val[row];

    if (!wid_player_stats_col3[row][val+1]) {
        return (true);
    }

    wid_player_stats_val[row]++;

    wid_destroy_nodelay(&wid_player_stats_container);
    wid_player_stats_create(player_stats);

    return (true);
}

static uint8_t wid_player_stats_col2_mouse_event (widp w,
                                                  int32_t x, int32_t y,
                                                  uint32_t button)
{
    wid_player_stats_col1_name_mouse_event(w, x, y, button);

    return (true);
}

static uint8_t wid_player_stats_col3_mouse_event (widp w,
                                                  int32_t x, int32_t y,
                                                  uint32_t button)
{
    return (true);
}

static uint8_t wid_player_stats_all_done_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'q':
        case SDLK_ESCAPE:
            wid_player_stats_hide();
            return (true);

        default:
            break;
    }

    return (false);
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

        for (i=0; i<ARRAY_SIZE(wid_player_stats_col1); i++)
        {
            widp w = wid_new_square_button(wid_player_stats_container,
                                           wid_player_stats_col1[i]);

            fpoint tl = {0.05, 0.2};
            fpoint br = {0.48, 0.2};

            br.y += (double)i * ROW_HEIGHT;
            tl.y += (double)i * ROW_HEIGHT;
            br.y += ROW_HEIGHT;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, wid_player_stats_col1[i]);
            wid_set_font(w, small_font);

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
            wid_set_square(w);
        }
    }

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(wid_player_stats_col1); i++) {

            if (!wid_player_stats_col2[i]) {
                continue;
            }

            widp w = wid_new_square_button(wid_player_stats_container,
                                           wid_player_stats_col2[i]);

            fpoint tl = {0.48, 0.2};
            fpoint br = {0.595, 0.2};

            br.y += (double)i * ROW_HEIGHT;
            tl.y += (double)i * ROW_HEIGHT;
            br.y += ROW_HEIGHT;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, wid_player_stats_col2[i]);
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
            wid_set_square(w);

        }
    }

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(wid_player_stats_col1); i++) {

            if (!wid_player_stats_col1[i]) {
                continue;
            }

            widp w = wid_new_square_button(wid_player_stats_container,
                                           wid_player_stats_col1[i]);

            fpoint tl = {0.82, 0.2};
            fpoint br = {0.95, 0.2};

            br.y += (double)i * ROW_HEIGHT;
            tl.y += (double)i * ROW_HEIGHT;
            br.y += ROW_HEIGHT;

            wid_set_tl_br_pct(w, tl, br);

            if (wid_player_stats_col3[i][wid_player_stats_val[i]]) {
                wid_set_text(w,
                            wid_player_stats_col3[i][wid_player_stats_val[i]]);
            } else {
                char *text = 0;
                int stat;

                switch (i) {
                case WID_INTRO_SETTINGS_ROW_EXPERIENCE:
                    stat = s->experience;
                    text = dynprintf("%u", stat);
                    break;
                case WID_INTRO_SETTINGS_ROW_LEVEL:
                    stat = s->experience / 1000;
                    text = dynprintf("%u", stat);
                    break;
                case WID_INTRO_SETTINGS_ROW_SPENDING_POINTS:
                    stat = s->spending_points;
                    text = dynprintf("%u", stat);
                    break;
                case WID_INTRO_SETTINGS_ROW_MAX_HEALTH:
                    text = dynprintf("%u", s->hp, s->max_hp);
                    break;
                case WID_INTRO_SETTINGS_ROW_MELEE:
                    stat = s->attack_melee;
                    break;
                case WID_INTRO_SETTINGS_ROW_RANGED:
                    stat = s->attack_ranged;
                    break;
                case WID_INTRO_SETTINGS_ROW_DEFENSE:
                    stat = s->defense;
                    break;
                case WID_INTRO_SETTINGS_ROW_SPEED:
                    stat = s->speed;
                    break;
                case WID_INTRO_SETTINGS_ROW_VISION:
                    stat = s->vision;
                    break;
                case WID_INTRO_SETTINGS_ROW_HEALING:
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
            }
            wid_set_font(w, small_font);

            color c = DARKGRAY;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_bevel(w, 2);
            wid_set_no_shape(w);

            wid_set_on_mouse_down(w, wid_player_stats_col3_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);
        }
    }

    {
        const char *button_name = "All Done";

        widp w = wid_new_rounded_small_button(wid_player_stats_container,
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

        wid_set_on_mouse_down(w, wid_player_stats_all_done_mouse_event);
        wid_set_on_key_down(w, wid_player_stats_all_done_key_event);

        wid_set_tex(w, 0, "button_black");
        wid_set_square(w);
    }

    wid_raise(wid_player_stats);

    wid_update(wid_player_stats);

    wid_set_tex(wid_player_stats, 0, "gothic_tall_axe");
    wid_set_square(wid_player_stats);

    wid_move_to_pct_centered_in(wid_player_stats, 0.15, 0.45, wid_swipe_delay);
    wid_raise(wid_player_stats);
}

static void wid_player_stats_destroy (void)
{
    wid_destroy(&wid_player_stats);
}
