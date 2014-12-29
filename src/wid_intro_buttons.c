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

/*
 * Mouse up etc...
 */
static uint8_t wid_intro_quit_receive_mouse_down (widp w,
                                                int32_t x, int32_t y,
                                                uint32_t button)
{
    return (wid_intro_quit_selected());
}

void wid_intro_intro_extra_selected (void)
{
    LOG("Intro extra options selected");

    wid_intro_extra_visible();
}

static uint8_t wid_intro_intro_extra_mouse_event (widp w, int32_t x, int32_t y,
                                                  uint32_t button)
{
    wid_intro_intro_extra_selected();

    return (true);
}

void wid_intro_buttons_hide (void)
{
    wid_destroy(&wid_intro_buttons);
}

void wid_intro_buttons_visible (void)
{
    if (wid_intro_buttons) {
        wid_destroy(&wid_intro_buttons);
    }

    wid_intro_buttons = wid_new_window("intro buttons");

    wid_set_no_shape(wid_intro_buttons);

    fpoint tl = {0.0f, 0.0f};
    fpoint br = {1.0f, 1.0f};
    wid_set_tl_br_pct(wid_intro_buttons, tl, br);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(wid_intro_buttons, WID_MODE_NORMAL);
    wid_set_color(wid_intro_buttons, WID_COLOR_TL, col);
    wid_set_color(wid_intro_buttons, WID_COLOR_BR, col);
    wid_set_color(wid_intro_buttons, WID_COLOR_BG, col);

    wid_update(wid_intro_buttons);

    {
        widp child;

        child = wid_new_square_button(wid_intro_buttons, "Extra");
        wid_set_font(child, med_font);

        fpoint tl = {0.0f, 0.90f};
        fpoint br = {0.3f, 0.95f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "%%fmt=left$%%tile=button_x$Extra stuff");

        wid_set_no_shape(child);
        wid_set_color(child, WID_COLOR_TEXT, GRAY90);
        wid_set_mode(child, WID_MODE_OVER);
        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_intro_extra_mouse_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro_buttons, "Quick start");
        wid_set_font(child, med_font);

        fpoint tl = {0.0f, 0.95f};
        fpoint br = {0.3f, 1.00f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "%%fmt=left$%%tile=button_s$Quick start");

        wid_set_no_shape(child);
        wid_set_color(child, WID_COLOR_TEXT, GRAY90);
        wid_set_mode(child, WID_MODE_OVER);
        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_intro_extra_mouse_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro_buttons, "Quit");
        wid_set_font(child, med_font);

        fpoint tl = {0.9f, 0.95f};
        fpoint br = {1.0f, 1.00f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "%%fmt=left$%%tile=button_q$Quit");

        wid_set_no_shape(child);
        wid_set_color(child, WID_COLOR_TEXT, GRAY90);
        wid_set_mode(child, WID_MODE_OVER);
        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_quit_receive_mouse_down);
        wid_raise(child);
        wid_set_do_not_lower(child, true);
    }

    wid_update(wid_intro_buttons);
}
