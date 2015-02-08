/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "wid_intro.h"
#include "wid_choose_player.h"
#include "wid_popup.h"
#include "wid_intro_extra.h"
#include "wid_intro_about.h"
#include "wid_intro_settings.h"
#include "wid_intro_buttons.h"
#include "wid_game_over.h"
#include "wid_game_map_client.h"
#include "wid_hiscore.h"
#include "wid_hiscore.h"
#include "wid_notify.h"
#include "wid_server_join.h"
#include "wid_server_create.h"
#include "thing_template.h"
#include "music.h"
#include "level.h"
#include "timer.h"
#include "socket_util.h"
#include "server.h"
#include "glapi.h"

widp wid_intro_buttons;
static widp wid_intro_quit_popup;

static void wid_intro_quit_callback_yes (widp wid)
{
    wid_destroy(&wid_intro_quit_popup);

    FINI_LOG("quit yes selected");
    sdl_exit();
}

static void wid_intro_quit_callback_no (widp wid)
{
    wid_destroy(&wid_intro_quit_popup);
}

uint8_t wid_intro_quit_selected (void)
{
    LOG("Intro quit selected");

    if (wid_intro_quit_popup) {
        return (false);
    }

    wid_intro_quit_popup = wid_popup(
          "",
          "%%fg=red$Quit game?", 
          0.5, 0.5,                 /* x,y postition in percent */
          large_font,               /* title font */
          large_font,              /* body font */
          large_font,              /* button font */
          2,                        /* number buttons */
          "%%tile=button_y$Yes    ", wid_intro_quit_callback_yes,
          "%%tile=button_n$No    ",  wid_intro_quit_callback_no);

    wid_set_tex(wid_intro_quit_popup, 0, "gothic_wide");
    wid_set_square(wid_intro_quit_popup);

    return (true);
}
