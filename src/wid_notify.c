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

typedef struct {
    uint32_t created;
    widp notify_box_wid;
    char text[200];
} wid_notify_elem;

#define MAX_ELEMS 10
static uint32_t nelems;
static wid_notify_elem elems[MAX_ELEMS];

static double X = 0.675;
static uint32_t DELAY = 5000;

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

    chars_per_line = 25;
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
        color bg = BLACK;
        color fg = WHITE;

        switch (level) {
        case CHAT:
            bg = BLUE;
            fg = WHITE;
            break;
        case GENINFO:
            bg = BLUE;
            bg.a = 100;
            fg = YELLOW;
            break;
        case INFO:
            bg = WHITE;
            bg.a = 20;
            break;
        case WARNING:
            bg = ORANGE;
            fg = BLACK;
            break;
        case CRITICAL:
            bg = DARKRED;
            fg = RED;
            break;
        }

        wid_set_color(wid_notify_window, WID_COLOR_BG, bg);
        wid_set_color(wid_notify_window, WID_COLOR_TEXT, fg);

        color c = WHITE;
        c.a = 50;
        wid_set_color(wid_notify_window, WID_COLOR_TL, c);
        wid_set_color(wid_notify_window, WID_COLOR_BR, c);

        wid_move_to_pct(wid_notify_window, X + 0.2, 1.0);
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
            wid_set_text_outline(child, true);
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
            wid_get_height(elem->notify_box_wid) / 
                (double)global_config.video_gl_height;

        wid_move_to_pct_in(elem->notify_box_wid, X, h, 200);
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
    wid_destroy(&elem->notify_box_wid);
    nelems--;

    /*
     * Move the whole table down one element.
     */
    uint32_t i;

    for (i = 0; i < nelems; i++) {
        wid_notify_elem *e = &elems[i];
        wid_notify_elem *f = &elems[i+1];

        memcpy(e, f, sizeof(*e));
    }

    wid_notify_elem *e = &elems[i];
    memset(e, 0, sizeof(*e));

    wid_notify_move_wids();
}

widp wid_notify (uint32_t level, const char *text)
{
    widp w;

    if (nelems) {
        wid_notify_elem *e = &elems[nelems - 1];

        if (!strcmp(e->text, text)) {
            wid_set_mode(e->notify_box_wid, WID_MODE_ACTIVE);
            wid_set_color(e->notify_box_wid, WID_COLOR_BG, RED);

            return (e->notify_box_wid);
        }
    }

    w = wid_notify_internal(text, level);

    while (nelems >= MAX_ELEMS - 1) {
        wid_notify_scroll();
    }

    int32_t timestamp_now = time_update_time_milli();

    wid_notify_elem *e = &elems[nelems];
    e->created = timestamp_now;
    e->notify_box_wid = w;
    strncpy(e->text, text, sizeof(e->text) - 1);

    nelems++;

    wid_notify_move_wids();

    return (w);
}

void wid_notify_tick (void)
{
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
