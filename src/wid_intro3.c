/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "wid_intro3.h"
#include "wid_intro.h"
#include "wid_server_join.h"
#include "wid_server_create.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"
#include "level.h"
#include "timer.h"
#include "glapi.h"

static widp wid_intro3;
static widp wid_intro3_background;

static void wid_intro3_play_selected(void);

static uint8_t wid_intro3_init_done;
static void wid_intro3_create(void);

static int intro_effect_delay = 200;

uint8_t wid_intro3_init (void)
{
    if (!wid_intro3_init_done) {
    }

    wid_intro3_init_done = true;

    return (true);
}

void wid_intro3_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_intro3_init_done) {
        wid_intro3_init_done = false;

        if (wid_intro3) {
            wid_destroy(&wid_intro3);
            wid_destroy_in(wid_intro3_background, wid_hide_delay * 2);
        }
    }
}

static uint8_t wid_intro3_is_hidden;
static uint8_t wid_intro3_is_visible;

void wid_intro3_hide (void)
{
    if (wid_intro3_is_hidden) {
        return;
    }

    wid_intro3_is_hidden = true;
    wid_intro3_is_visible = false;

    if (!wid_intro3) {
        DIE("no wid intro");
    }

    wid_fade_out(wid_intro3_background, intro_effect_delay);

    wid_hide(wid_intro3, 0);
    wid_raise(wid_intro3);
    wid_update(wid_intro3);
}

void wid_intro3_visible (void)
{
    if (wid_intro3_is_visible) {
        return;
    }

    wid_intro3_create();

    wid_intro3_is_visible = true;
    wid_intro3_is_hidden = false;

    if (!wid_intro3) {
        DIE("no wid intro");
    }

    if (game_over) {
        game_over = false;
        return;
    }

    wid_visible(wid_intro3, 0);
    wid_raise(wid_intro3);
    wid_update(wid_intro3);

    wid_fade_in(wid_intro3_background, intro_effect_delay);
}

static void wid_server_join_selected (void)
{
    wid_server_join_visible();
}

static void wid_server_create_selected (void)
{
    wid_server_create_visible();
}

static void wid_intro3_play_selected_cb (void *context)
{
    wid_server_join_hide();
    wid_server_create_hide();

    wid_game_map_server_visible();
    wid_game_map_client_visible();

    wid_intro_hide();
}

static void wid_intro3_multi_play_selected_cb (void *context)
{
    wid_server_create_selected();
    wid_server_join_selected();
}

static void wid_intro3_multi_play_selected (void)
{
    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_intro3_multi_play_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start game",
            intro_effect_delay,
            0 /* jitter */);

    wid_intro3_hide();
}

static void wid_intro3_play_selected (void)
{
    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_intro3_play_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start game",
            intro_effect_delay,
            0 /* jitter */);

    wid_intro3_hide();
}

static uint8_t wid_intro3_play_mouse_event (widp w, int32_t x, int32_t y,
                                           uint32_t button)
{
    wid_intro3_play_selected();

    return (true);
}

static uint8_t wid_intro3_multi_play_mouse_event (widp w, int32_t x, int32_t y,
                                           uint32_t button)
{
    wid_intro3_multi_play_selected();

    return (true);
}

static uint8_t wid_intro3_go_back_mouse_event (widp w, int32_t x, int32_t y,
                                               uint32_t button)
{
    wid_intro3_hide();
    wid_intro_visible();

    return (true);
}

static uint8_t wid_intro3_play_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case ' ':
        case 's':
            wid_intro3_play_selected();
            return (true);

        case 'm':
            wid_intro3_multi_play_selected();
            return (true);

        case SDLK_ESCAPE:
            wid_intro3_hide();
            wid_intro_visible();
            return (true);

        default:
            break;
    }

    return (false);
}

static uint8_t wid_intro3_multi_play_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case ' ':
        case 'm':
            wid_intro3_multi_play_selected();
            return (true);

        case 's':
            wid_intro3_play_selected();
            return (true);

        case SDLK_ESCAPE:
            wid_intro3_hide();
            wid_intro_visible();
            return (true);

        default:
            break;
    }

    return (false);
}

static void wid_intro3_bg_create (void)
{
    widp wid;

    if (wid_intro3_background) {
        return;
    }

    {
        wid = wid_intro3_background = wid_new_window("bg");

        float f;

        f = (1024.0 / 680.0);

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, f };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title3");

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

static void wid_intro3_create (void)
{
    if (wid_intro3) {
        return;
    }

    wid_intro3 = wid_new_window("intro buttons");

    wid_set_no_shape(wid_intro3);

    fpoint tl = {0.0f, 0.0f};
    fpoint br = {1.0f, 1.0f};
    wid_set_tl_br_pct(wid_intro3, tl, br);
    wid_set_on_mouse_down(wid_intro3, wid_intro3_play_mouse_event);
    wid_set_on_key_down(wid_intro3, wid_intro3_play_key_event);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(wid_intro3, WID_MODE_NORMAL);
    wid_set_color(wid_intro3, WID_COLOR_TL, col);
    wid_set_color(wid_intro3, WID_COLOR_BR, col);
    wid_set_color(wid_intro3, WID_COLOR_BG, col);

    {
        widp child;

        child = wid_new_square_button(wid_intro3, "play");
        wid_set_font(child, large_font);
        wid_set_no_shape(child);

        fpoint tl = {0.1f, 0.00f};
        fpoint br = {0.5f, 1.00f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "start game");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = ORANGE;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c = RED;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);
        wid_set_text_outline(child, true);

        wid_set_on_mouse_down(child, wid_intro3_play_mouse_event);
        wid_set_on_key_down(child, wid_intro3_play_key_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro3, "play");
        wid_set_font(child, large_font);
        wid_set_no_shape(child);

        fpoint tl = {0.5f, 0.00f};
        fpoint br = {1.0f, 1.00f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "join game");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = ORANGE;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c = RED;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);
        wid_set_text_outline(child, true);

        wid_set_on_mouse_down(child, wid_intro3_multi_play_mouse_event);
        wid_set_on_key_down(child, wid_intro3_multi_play_key_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro3, "Go back");
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

        wid_set_on_mouse_down(child, wid_intro3_go_back_mouse_event);
        wid_raise(child);
        wid_set_do_not_lower(child, true);
    }

    wid_intro3_bg_create();
    wid_update(wid_intro3);

    wid_move_to_pct_centered(wid_intro3, 0.5f, 0.5f);
}
