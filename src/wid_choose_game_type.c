/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "wid.h"
#include "wid_choose_game_type.h"
#include "wid_choose_stats.h"
#include "wid_choose_name.h"
#include "wid_intro.h"
#include "wid_server_join.h"
#include "wid_server_create.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"
#include "timer.h"
#include "glapi.h"
#include "server.h"
#include "wid_menu.h"

static widp wid_choose_game_type;
static widp wid_choose_game_type_background;

static void wid_choose_game_type_play_selected(void);

static uint8_t wid_choose_game_type_init_done;
static void wid_choose_game_type_create(void);
static widp menu;

static int intro_effect_delay = 200;

uint8_t wid_choose_game_type_init (void)
{
    if (!wid_choose_game_type_init_done) {
    }

    wid_choose_game_type_init_done = true;

    return (true);
}

void wid_choose_game_type_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_choose_game_type_init_done) {
        wid_choose_game_type_init_done = false;

        if (wid_choose_game_type) {
            wid_destroy(&wid_choose_game_type);
            wid_destroy_in(wid_choose_game_type_background, wid_hide_delay * 2);
            wid_choose_game_type_background = 0;
        }

        if (menu) {
            wid_destroy(&menu);
        }
    }
}

static uint8_t wid_choose_game_type_is_hidden;
static uint8_t wid_choose_game_type_is_visible;

void wid_choose_game_type_hide (void)
{
    if (wid_choose_game_type_is_hidden) {
        return;
    }

    wid_choose_game_type_is_hidden = true;
    wid_choose_game_type_is_visible = false;

    if (!wid_choose_game_type) {
        ERR("no wid intro");
    }

    wid_fade_out(wid_choose_game_type_background, intro_effect_delay);

    wid_hide(wid_choose_game_type, 0);
    wid_raise(wid_choose_game_type);
    wid_update(wid_choose_game_type);

    wid_destroy(&wid_choose_game_type);
    wid_destroy_in(wid_choose_game_type_background, wid_hide_delay * 2);
    wid_choose_game_type_background = 0;

    if (menu) {
        wid_destroy(&menu);
    }
}

void wid_choose_game_type_visible (void)
{
    if (wid_choose_game_type_is_visible) {
        wid_choose_game_type_hide();
    }

    wid_choose_game_type_create();

    wid_choose_game_type_is_visible = true;
    wid_choose_game_type_is_hidden = false;

    if (!wid_choose_game_type) {
        ERR("no wid intro");
    }

    if (global_config.game_over) {
        global_config.game_over = false;
        return;
    }

    wid_visible(wid_choose_game_type, 0);
    wid_raise(wid_choose_game_type);
    wid_update(wid_choose_game_type);

    wid_fade_in(wid_choose_game_type_background, intro_effect_delay);
}

static void wid_server_join_selected (void)
{
    LOG("Server join selected");

    wid_choose_game_type_hide();
    wid_server_join_visible();
}

static void wid_server_create_selected (void)
{
    LOG("Server create selected");

    wid_server_create_visible();
}

static void wid_choose_player_play_selected_cb (void *context)
{
    wid_game_map_server_visible();
    wid_game_map_client_visible();

    wid_intro_hide();
}

static void wid_choose_player_play_selected (void)
{
    LOG("Play from stats selection");

    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_choose_player_play_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start game",
            intro_effect_delay,
            0 /* jitter */);

    wid_choose_game_type_hide();
}

static void wid_choose_game_type_single_play_selected_cb (void *context)
{
    single_player_mode = 1;

    wid_server_join_hide();
    wid_server_create_hide();

    wid_choose_player_play_selected();
}

static void wid_choose_game_type_start_server_selected_cb (void *context)
{
    single_player_mode = 0;

    wid_server_create_selected();
}

static void wid_choose_game_type_start_server_selected (void)
{
    LOG("Start server selected");

    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_choose_game_type_start_server_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start server",
            intro_effect_delay,
            0 /* jitter */);

    wid_destroy(&menu);
}

static void wid_choose_game_type_stop_server_selected_cb (void *context)
{
    wid_choose_game_type_hide();
    server_stop();
    wid_choose_game_type_visible();
}

static void wid_choose_game_type_stop_server_selected (void)
{
    LOG("Stop server selected");

    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_choose_game_type_stop_server_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "stop game",
            intro_effect_delay,
            0 /* jitter */);

    wid_choose_game_type_hide();
}

static void wid_choose_game_type_join_game_selected_cb (void *context)
{
    single_player_mode = 0;

    wid_server_join_selected();
}

static void wid_choose_game_type_join_game_selected (void)
{
    LOG("Join game selected");

    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_choose_game_type_join_game_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "Join game",
            intro_effect_delay,
            0 /* jitter */);

    wid_choose_game_type_hide();
}

static void wid_choose_game_type_play_selected (void)
{
    LOG("Single player selected");

    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_choose_game_type_single_play_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start game",
            intro_effect_delay,
            0 /* jitter */);

    wid_choose_game_type_hide();
}

static uint8_t wid_choose_game_type_play_mouse_event (widp w, int32_t x, int32_t y,
                                           uint32_t button)
{
    wid_choose_game_type_play_selected();

    return (true);
}

static uint8_t 
wid_choose_game_type_start_server_mouse_event (widp w, int32_t x, int32_t y,
                                               uint32_t button)
{
    wid_choose_game_type_start_server_selected();

    return (true);
}

static uint8_t 
wid_choose_game_type_stop_server_mouse_event (widp w, int32_t x, int32_t y,
                                              uint32_t button)
{
    wid_choose_game_type_stop_server_selected();

    return (true);
}

static uint8_t 
wid_choose_game_type_join_game_mouse_event (widp w, int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_choose_game_type_join_game_selected();

    return (true);
}

static uint8_t 
wid_choose_game_type_go_back_mouse_event (widp w, int32_t x, int32_t y,
                                          uint32_t button)
{
    wid_choose_game_type_hide();

    /*
     * Don't go back to choose stats as all points are spent.
     */
    wid_choose_name_visible();

    return (true);
}

static uint8_t wid_choose_game_type_play_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case ' ':
        case 's':
            wid_choose_game_type_play_selected();
            return (true);

        case 'm':
            wid_choose_game_type_start_server_selected();
            return (true);

        case 'b':
        case 'q':
        case SDLK_ESCAPE:
            wid_choose_game_type_hide();
            wid_intro_visible();
            return (true);

        default:
            break;
    }

    return (false);
}

static void wid_choose_game_type_bg_create (void)
{
    widp wid;

    if (wid_choose_game_type_background) {
        return;
    }

    {
        wid = wid_choose_game_type_background = wid_new_window("bg");

        float f = (1024.0 / 680.0);

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

static void wid_choose_game_type_create (void)
{
    if (wid_choose_game_type) {
        return;
    }

    wid_choose_game_type = wid_new_window("intro buttons");

    wid_set_no_shape(wid_choose_game_type);

    fpoint tl = {0.0f, 0.0f};
    fpoint br = {1.0f, 1.0f};
    wid_set_tl_br_pct(wid_choose_game_type, tl, br);
    wid_set_on_mouse_down(wid_choose_game_type, wid_choose_game_type_play_mouse_event);
    wid_set_on_key_down(wid_choose_game_type, wid_choose_game_type_play_key_event);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(wid_choose_game_type, WID_MODE_NORMAL);
    wid_set_color(wid_choose_game_type, WID_COLOR_TL, col);
    wid_set_color(wid_choose_game_type, WID_COLOR_BR, col);
    wid_set_color(wid_choose_game_type, WID_COLOR_BG, col);

    wid_choose_game_type_bg_create();
    wid_update(wid_choose_game_type);

    wid_move_to_pct_centered(wid_choose_game_type, 0.5f, 0.5f);

    on_mouse_down_t server_fn;
    const char *server_text;

    if (server_socket) {
        server_text = "Stop server (experimental)";
        server_fn = wid_choose_game_type_stop_server_mouse_event;
    } else {
        server_text = "Start a server (experimental)";
        server_fn = wid_choose_game_type_start_server_mouse_event;
    }

    menu = wid_menu(0,
                 vvlarge_font,
                 large_font,
                 0, // on_update
                 0.5, /* x */
                 0.4, /* y */
                 1, /* columns */
                 1, /* focus */
                 4, /* items */

                 (int) 'j', "Join server (experimental)", 
                    wid_choose_game_type_join_game_mouse_event,

                 (int) 's', "Single player", 
                    wid_choose_game_type_play_mouse_event,

                 (int) 'm', server_text, server_fn,

                 (int) 'b', "Back", wid_choose_game_type_go_back_mouse_event);
}
