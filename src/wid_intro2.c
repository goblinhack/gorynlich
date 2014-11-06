/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "wid_intro.h"
#include "wid_intro2.h"
#include "wid_intro3.h"
#include "level.h"
#include "timer.h"
#include "time.h"
#include "client.h"
#include "thing.h"
#include "wid_notify.h"
#include "wid_player_stats.h"
#include "wid_player_info.h"
#include "wid_player_inventory.h"
#include "wid_player_action.h"
#include "name.h"
#include "glapi.h"

static widp wid_intro2;
static widp wid_intro2_background;

static void wid_intro2_play_selected(void);

static uint8_t wid_intro2_init_done;
static void wid_intro2_create(void);

static int intro_effect_delay = 200;

uint8_t wid_intro2_init (void)
{
    if (!wid_intro2_init_done) {
    }

    wid_intro2_init_done = true;

    return (true);
}

void wid_intro2_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_intro2_init_done) {
        wid_intro2_init_done = false;

        if (wid_intro2) {
            wid_destroy(&wid_intro2);
            wid_destroy_in(wid_intro2_background, wid_hide_delay * 2);
        }
    }
}

static uint8_t wid_intro2_is_hidden;
static uint8_t wid_intro2_is_visible;

void wid_intro2_hide (void)
{
    if (wid_intro2_is_hidden) {
        return;
    }

    wid_intro2_is_hidden = true;
    wid_intro2_is_visible = false;

    if (!wid_intro2) {
        DIE("no wid intro");
    }

    wid_fade_out(wid_intro2_background, intro_effect_delay);

    wid_hide(wid_intro2, 0);
    wid_raise(wid_intro2);
    wid_update(wid_intro2);
    
    wid_player_stats_hide();
    wid_player_info_hide();
    wid_player_inventory_hide();
    wid_player_action_hide();
}

void wid_intro2_visible (void)
{
    if (wid_intro2_is_visible) {
        return;
    }

    wid_notify_flush();

    wid_intro2_create();

    wid_intro2_is_visible = true;
    wid_intro2_is_hidden = false;

    if (!wid_intro2) {
        DIE("no wid intro");
    }

    if (game_over) {
        game_over = false;
        return;
    }

    wid_visible(wid_intro2, 0);
    wid_raise(wid_intro2);
    wid_update(wid_intro2);

    wid_fade_in(wid_intro2_background, intro_effect_delay);

    /*
     * Start with some random junk.
     */
    thing_statsp s;
    s = &global_config.player_stats;
    thing_stats_get_random(s);

    wid_player_stats_visible(s);
    wid_player_info_visible(s);
    wid_player_inventory_visible(s);
    wid_player_action_visible(s, false);
}

static void wid_intro2_play_selected_cb (void *context)
{
    wid_intro3_visible();
}

static void wid_intro2_play_selected (void)
{
    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_intro2_play_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start game",
            intro_effect_delay,
            0 /* jitter */);

    wid_intro2_hide();
}

static uint8_t wid_intro2_play_mouse_event (widp w, int32_t x, int32_t y,
                                           uint32_t button)
{
    wid_intro2_play_selected();

    return (true);
}

static void wid_intro_intro_all_done_selected (void)
{
    wid_intro2_play_selected();
}

static uint8_t wid_intro_go_back_selected (void)
{
    wid_intro2_hide();
    wid_intro_visible();

    return (true);
}

static uint8_t wid_intro2_go_back_mouse_event (widp w, int32_t x, int32_t y,
                                               uint32_t button)
{
    wid_intro_go_back_selected();

    return (true);
}

static uint8_t wid_intro2_play_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case ' ':
            wid_intro2_play_selected();
            return (true);

        case 'b':
        case 'q':
        case SDLK_ESCAPE:
            wid_intro_go_back_selected();
            return (true);

        case 'a':
            wid_intro_intro_all_done_selected();
            return (true);

        default:
            break;
    }

    return (false);
}

static void wid_intro2_bg_create (void)
{
    widp wid;

    if (wid_intro2_background) {
        return;
    }

    {
        wid = wid_intro2_background = wid_new_window("bg");

        float f;

        f = (1024.0 / 680.0);

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, f };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title2");

        wid_lower(wid);

        color c;
        c = WHITE;
        wid_set_mode(wid, WID_MODE_NORMAL);
        wid_set_color(wid, WID_COLOR_TL, c);
        wid_set_color(wid, WID_COLOR_BR, c);
        wid_set_color(wid, WID_COLOR_BG, c);

        wid_update(wid);
    }
}

static uint8_t wid_intro2_intro_all_done_mouse_event (widp w, 
                                                      int32_t x, int32_t y,
                                                      uint32_t button)
{
    wid_intro_intro_all_done_selected();

    return (true);
}

static void wid_intro2_create (void)
{
    if (wid_intro2) {
        return;
    }

    wid_intro2 = wid_new_window("intro buttons");

    wid_set_no_shape(wid_intro2);

    fpoint tl = {0.0f, 0.0f};
    fpoint br = {1.0f, 1.0f};
    wid_set_tl_br_pct(wid_intro2, tl, br);
    wid_set_on_mouse_down(wid_intro2, wid_intro2_play_mouse_event);
    wid_set_on_key_down(wid_intro2, wid_intro2_play_key_event);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(wid_intro2, WID_MODE_NORMAL);
    wid_set_color(wid_intro2, WID_COLOR_TL, col);
    wid_set_color(wid_intro2, WID_COLOR_BR, col);
    wid_set_color(wid_intro2, WID_COLOR_BG, col);
    wid_set_on_mouse_down(wid_intro2, wid_intro2_play_mouse_event);
    wid_set_on_key_down(wid_intro2, wid_intro2_play_key_event);

    {
        widp child;

        child = wid_new_square_button(wid_intro2, "All done");
        wid_set_font(child, small_font);

        fpoint tl = {0.0f, 0.95f};
        fpoint br = {0.3f, 1.00f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "%%fmt=left$%%tile=button_a$All done");

        wid_set_no_shape(child);
        wid_set_color(child, WID_COLOR_TEXT, GRAY90);
        wid_set_mode(child, WID_MODE_OVER);
        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro2_intro_all_done_mouse_event);
        wid_raise(child);
        wid_set_do_not_lower(child, true);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro2, "Go back");
        wid_set_font(child, small_font);

        fpoint tl = {0.9f, 0.95f};
        fpoint br = {1.0f, 1.00f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "%%fmt=left$%%tile=button_b$Go back");

        wid_set_no_shape(child);
        wid_set_color(child, WID_COLOR_TEXT, GRAY90);
        wid_set_mode(child, WID_MODE_OVER);
        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro2_go_back_mouse_event);
        wid_raise(child);
        wid_set_do_not_lower(child, true);
    }

    wid_intro2_bg_create();
    wid_update(wid_intro2);

    wid_move_to_pct_centered(wid_intro2, 0.5f, 0.5f);
}
