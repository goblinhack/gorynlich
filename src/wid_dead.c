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
#include "wid_dead.h"
#include "thing.h"
#include "wid_notify.h"
#include "timer.h"
#include "client.h"
#include "wid_game_map_client.h"
#include "wid_intro.h"
#include "player.h"
#include "string.h"
#include "music.h"

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

static void wid_dead_disconnect_with_server (void)
{
    LOG("Client: Disconnect with server");

    client_socket_close(0, 0);
    wid_game_map_client_hide();
    wid_game_map_client_wid_destroy();
    wid_dead_destroy();
}

static void wid_dead_rejoin_callback_yes (widp wid)
{
    LOG("Client: Rejoin, yes");
    wid_destroy(&wid_rejoin_game_yes_no);
    wid_dead_disconnect_with_server();
    client_socket_join(0, 0, 0, true /* quiet */);
    wid_game_map_client_visible();
}

static void wid_dead_rejoin_callback_no (widp wid)
{
    LOG("Client: Rejoin, no");
    wid_destroy(&wid_rejoin_game_yes_no);
    wid_dead_disconnect_with_server();
    wid_intro_visible();
}

static void wid_dead_replay_callback_yes (widp wid)
{
    LOG("Client: Replay, yes");
    wid_destroy(&wid_replay_game_yes_no);
    wid_dead_disconnect_with_server();
    client_socket_join(0, 0, 0, true /* quiet */);
    wid_game_map_client_visible();
}

static void wid_dead_replay_callback_no (widp wid)
{
    LOG("Client: Replay, no");
    wid_destroy(&wid_replay_game_yes_no);
    wid_dead_disconnect_with_server();
    wid_intro_visible();
}

static uint8_t is_rejoin_allowed;

static void wid_dead_gravestone_appeared (void *context)
{
    LOG("Client: Gravestone raised");

    static const char *messages[] = {
        "You bit the dust",
        "You snuffed it",
        "You croaked it",
        "You left the mortal coil",
        "You died horribly",
        "You find yourself sadly departed",
        "You seem to be suddenly expired",
        "Hmm you seem to be dead",
        "You passed on",
        "You are in a better place",
        "You are at rest, sort of",
        "You bought the farm",
        "You checked out",
        "You slipped away",
        "You kicked the bucket",
        "You are pushing up daisies",
        "You gave up the ghost",
        "You had your final summons",
        "You have gone to a better place",
        "You paid the piper",
        "You reached your final destination",
        "You number was up",
        "You bade farewell to life",
        "You crossed the great divide",
        "You went to the spirit in the sky",
        "You went to the ranch in the sky",
        "You went to the restaurant at the end of the universe",
        "You came to a sticky end",
        "You popped your clogs",
        "You went off to make your maker",
        "You joined the choir invisible",
        "Your god told you to slow down",
        "You are six feet under",
        "You are  off to the happy hunting ground",
        "You fell off the perch",
        "You are as lifeless as a norweigian blue",
        "I had such high hopes for you",
        "You dissapoint me",
        "Pathetic",
        "Good try I suppose",
        "Well, that could have been worse",
        "Don't play games as a career",
        "Are you paid to play this?",
        "Are you paid to play this?\nYou shouldn't be",
        "Is that the best you could do?",
        "Awful. Just awful.",
        "My son plays better than that",
        "Let your mum play next time instead",
        "Let your mom play next time instead",
        "Let your pet pigeon play next time instead",
        "I have a goat plays better than you",
        "My pet pigeon ccould do better",
        "Worst game ever",
        "Epic fail",
        "Just stop playing. You'll never win",
        "Believe in yourself more\nSomeone has to",
        "Don't give up\nIn general. But give up on this game",
        "Goodbye cruel world",
    };

    wid_notify_flush();

    if (is_rejoin_allowed) {

        wid_rejoin_game_yes_no = wid_popup(
            "%%fg=red$Rejoin game?\n",
            messages[rand() % ARRAY_SIZE(messages)],
            0.5, 0.2f,                /* x,y postition in percent */
            small_font,               /* title font */
            vsmall_font,              /* body font */
            vsmall_font,              /* button font */
            2,                        /* number buttons */
            "%%tile=button_y$Yes       ", wid_dead_rejoin_callback_yes,
            "%%tile=button_n$No       ",  wid_dead_rejoin_callback_no);

        wid_set_tex(wid_rejoin_game_yes_no, 0, "gothic_wide");
        wid_set_square(wid_rejoin_game_yes_no);
    } else {
        wid_replay_game_yes_no = wid_popup(
            "%%fg=red$Play again?\n",
            messages[rand() % ARRAY_SIZE(messages)],
            0.5, 0.2f,                /* x,y postition in percent */
            small_font,               /* title font */
            vsmall_font,              /* body font */
            vsmall_font,              /* button font */
            2,                        /* number buttons */
            "%%tile=button_y$Yes       ", wid_dead_replay_callback_yes,
            "%%tile=button_n$No       ",  wid_dead_replay_callback_no);

        wid_set_tex(wid_replay_game_yes_no, 0, "gothic_wide");
        wid_set_square(wid_replay_game_yes_no);
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
    fpoint tl = { 0.0, 0.2 };
    fpoint br = { 0.4, 1.0 };

    wid_set_tl_br_pct(w, tl, br);

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, WHITE);
    wid_set_color(w, WID_COLOR_BR, WHITE);
    wid_set_color(w, WID_COLOR_BG, WHITE);
    wid_set_font(w, vsmall_font);
    wid_set_bevelled(w, 10);
    wid_set_tex(w, 0, "gravestone");
    wid_set_ignore_events(w, true);

    wid_destroy_in(w, 110000);

    {
        widp w2 = wid_new_square_button(w, "dead");
        wid_set_mode(w2, WID_MODE_NORMAL);
        color c = GRAY;
        c.a = 0;
        wid_set_color(w2, WID_COLOR_TL, c);
        wid_set_color(w2, WID_COLOR_BR, c);
        wid_set_color(w2, WID_COLOR_BG, c);
        wid_set_color(w2, WID_COLOR_TEXT, GRAY);
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
        wid_set_color(w2, WID_COLOR_TEXT, GRAY);
        wid_set_text(w2, reason);
        wid_set_font(w2, vsmall_font);
        wid_set_ignore_events(w2, true);

        {
            fpoint tl = { 0.0, 0.7 };
            fpoint br = { 1.0, 0.8 };
            wid_set_tl_br_pct(w2, tl, br);
        }
    }

    wid_raise(w);

    wid_update(w);
    wid_move_to_pct(w, 0.6, 1.3);

    music_play_dead();

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
