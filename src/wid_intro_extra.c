/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "wid_intro_extra.h"
#include "wid_popup.h"
#include "wid_editor.h"
#include "wid_intro_about.h"
#include "wid_intro_settings.h"
#include "wid_hiscore.h"
#include "wid_hiscore.h"
#include "wid_notify.h"
#include "wid_server_join.h"
#include "wid_server_create.h"
#include "thing_template.h"
#include "music.h"
#include "level.h"
#include "timer.h"
#include "socket.h"
#include "server.h"
#include "glapi.h"

static widp wid_intro_extra;

static uint8_t wid_intro_extra_quit_selected(void);
static void wid_intro_extra_editor_selected(void);
static void wid_intro_extra_about_selected(void);
static void wid_intro_extra_settings_selected(void);
static void wid_intro_extra_hiscore_selected(void);
static void wid_server_join_selected(void);

static uint8_t wid_intro_extra_init_done;
static void wid_intro_extra_create(void);

static int intro_effect_delay = 500;

uint8_t wid_intro_extra_init (void)
{
    if (!wid_intro_extra_init_done) {
        wid_intro_extra_create();
    }

    wid_intro_extra_init_done = true;

    return (true);
}

void wid_intro_extra_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_intro_extra_init_done) {
        wid_intro_extra_init_done = false;

        if (wid_intro_extra) {
            wid_destroy(&wid_intro_extra);
        }
    }
}

static uint8_t wid_intro_extra_is_hidden;
static uint8_t wid_intro_extra_is_visible;

void wid_intro_extra_hide (void)
{
    if (wid_intro_extra_is_hidden) {
        return;
    }

    LOG("Client: Intro screen hide");

    wid_intro_extra_is_hidden = true;
    wid_intro_extra_is_visible = false;

    music_halt();

    if (!wid_intro_extra) {
        DIE("no wid intro");
    }

    wid_hide(wid_intro_extra, 0);
}

void wid_intro_extra_visible (void)
{
    if (wid_intro_extra_is_visible) {
        return;
    }

    if (wid_intro_extra) {
        wid_destroy(&wid_intro_extra);
        wid_intro_extra_create();
    }

    wid_notify_flush();

    LOG("Client: Intro screen show");

    wid_intro_extra_is_visible = true;
    wid_intro_extra_is_hidden = false;

    if (!wid_intro_extra) {
        DIE("no wid intro");
    }

    wid_visible(wid_intro_extra, 0);
}

static void wid_intro_extra_about_selected (void)
{
    wid_intro_about_visible();
}

static void wid_intro_extra_settings_selected (void)
{
    wid_intro_settings_visible();
}

static void wid_intro_extra_hiscore_selected (void)
{
    wid_hiscore_visible();
}

static void wid_server_join_selected (void)
{
    wid_server_join_visible();
}

static uint8_t wid_intro_extra_about_mouse_event (widp w, int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_intro_extra_about_selected();

    return (true);
}

static uint8_t wid_intro_extra_settings_mouse_event (widp w, int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_intro_extra_settings_selected();

    return (true);
}

static uint8_t wid_intro_extra_hiscore_mouse_event (widp w, int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_intro_extra_hiscore_selected();

    return (true);
}

static uint8_t wid_server_join_mouse_event (widp w, int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_server_join_selected();

    return (true);
}

static void wid_intro_extra_editor_selected_cb (void *context)
{
    wid_editor_visible();
}

static void wid_intro_extra_editor_selected (void)
{
    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_intro_extra_editor_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start editor",
            intro_effect_delay,
            0 /* jitter */);

    wid_intro_extra_hide();
}

static uint8_t wid_intro_extra_editor_mouse_event (widp w, int32_t x, int32_t y,
                                             uint32_t button)
{
    wid_intro_extra_editor_selected();

    return (true);
}

static widp wid_intro_extra_quit_popup;

static void wid_intro_extra_quit_callback_yes (widp wid)
{
    wid_destroy(&wid_intro_extra_quit_popup);

    FINI_LOG("quit yes selected");
    sdl_exit();
}

static void wid_intro_extra_quit_callback_no (widp wid)
{
    wid_destroy(&wid_intro_extra_quit_popup);
}

static uint8_t wid_intro_extra_quit_selected (void)
{
    if (wid_intro_extra_quit_popup) {
        return (false);
    }

    wid_intro_extra_quit_popup = wid_popup(
          "\n"
          "\n"
          ,
          "%%fg=red$Quit game?",    /* title */
          0.5, 0.5,                 /* x,y postition in percent */
          small_font,               /* title font */
          vsmall_font,              /* body font */
          vsmall_font,              /* button font */
          2,                        /* number buttons */
          "   Yes   ", wid_intro_extra_quit_callback_yes,
          "    No    ", wid_intro_extra_quit_callback_no);

    wid_set_tex(wid_intro_extra_quit_popup, 0, "gothic_wide");
    wid_set_square(wid_intro_extra_quit_popup);

    return (true);
}

/*
 * Mouse up etc...
 */
static uint8_t wid_intro_extra_quit_receive_mouse_down (widp w,
                                                int32_t x, int32_t y,
                                                uint32_t button)
{
    return (wid_intro_extra_quit_selected());
}

static void wid_intro_extra_create (void)
{
    if (wid_intro_extra) {
        return;
    }

    music_play_intro();

    wid_intro_extra = wid_new_window("intro buttons");

    wid_set_no_shape(wid_intro_extra);

    fpoint tl = {0.0f, 0.0f};
    fpoint br = {1.0f, 1.0f};
    wid_set_tl_br_pct(wid_intro_extra, tl, br);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(wid_intro_extra, WID_MODE_NORMAL);
    wid_set_color(wid_intro_extra, WID_COLOR_TL, col);
    wid_set_color(wid_intro_extra, WID_COLOR_BR, col);
    wid_set_color(wid_intro_extra, WID_COLOR_BG, col);

    {
        widp child;

        child = wid_new_square_button(wid_intro_extra, "Editor");
        wid_set_font(child, small_font);
        wid_set_no_shape(child);

        fpoint tl = {0.1f, 0.0f};
        fpoint br = {0.3f, 0.05};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "%%tile=button_e$Editor");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = WHITE;
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 255;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 255;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_extra_editor_mouse_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro_extra, "About");
        wid_set_font(child, small_font);
        wid_set_no_shape(child);

        fpoint tl = {0.3f, 0.0f};
        fpoint br = {0.5f, 0.05};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "About");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = WHITE;
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 255;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 225;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_extra_about_mouse_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro_extra, "Settings");
        wid_set_font(child, small_font);
        wid_set_no_shape(child);

        fpoint tl = {0.5f, 0.0f};
        fpoint br = {0.7f, 0.05};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "Settings");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = WHITE;
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 255;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 225;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_extra_settings_mouse_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro_extra, "Hiscore");
        wid_set_font(child, small_font);
        wid_set_no_shape(child);

        fpoint tl = {0.7f, 0.0f};
        fpoint br = {0.9f, 0.05};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "Hiscore");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = WHITE;
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 255;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 225;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_intro_extra_hiscore_mouse_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro_extra, "servers");
        wid_set_font(child, small_font);
        wid_set_no_shape(child);

        fpoint tl = {0.7f, 0.2f};
        fpoint br = {0.9f, 0.05};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "server");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = WHITE;
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 255;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 225;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_server_join_mouse_event);
    }

    {
        widp child;

        child = wid_new_square_button(wid_intro_extra, "quit");
        wid_set_font(child, small_font);
        wid_set_no_shape(child);

        fpoint tl = {0.66f, 0.90f};
        fpoint br = {1.00f, 1.00f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "Quit");

        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        color c = WHITE;
        c.a = 200;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 255;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_mode(child, WID_MODE_FOCUS);
        c.a = 225;
        wid_set_color(child, WID_COLOR_TEXT, c);

        wid_set_on_mouse_down(child, wid_intro_extra_quit_receive_mouse_down);
    }

    wid_update(wid_intro_extra);

    wid_move_to_pct_centered(wid_intro_extra, 0.5f, 0.5f);
}
