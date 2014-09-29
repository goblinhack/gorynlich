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

static void wid_player_stats_create(player_stats_t);
static void wid_player_stats_destroy(void);
static player_stats_t player_stats;

#define WID_INTRO_MAX_SETTINGS  5
#define WID_INTRO_MAX_VAL      30 

enum {
    WID_INTRO_SETTINGS_ROW_MELEE,
    WID_INTRO_SETTINGS_ROW_RANGED,
    WID_INTRO_SETTINGS_ROW_SPEED,
    WID_INTRO_SETTINGS_ROW_VISION,
    WID_INTRO_SETTINGS_ROW_HEALING,
};

static const char *wid_player_stats_col1[WID_INTRO_MAX_SETTINGS] = {
    "Melee",
    "Ranged",
    "Speed",
    "Vision",
    "Healing",
};

static const char *wid_player_stats_col2[WID_INTRO_MAX_SETTINGS] = {
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

void wid_player_stats_visible (player_stats_t t)
{
    wid_player_stats_create(t);
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
    return (true);
}

static uint8_t wid_player_stats_col2_mouse_event (widp w,
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

static uint8_t wid_player_stats_receive_mouse_motion (
                    widp w,
                    int32_t x, int32_t y,
                    int32_t relx, int32_t rely,
                    int32_t wheelx, int32_t wheely)
{
    if (wheelx || wheely) {
        /*
         * Allow scrolling.
         */
        return (false);
    }

    /*
     * Block moving the window.
     */
    return (true);
}

static void wid_player_stats_create (player_stats_t s)
{
    player_stats = s;

    if (!wid_player_stats) {
        widp w = wid_player_stats = wid_new_rounded_window("wid player_stats");

        fpoint tl = {0.1, 0.2};
        fpoint br = {0.9, 0.8};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_font(w, med_font);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);

        color c = BLACK;
        c.a = 200;
        wid_set_color(w, WID_COLOR_BG, c);

        c = STEELBLUE;
        c.a = 150;
        wid_set_color(w, WID_COLOR_TL, c);
        wid_set_color(w, WID_COLOR_BR, c);
        wid_set_bevel(w, 4);

        wid_set_on_mouse_motion(w, wid_player_stats_receive_mouse_motion);
    }

    {
        widp w = wid_player_stats_container =
            wid_new_container(wid_player_stats, "wid player_stats container");

        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(w, tl, br);
    }

    {
        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 0.2};

        widp w = wid_new_container(wid_player_stats, "wid player_stats title");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Settings");
        wid_set_font(w, large_font);
        wid_set_color(w, WID_COLOR_TEXT, STEELBLUE);

        wid_set_text_outline(w, true);
    }

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(wid_player_stats_col1); i++)
        {
            widp w = wid_new_square_button(wid_player_stats_container,
                                           wid_player_stats_col1[i]);

            fpoint tl = {0.05, 0.2};
            fpoint br = {0.48, 0.3};

            double height = 0.12;

            br.y += (double)i * height;
            tl.y += (double)i * height;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, wid_player_stats_col1[i]);
            wid_set_font(w, med_font);

            color c = BLACK;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_on_mouse_up(w, wid_player_stats_col1_name_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);
            wid_set_bevel(w,0);
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

            fpoint tl = {0.49, 0.2};
            fpoint br = {0.595, 0.3};

            double height = 0.12;

            br.y += (double)i * height;
            tl.y += (double)i * height;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, wid_player_stats_col2[i]);
            wid_set_font(w, med_font);

            color c = LIME;

            c.a = 150;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            c.a = 250;
            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_on_mouse_up(w, wid_player_stats_col2_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);
            wid_set_bevel(w,0);
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

            fpoint tl = {0.72, 0.2};
            fpoint br = {0.95, 0.3};

            double height = 0.12;

            br.y += (double)i * height;
            tl.y += (double)i * height;

            wid_set_tl_br_pct(w, tl, br);

            if (wid_player_stats_col3[i][wid_player_stats_val[i]]) {
                wid_set_text(w,
                            wid_player_stats_col3[i][wid_player_stats_val[i]]);
            } else {
                char *text;

                text = dynprintf("%u", 0);

                switch (i) {
                case WID_INTRO_SETTINGS_ROW_MELEE:
                    wid_set_text(w, text);
                    break;
                case WID_INTRO_SETTINGS_ROW_RANGED:
                    wid_set_text(w, text);
                    break;
                case WID_INTRO_SETTINGS_ROW_SPEED:
                    wid_set_text(w, text);
                    break;
                case WID_INTRO_SETTINGS_ROW_HEALING:
                    wid_set_text(w, text);
                    break;
                }

                myfree(text);
            }
            wid_set_font(w, med_font);

            color c = BLUE;

            c.a = 100;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_on_mouse_up(w, wid_player_stats_col3_mouse_event);
            wid_set_client_context(w, (void*)(uintptr_t)i);
            wid_set_bevel(w,0);
        }
    }

    {
        const char *button_name = "All Done";

        widp w = wid_new_rounded_small_button(wid_player_stats_container,
                                              button_name);

        fpoint tl = {0.7, 0.85};
        fpoint br = {0.95, 0.98};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_text(w, button_name);
        wid_set_font(w, med_font);

        color c = STEELBLUE;

        c.a = 100;
        wid_set_mode(w, WID_MODE_NORMAL);
        wid_set_color(w, WID_COLOR_BG, c);

        c.a = 250;
        wid_set_mode(w, WID_MODE_OVER);
        wid_set_color(w, WID_COLOR_BG, c);

        wid_set_mode(w, WID_MODE_FOCUS);

        wid_set_mode(w, WID_MODE_NORMAL);

        wid_set_on_mouse_up(w, wid_player_stats_all_done_mouse_event);
        wid_set_on_key_down(w, wid_player_stats_all_done_key_event);
    }

    wid_raise(wid_player_stats);

    wid_update(wid_player_stats);
}

static void wid_player_stats_destroy (void)
{
    wid_destroy(&wid_player_stats);
}
