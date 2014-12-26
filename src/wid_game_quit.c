/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "wid_popup.h"
#include "wid_game_quit.h"
#include "client.h"
#include "wid_game_map_client.h"
#include "wid_intro.h"
#include "string_util.h"

static widp wid_really_quit_game_yes_no;

static uint8_t wid_game_quit_init_done;
static void wid_game_quit_create(void);
static void wid_game_quit_destroy(void);

uint8_t wid_game_quit_init (void)
{
    if (!wid_game_quit_init_done) {
    }

    wid_game_quit_init_done = true;

    return (true);
}

void wid_game_quit_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_game_quit_init_done) {
        wid_game_quit_init_done = false;

        wid_game_quit_destroy();
    }
}

void wid_game_quit_hide (void)
{
    wid_game_quit_destroy();
}

void wid_game_quit_visible (void)
{
    wid_game_quit_create();
}

static void wid_game_quit_destroy (void)
{
    wid_destroy(&wid_really_quit_game_yes_no);
}

static void wid_game_quit_disconnect_with_server (void)
{
    LOG("Client: Disconnect with server");

    client_socket_close(0, 0);
    wid_game_map_client_hide();
    wid_game_map_client_wid_destroy();
    wid_game_quit_destroy();
}

static void wid_game_quit_really_quit_callback_yes (widp wid)
{
    LOG("Client: really quit, yes");
    wid_destroy(&wid_really_quit_game_yes_no);
    wid_game_quit_disconnect_with_server();
    wid_intro_visible();
}

static void wid_game_quit_really_quit_callback_no (widp wid)
{
    LOG("Client: really quit, no");
    wid_destroy(&wid_really_quit_game_yes_no);
}

static void wid_game_quit_create (void)
{
    if (sdl_is_exiting()) {
        return;
    }

    static const char *messages[] = {
        "I thought you were made of the right stuff",
        "I thought you were tougher",
        "Cluck cluck!",
        "You gonna bail on me?",
        "Cmon, this is no fun?",
        "Keep going you coward",
        "Look I'll be nice, keep playing",
        "When the going gets tough, you just leave, right?",
        "Ho hum. Such High hopes...",
        "What? Wait",
        "Why? Why? Why?",
        "What did I ever do to you?\nExcept all the monster attacks",
        "Don't do this to me",
        "Please Dave, don't do it",
        "You will not quit, soldier",
        "Perhaps you should play snap instead?",
        "Too tough for you eh?",
        "I give in, looks like you do too",
        "You canny take it cap'n?",
        "Just one more level, cmon",
        "It gets easier, honest",
        "I'll be nice in the next level",
        "Go on then, go off an cry",
        "Is it something I said?",
        "Is it something I killed?",
        "Is it something I manifested?",
        "Let's not break up just yet!",
        "It wont get better if you quit",
        "You quit now, I'll make the next level worse!",
    };

    if (wid_really_quit_game_yes_no) {
        return;
    }

    wid_really_quit_game_yes_no = wid_popup(
            messages[myrand() % ARRAY_SIZE(messages)],
            "%%fg=red$Quit game?\n",
            0.5, 0.5f,                /* x,y postition in percent */
            med_font,               /* title font */
            med_font,              /* body font */
            med_font,              /* button font */
            2,                        /* number buttons */
            "%%tile=button_y$Yes    ", wid_game_quit_really_quit_callback_yes,
            "%%tile=button_n$No    ",  wid_game_quit_really_quit_callback_no);

    wid_set_tex(wid_really_quit_game_yes_no, 0, "gothic_wide");
    wid_set_square(wid_really_quit_game_yes_no);
}
