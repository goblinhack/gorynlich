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

static widp wid_dead;
static widp wid_dead_credits;
static uint8_t wid_dead_init_done;
static void wid_dead_create(const char *name, const char *reason);
static void wid_dead_destroy(void);
static void wid_dead_finished(widp wid);

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

void wid_dead_visible (const char *name, const char *reason)
{
    wid_dead_create(name, reason);
}

static void wid_dead_destroy (void)
{
    wid_destroy(&wid_dead);
    wid_destroy(&wid_dead_credits);
}

static void wid_dead_finished (widp wid)
{
    if (sdl_is_exiting()) {
        return;
    }

    wid_dead_hide();

    wid_intro_visible();
}

static void wid_dead_flush_logs (void *context)
{
    wid_notify_flush();
}

static void wid_dead_create (const char *name, const char *reason)
{
    if (sdl_is_exiting()) {
        return;
    }

    widp w = wid_new_window("dead");
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

    wid_destroy_in(w, 110000);
    wid_set_on_destroy(w, wid_dead_finished);

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

        {
            fpoint tl = { 0.0, 0.8 };
            fpoint br = { 1.0, 0.9 };
            wid_set_tl_br_pct(w2, tl, br);
        }
    }

    wid_raise(w);

    wid_update(w);
    wid_move_to_pct(w, 0.6, 1.5);

    static uint32_t gravestone_appear_delay = 5000;

    wid_move_to_pct_in(w, 0.6, 0.3, gravestone_appear_delay);

    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_dead_flush_logs,
            (action_timer_destroy_callback)0,
            0, /* context */
            "wid dead timer",
            gravestone_appear_delay,
            0 /* jitter */);

}
