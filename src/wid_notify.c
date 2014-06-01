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
#include "tree.h"
#include "string_ext.h"

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
static uint32_t DELAY = 5000;

#if 0
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

    }

    wid_raise(wid_notify_window);

    wid_update(wid_notify_window);

    wid_set_do_not_lower(wid_notify_window, true);

    return (wid_notify_window);
}
#endif
/*
 * Create the wid_notify
 */
static widp
wid_notify_internal (const char *text, uint32_t level)
{
    widp wid_notify_container;
    widp wid_notify_window;
    tree_string_split_node *n;
    tree_root *d;
    uint32_t maxw = 0;
    uint32_t maxh = 0;
    uint32_t toth = 0;
    uint32_t rows = 0;
    uint32_t chars_per_line;
    uint32_t max_rows;

    chars_per_line = 20;
    max_rows = 22;

    d = split(text, chars_per_line);

    /*
     * Get some rough sizes for the font.
     */
    { TREE_WALK(d, n) {
        uint32_t w;
        uint32_t h;

        ttf_text_size(small_font, n->line, &w, &h, 0, 1.0f, 1.0f,
                      false /* fixed width */);

        /*
         * .5 line spacing for single newlines.
         */
        if (!strlen(n->line)) {
            h = maxh/2;
        }

        maxw = max(w, maxw);
        maxh = max(h, maxh);

        toth += h;
        rows++;
    } }

    {
        wid_notify_window = wid_new_rounded_window("wid_notify");

        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        br.x += maxw;

        if (rows > max_rows) {
            br.y += maxh * max_rows;
        } else {
            br.y += toth;
        }

        wid_set_tl_br(wid_notify_window, tl, br);
    }

    {
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
            c = DARKRED;
            break;
        }

        wid_set_color(wid_notify_window, WID_COLOR_BG, c);

        c = WHITE;
        c.a = 150;
        wid_set_color(wid_notify_window, WID_COLOR_TL, c);
        wid_set_color(wid_notify_window, WID_COLOR_BR, c);

        wid_move_to_pct(wid_notify_window, X, 0);
    }

    {
        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        br.x += maxw;

        if (rows > max_rows) {
            br.y += maxh * max_rows;
        } else {
            br.y += toth;
        }

        wid_notify_container = wid_new_container(wid_notify_window,
                                                 "wid notify container2");
        wid_set_tl_br(wid_notify_container, tl, br);
    }

    {
        int32_t row = 0;
        int32_t h = 0;
        widp child;

        TREE_WALK(d, n) {
            fpoint tl = {0.0f, 0.0f};
            fpoint br = {0.0f, 0.0f};

            tl.x = 0;
            tl.y = h;
            br.x = maxw;
            br.y = h + maxh;

            row++;

            /*
             * .5 line spacing for single newlines.
             */
            if (!strlen(n->line)) {
                h += maxh/2;
            } else {
                h += maxh;
            }

            child = wid_new_container(wid_notify_container,
                                      "wid notify container3");
            wid_set_tl_br(child, tl, br);
            wid_set_text(child, n->line);
            wid_set_text_lhs(child, true);
            wid_set_font(child, vsmall_font);
//            wid_set_text_outline(child, true);
            wid_set_color(child, WID_COLOR_TEXT, WHITE);
        }

        split_free(&d);
    }

    wid_set_bevel(wid_notify_window, 1);
    wid_set_rounded_small(wid_notify_window);

    wid_raise(wid_notify_window);

    wid_set_do_not_lower(wid_notify_window, true);

    wid_fade_in(wid_notify_window, wid_fade_delay);

    wid_set_ignore_events(wid_notify_window, true);

    return (wid_notify_window);
}

static void wid_notify_move_wids (void)
{
    /*
     * Move widgets to their new position.
     */
    uint32_t i;

    double h = 1.0;

    for (i = 0; i < nelems; i++) {
        wid_notify_elem *elem = &elems[i];

        h -= 
            wid_get_height(elem->w) / (double)global_config.video_gl_height;

        wid_move_to_pct_in(elem->w, X, h, 500);
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
        wid_notify("you kill the long named creatuee", NORMAL);
        wid_notify("there", NORMAL);
        wid_notify("you", NORMAL);
        wid_notify("oh-oh", WARNING);
        wid_notify("panic panic panic panic panic panic v panic v v panic panic panic v panicx x x x x x x x x x x x x x x x x x x panic dive dive!", SEVERE);
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
