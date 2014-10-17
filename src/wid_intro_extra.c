/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "wid_intro_extra.h"
#include "wid_intro.h"
#include "wid_popup.h"
#include "wid_editor.h"
#include "wid_intro_about.h"
#include "wid_intro_settings.h"
#include "wid_intro_buttons.h"
#include "wid_hiscore.h"
#include "wid_hiscore.h"
#include "wid_notify.h"
#include "wid_server_join.h"
#include "wid_server_create.h"
#include "thing_template.h"
#include "level.h"
#include "timer.h"
#include "socket.h"
#include "server.h"
#include "glapi.h"

static widp wid_intro_extra;
static widp wid_intro_extra_container;
static uint8_t wid_intro_extra_init_done;
static void wid_intro_extra_create(void);
static int intro_effect_delay = 500;

static const char *wid_intro_button_names[] = {
    "Settings",
    "Level editor",
    "Legends of old",
    "Credits",
};

typedef void (*on_event_t)(void);

static void wid_intro_extra_editor_selected(void);
static void wid_intro_extra_about_selected(void);
static void wid_intro_extra_settings_selected(void);
static void wid_intro_extra_hiscore_selected(void);

static on_event_t wid_intro_button_mouse_down[] = {
    wid_intro_extra_settings_selected,
    wid_intro_extra_editor_selected,
    wid_intro_extra_hiscore_selected,
    wid_intro_extra_about_selected,
};

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

    if (!wid_intro_extra) {
        DIE("no wid intro");
    }

    wid_hide(wid_intro_extra, 0);
}

void wid_intro_extra_visible (void)
{
    wid_intro_buttons_hide();

    if (wid_intro_extra_is_visible) {
        return;
    }

    if (wid_intro_extra) {
        wid_destroy(&wid_intro_extra);
    }

    wid_intro_extra_create();

    wid_intro_extra_is_visible = true;
    wid_intro_extra_is_hidden = false;

    if (!wid_intro_extra) {
        DIE("no wid intro");
    }

    wid_visible(wid_intro_extra, 0);

    wid_intro_buttons_hide();
}

static void wid_intro_extra_about_selected (void)
{
    wid_intro_extra_hide();

    wid_intro_about_visible();
}

static void wid_intro_extra_settings_selected (void)
{
    wid_intro_extra_hide();

    wid_intro_settings_visible();
}

static void wid_intro_extra_hiscore_selected (void)
{
    wid_intro_extra_hide();

    wid_hiscore_visible();
}

static void wid_intro_extra_editor_selected_cb (void *context)
{
    wid_intro_extra_hide();

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
    wid_intro_hide();
}

static uint8_t wid_intro_button_event (widp w, int32_t x, int32_t y,
                                       uint32_t button)
{
    /*
     * Increment.
     */
    int32_t row = (typeof(row)) (intptr_t) wid_get_client_context(w);

    (wid_intro_button_mouse_down[row])();

    return (true);
}

static uint8_t wid_intro_extra_mouse_back_event (widp w, int32_t x, int32_t y,
                                                 uint32_t button)
{
    wid_intro_extra_hide();

    wid_intro_buttons_visible();

    return (true);
}

static uint8_t wid_intro_extra_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'b':
        case 'q':
        case SDLK_ESCAPE:
            wid_intro_extra_hide();

            wid_intro_buttons_visible();
            return (true);

        default:
            break;
    }

    return (false);
}

static uint8_t wid_intro_extra_receive_mouse_motion (
                    widp w,
                    int32_t x, int32_t y,
                    int32_t relx, int32_t rely,
                    int32_t wheelx, int32_t wheely)
{
    if (wheelx || wheely) {
        /*
         * Allow scrolling.
         */
        return (false);
    }

    /*
     * Block moving the window.
     */
    return (true);
}

static void wid_intro_extra_create (void)
{
    if (!wid_intro_extra) {
        widp w = wid_intro_extra = wid_new_rounded_window("wid extra");

        fpoint tl = {0.2, 0.05};
        fpoint br = {0.8, 0.95};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_font(w, small_font);

        wid_set_color(w, WID_COLOR_TEXT, WHITE);

        wid_set_on_mouse_motion(w, wid_intro_extra_receive_mouse_motion);
    }

    {
        widp w = wid_intro_extra_container =
            wid_new_container(wid_intro_extra, "wid extra container");

        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 1.0};

        wid_set_tl_br_pct(w, tl, br);
    }

    {
        fpoint tl = {0.0, 0.0};
        fpoint br = {1.0, 0.15};

        widp w = wid_new_container(wid_intro_extra, "wid extra title");

        wid_set_tl_br_pct(w, tl, br);

        wid_set_text(w, "Stuffs");
        wid_set_font(w, small_font);
        wid_set_color(w, WID_COLOR_TEXT, GOLD);

        wid_set_text_outline(w, true);
    }

    {
        uint32_t i;

        for (i=0; i<ARRAY_SIZE(wid_intro_button_names); i++) {

            if (!wid_intro_button_names[i]) {
                continue;
            }

            widp w = wid_new_square_button(wid_intro_extra_container,
                                           wid_intro_button_names[i]);

            fpoint tl = {0.2, 0.2};
            fpoint br = {0.8, 0.3};

            double height = 0.12;

            br.y += (double)i * height;
            tl.y += (double)i * height;

            wid_set_tl_br_pct(w, tl, br);
            wid_set_text(w, wid_intro_button_names[i]);
            wid_set_font(w, small_font);

            color c = WHITE;

            c.a = 200;
            wid_set_mode(w, WID_MODE_NORMAL);
            wid_set_color(w, WID_COLOR_BG, c);

            c.a = 250;
            wid_set_mode(w, WID_MODE_OVER);
            wid_set_color(w, WID_COLOR_BG, c);

            wid_set_mode(w, WID_MODE_NORMAL);

            wid_set_client_context(w, (void*)(uintptr_t)i);
            wid_set_on_mouse_down(w, wid_intro_button_event);
            wid_set_bevel(w,0);

            wid_set_tex(w, 0, "button_green");
            wid_set_square(w);
        }
    }

    {
        widp w = wid_new_rounded_small_button(wid_intro_extra_container,
                                              "back");

        fpoint tl = {0.70, 0.80};
        fpoint br = {0.90, 0.90};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_text(w, "%%tile=button_b$Back");
        wid_set_font(w, small_font);

        color c = WHITE;

        c.a = 220;
        wid_set_mode(w, WID_MODE_NORMAL);
        wid_set_color(w, WID_COLOR_BG, c);

        c.a = 255;
        wid_set_mode(w, WID_MODE_OVER);
        wid_set_color(w, WID_COLOR_BG, c);

        wid_set_mode(w, WID_MODE_FOCUS);

        wid_set_mode(w, WID_MODE_NORMAL);

        wid_set_on_mouse_down(w, wid_intro_extra_mouse_back_event);
        wid_set_on_key_down(w, wid_intro_extra_key_event);

        wid_set_tex(w, 0, "button_black");
        wid_set_square(w);

    }

    wid_raise(wid_intro_extra);

    wid_update(wid_intro_extra);

    wid_set_tex(wid_intro_extra, 0, "window_gothic");
    wid_set_square(wid_intro_extra);

    wid_move_to_pct_centered(wid_intro_extra, 0.5, 0.5 - 1.0);
    wid_move_to_pct_centered_in(wid_intro_extra, 0.5, 0.5, 200);
}
