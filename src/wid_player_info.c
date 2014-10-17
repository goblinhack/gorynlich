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
#include "wid_player_inventory.h"
#include "string.h"
#include "thing_template.h"
#include "time.h"
#include "client.h"
#include "name.h"

static widp wid_player_info;
static uint8_t wid_player_info_init_done;
int wid_player_info_set_name;

static void wid_player_info_create(player_stats_t *);
static void wid_player_info_destroy(void);
static player_stats_t *player_stats;

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

static void wid_player_info_buttons_tick (widp wid)
{
    int tick = time_get_time_milli() / 100;
    int which = tick % 8;

    const char *tn = player_stats->pclass;

    char tilename[40];

    switch (which) {
    case 0: snprintf(tilename, sizeof(tilename) - 1, "%s-demo-right", tn); break;
    case 1: snprintf(tilename, sizeof(tilename) - 1, "%s-demo-br", tn); break;
    case 2: snprintf(tilename, sizeof(tilename) - 1, "%s-demo-down", tn); break;
    case 3: snprintf(tilename, sizeof(tilename) - 1, "%s-demo-bl", tn); break;
    case 4: snprintf(tilename, sizeof(tilename) - 1, "%s-demo-left", tn); break;
    case 5: snprintf(tilename, sizeof(tilename) - 1, "%s-demo-tl", tn); break;
    case 6: snprintf(tilename, sizeof(tilename) - 1, "%s-demo-up", tn); break;
    case 7: snprintf(tilename, sizeof(tilename) - 1, "%s-demo-tr", tn); break;
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

            wid_player_info_set_name = true;
            break;
        }

        default:
            break;
    }

    /*
     * Feed to the general input handler
     */
    r = (wid_receive_input(w, key));

    name = (char*) wid_get_text(w);

    wid_player_info_set_name = true;

    if (!strlen(name)) {
        wid_player_info_set_name = false;

        strncpy(player_stats->pname, name_random(player_stats->pclass),
                sizeof(player_stats->pname) - 1);
    }

    return (r);
}

static uint8_t wid_player_info_select_name_event (widp w, int32_t x, int32_t y,
                                             uint32_t button)
{
    wid_set_on_key_down(w, wid_player_info_name_receive_input);

    wid_set_show_cursor(w, true);

    return (true);
}

static void wid_player_info_create (player_stats_t *s)
{
    player_stats = s;

    if (!wid_player_info) {
        widp w = wid_player_info = 
                        wid_new_rounded_window("wid player_stats");

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
        widp w = 
            wid_new_square_button(wid_player_info, 
                                  "wid player_stats player");

        fpoint tl = {0.3, 0.2};
        fpoint br = {0.7, 0.4};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_on_tick(w, wid_player_info_buttons_tick);

        wid_set_no_shape(w);
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
        wid_set_color(w, WID_COLOR_TL, STEELBLUE);
        wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        wid_set_text_outline(w, true);
        wid_raise(w);
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
        wid_set_color(w, WID_COLOR_TL, STEELBLUE);
        wid_set_color(w, WID_COLOR_BR, STEELBLUE);
        wid_set_square(w);
        wid_set_bevelled(w, true);
        wid_set_bevel(w, 2);
        wid_set_text_outline(w, true);
        wid_set_on_mouse_down(w, wid_player_info_select_name_event);
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

        fpoint tl = {0.3, 0.55};
        fpoint br = {0.45, 0.70};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_tooltip(w, "Your helmet");

        int id = s->worn[THING_WORN_ARMOR];

        wid_player_inventory_button_style(w, s, id);

//        wid_set_on_mouse_down(w, wid_intro_settings_col4_mouse_event);
    }

    {
        widp w = wid_new_square_button(wid_player_info, 
                                       "wid intro item armor");

        fpoint tl = {0.8, 0.55};
        fpoint br = {0.95, 0.70};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_tooltip(w, "Your armor");

        int id = s->worn[THING_WORN_HELMET];

        wid_player_inventory_button_style(w, s, id);

//        wid_set_on_mouse_down(w, wid_intro_settings_col4_mouse_event);
    }

    {
        widp w = wid_new_square_button(wid_player_info, 
                                       "wid intro item boots");

        fpoint tl = {0.3, 0.71};
        fpoint br = {0.45, 0.86};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_tooltip(w, "Your boots");

        int id = s->worn[THING_WORN_BOOTS];

        wid_player_inventory_button_style(w, s, id);

//        wid_set_on_mouse_down(w, wid_intro_settings_col4_mouse_event);
    }

    {
        widp w = wid_new_square_button(wid_player_info, 
                                       "wid intro item left arm");

        fpoint tl = {0.8, 0.71};
        fpoint br = {0.95, 0.79};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_tooltip(w, "Left hand");

        int id = s->worn[THING_WORN_ARM_LEFT];

        wid_player_inventory_button_style(w, s, id);

//        wid_set_on_mouse_down(w, wid_intro_settings_col4_mouse_event);
    }

    {
        widp w = wid_new_square_button(wid_player_info, 
                                       "wid intro item right arm");

        fpoint tl = {0.8, 0.80};
        fpoint br = {0.95, 0.88};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_tooltip(w, "Right hand");

        int id = s->worn[THING_WORN_ARM_RIGHT];

        wid_player_inventory_button_style(w, s, id);

//        wid_set_on_mouse_down(w, wid_intro_settings_col4_mouse_event);
//
    }

    wid_move_to_pct_centered(wid_player_info, 0.5, 0.45);

    wid_raise(wid_player_info);
    wid_update(wid_player_info);
}

static void wid_player_info_destroy (void)
{
    wid_destroy(&wid_player_info);
}
