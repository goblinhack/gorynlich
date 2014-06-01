/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "ttf.h"
#include "wid_notify.h"
#include "time.h"

static const int32_t PAD_X = 80;
static const int32_t PAD_Y = 25;

typedef struct {
    uint32_t created;
    widp w;
} wid_notify_elem;

#define MAX_ELEMS 10
static uint32_t nelems;
static wid_notify_elem elems[MAX_ELEMS];

static double X = 0.675;
static double Y = 1.0;
static double H = 0.05;
static uint32_t DELAY = 5000;

/*
 * Create the wid_notify
 */
static widp
wid_notify_internal (const char *text, uint32_t level)
{
    widp wid_notify_window;
    uint32_t maxw = 0;
    uint32_t maxh = 0;
    fontp font = vsmall_font;

    ttf_text_size(font, text, &maxw, &maxh, 0, 1.0f, 1.0f,
                  false /* fixed width */);

    {
        wid_notify_window = wid_new_rounded_window("wid_notify");
        wid_set_text(wid_notify_window, text);
        wid_set_font(wid_notify_window, font);
        wid_set_bevel(wid_notify_window, 1);
        wid_set_rounded_small(wid_notify_window);

        wid_set_color(wid_notify_window, WID_COLOR_TEXT, WHITE);

        color c = BLACK;

        switch (level) {
        case 0:
            c = WHITE;
            c.a = 20;
            break;
        case 1:
            c = ORANGE;
            break;
        case 2:
            c = RED;
            break;
        }

        wid_set_color(wid_notify_window, WID_COLOR_BG, c);

        c = WHITE;
        c.a = 150;
        wid_set_color(wid_notify_window, WID_COLOR_TL, c);
        wid_set_color(wid_notify_window, WID_COLOR_BR, c);
        wid_set_text_outline(wid_notify_window, true);

        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        tl.x = X;
        tl.y = 0;

        br.x = 1.0;
        br.y = H;

        wid_set_tl_br_pct(wid_notify_window, tl, br);

        wid_move_to_pct(wid_notify_window, X, 0);
    }

    wid_fade_in(wid_notify_window, wid_fade_delay);

    wid_set_ignore_events(wid_notify_window, true);

    wid_raise(wid_notify_window);

    wid_update(wid_notify_window);

    wid_set_do_not_lower(wid_notify_window, true);

    return (wid_notify_window);
}

static void wid_notify_move_wids (void)
{
    /*
     * Move widgets to their new position.
     */
    uint32_t i;

    for (i = 0; i < nelems; i++) {
        wid_notify_elem *elem = &elems[i];

        wid_move_to_pct_in(elem->w, X, Y - H * (double) (i+1), 500);
    }
}

static void wid_notify_scroll (void)
{
    if (!nelems) {
        return;
    }

    /*
     * Pop the head element.
     */
    wid_notify_elem *elem = &elems[0];
    wid_destroy(&elem->w);
    nelems--;

    /*
     * Move the whole table down one element.
     */
    uint32_t i;

    for (i = 0; i < nelems; i++) {
        elems[i].w = elems[i+1].w;
        elems[i].created = elems[i+1].created;
    }
    elems[i].w = 0;
    elems[i].created = 0;

    wid_notify_move_wids();
}

widp wid_notify (const char *text, uint32_t level)
{
    widp w;

    w = wid_notify_internal(text, level);

    while (nelems >= MAX_ELEMS - 1) {
        wid_notify_scroll();
    }

    int32_t timestamp_now = time_update_time_milli();
    elems[nelems].created = timestamp_now;
    elems[nelems].w = w;
    nelems++;

    wid_notify_move_wids();

    return (w);
}

void wid_notify_tick (void)
{
    static int x;

    if (!x) {
        wid_notify("hello", NORMAL);
        wid_notify("there", NORMAL);
        wid_notify("you", NORMAL);
        wid_notify("oh-oh", WARNING);
        wid_notify("panic panic dive dive!", SEVERE);
        x = 1;
    }

    if (!nelems) {
        return;
    }

    int32_t timestamp_now = time_update_time_milli();
    wid_notify_elem *elem = &elems[0];

    if (timestamp_now - elem->created < DELAY) {
        return;
    }

    wid_notify_scroll();
}
