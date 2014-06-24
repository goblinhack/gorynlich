/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "wid_intro.h"
#include "wid_popup.h"
#include "wid_dead.h"
#include "thing.h"
#include "wid_notify.h"
#include "timer.h"
#include "client.h"
#include "wid_game_map_client.h"
#include "wid_intro.h"
#include "player.h"

static widp wid_gravestone;
static widp wid_click_to_continue;
static widp wid_rejoin_game_yes_no;
static widp wid_replay_game_yes_no;

static uint8_t wid_dead_init_done;
static void wid_dead_create(const char *name, 
                            const char *reason,
                            uint8_t rejoin_allowed);
static void wid_dead_destroy(void);

uint8_t wid_dead_init (void)
{
    if (!wid_dead_init_done) {
    }

    wid_dead_init_done = true;

    return (true);
}

void wid_dead_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_dead_init_done) {
        wid_dead_init_done = false;

        wid_dead_destroy();
    }
}

void wid_dead_hide (void)
{
    wid_dead_destroy();
}

void wid_dead_visible (const char *name, 
                       const char *reason,
                       uint8_t rejoin_allowed)
{
    wid_dead_create(name, reason, rejoin_allowed);
}

static void wid_dead_destroy (void)
{
    wid_destroy(&wid_gravestone);
    wid_destroy(&wid_click_to_continue);
    wid_destroy(&wid_rejoin_game_yes_no);
    wid_destroy(&wid_replay_game_yes_no);
}

static void wid_dead_ (void)
{
    LOG("Client: Gravestone raised, player quit");

    client_socket_leave();

    wid_game_map_client_hide();

    wid_game_map_client_wid_destroy();

    wid_dead_destroy();
}

static void wid_dead_rejoin_callback_yes (widp wid)
{
    wid_destroy(&wid_rejoin_game_yes_no);

    wid_dead_();

    client_socket_join(0, 0, 0, true /* quiet */);

    wid_game_map_client_visible();
}

static void wid_dead_rejoin_callback_no (widp wid)
{
    wid_destroy(&wid_rejoin_game_yes_no);

    wid_dead_();
}

static void wid_dead_replay_callback_yes (widp wid)
{
    wid_destroy(&wid_replay_game_yes_no);

    wid_dead_();

    client_socket_join(0, 0, 0, true /* quiet */);

    wid_game_map_client_visible();
}

static void wid_dead_replay_callback_no (widp wid)
{
    wid_destroy(&wid_replay_game_yes_no);

    wid_dead_();
}

static uint8_t is_rejoin_allowed;

static void wid_dead_gravestone_appeared (void *context)
{
    LOG("Client: Gravestone raised");

    wid_notify_flush();

    if (is_rejoin_allowed) {
        wid_rejoin_game_yes_no =
            wid_popup("%%fg=red$Rejoin?",
                      0                 /* title */,
                      0.5f, 0.5f,       /* x,y postition in percent */
                      large_font,       /* title font */
                      large_font,       /* body font */
                      large_font,       /* button font */
                      2,                /* number buttons */
                      "Yes", wid_dead_rejoin_callback_yes,
                      "No", wid_dead_rejoin_callback_no);

    } else {
        wid_replay_game_yes_no =
            wid_popup("%%fg=red$Replay?",
                      0                 /* title */,
                      0.5f, 0.5f,       /* x,y postition in percent */
                      large_font,       /* title font */
                      large_font,       /* body font */
                      large_font,       /* button font */
                      2,                /* number buttons */
                      "Yes", wid_dead_replay_callback_yes,
                      "No", wid_dead_replay_callback_no);
    }
}

static void wid_dead_create (const char *name, 
                             const char *reason,
                             uint8_t rejoin_allowed)
{
    if (sdl_is_exiting()) {
        return;
    }

    if (player && player->player) {
        LOG("Client: Player \"%s\" (Id %u) died, raise gravestone",
            player->player->name, player->player->key);
    } else {
        LOG("Client: Player died, raise gravestone");
    }

    widp w = wid_gravestone = wid_new_window("dead");
    fpoint tl = { 0.0, 0.3 };
    fpoint br = { 0.4, 1.0 };

    wid_set_tl_br_pct(w, tl, br);

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, WHITE);
    wid_set_color(w, WID_COLOR_BR, RED);
    wid_set_color(w, WID_COLOR_BG, WHITE);
    wid_set_font(w, vsmall_font);
    wid_set_bevelled(w, 10);
    wid_set_tex(w, 0, "gravestone");
    wid_set_ignore_events(w, true);

    wid_destroy_in(w, 110000);

    {
        widp w2 = wid_new_square_button(w, "dead");
        wid_set_mode(w2, WID_MODE_NORMAL);
        color c = RED;
        c.a = 0;
        wid_set_color(w2, WID_COLOR_TL, c);
        wid_set_color(w2, WID_COLOR_BR, c);
        wid_set_color(w2, WID_COLOR_BG, c);
        wid_set_color(w2, WID_COLOR_TEXT, GREEN);
        wid_set_text(w2, name);
        wid_set_font(w2, vsmall_font);
        wid_set_ignore_events(w2, true);

        {
            fpoint tl = { 0.0, 0.5 };
            fpoint br = { 1.0, 0.6 };
            wid_set_tl_br_pct(w2, tl, br);
        }
    }

    {
        widp w2 = wid_new_square_button(w, "dead");
        wid_set_mode(w2, WID_MODE_NORMAL);
        color c = BLACK;
        c.a = 0;
        wid_set_color(w2, WID_COLOR_TL, c);
        wid_set_color(w2, WID_COLOR_BR, c);
        wid_set_color(w2, WID_COLOR_BG, c);
        wid_set_color(w2, WID_COLOR_TEXT, GREEN);
        wid_set_text(w2, reason);
        wid_set_font(w2, vsmall_font);
        wid_set_ignore_events(w2, true);

        {
            fpoint tl = { 0.0, 0.8 };
            fpoint br = { 1.0, 0.9 };
            wid_set_tl_br_pct(w2, tl, br);
        }
    }

    wid_raise(w);

    wid_update(w);
    wid_move_to_pct(w, 0.6, 1.3);

    static uint32_t gravestone_appear_delay = 3000;

    wid_move_to_pct_in(w, 0.6, 0.3, gravestone_appear_delay);

    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_dead_gravestone_appeared,
            (action_timer_destroy_callback)0,
            0, /* context */
            "wid dead timer",
            gravestone_appear_delay,
            0 /* jitter */);

    is_rejoin_allowed = rejoin_allowed;
}
