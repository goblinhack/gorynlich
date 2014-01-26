/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "string_ext.h"
#include "ttf.h"
#include "wid_tooltip.h"
#include "config.h"

static const int32_t PAD_X = 80;
static const int32_t PAD_Y = 25;

/*
 * Create the wid_tooltip
 */
widp wid_tooltip (const char *text, float x, float y, fontp font)
{
    widp wid_tooltip_container;
    widp wid_tooltip_window;
    tree_string_split_node *n;
    tree_root *d;
    uint32_t maxw = 0;
    uint32_t maxh = 0;
    uint32_t toth = 0;
    uint32_t rows = 0;
    uint32_t chars_per_line;
    uint32_t max_rows;
    int32_t mx, my;

    /*
     * Make sure the tooltip does not obscure the mouse.
     */
    SDL_GetMouseState(&mx, &my);

    mx *= global_config.xscale;
    mx *= global_config.yscale;

    if ((my >= (int32_t) ((global_config.video_gl_width / 7) * 2)) &&
        (my <= (int32_t) ((global_config.video_gl_height / 7) * 5))) {
        /*
         * Mouse at to 
         */
        y = 0.2;
    }

    if (!font) {
        font = small_font;
    }

    if (font == large_font) {
        chars_per_line = 18;
        max_rows = 6;
    } else if (font == med_font) {
        chars_per_line = 40;
        max_rows = 8;
    } else {
        chars_per_line = 50;
        max_rows = 12;
    }

    d = split(text, chars_per_line);

    /*
     * Get some rough sizes for the font.
     */
    { TREE_WALK(d, n) {
        uint32_t w;
        uint32_t h;

        ttf_text_size(font, n->line, &w, &h, 0, 1.0f, 1.0f,
                      true /* fixed width */);

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
        wid_tooltip_window = wid_new_window("wid_tooltip");

        color c = BLACK;
        wid_set_color(wid_tooltip_window, WID_COLOR_BG, c);
        wid_set_color(wid_tooltip_window, WID_COLOR_TL, c);
        wid_set_color(wid_tooltip_window, WID_COLOR_BR, c);

        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        br.x += maxw;

        if (rows > max_rows) {
            br.y += maxh * max_rows;
        } else {
            br.y += toth;
        }

        br.x += PAD_X;
        br.y += PAD_Y;

        wid_set_tl_br(wid_tooltip_window, tl, br);
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

        tl.x += PAD_X/2;
        tl.y += PAD_Y/2;
        br.x += PAD_X/2;
        br.y += PAD_Y/2;

        wid_tooltip_container = wid_new_container(wid_tooltip_window,
                                                  "wid tooltip container");
        wid_set_tl_br(wid_tooltip_container, tl, br);
    }

    {
        int32_t row = 0;
        widp child;
        int32_t h = 0;

        TREE_WALK(d, n) {
            fpoint tl = {0.0f, 0.0f};
            fpoint br = {0.0f, 0.0f};

            tl.x = 0;
            tl.y = h;
            row++;
            br.x = maxw;
            br.y = h + maxh;

            /*
             * .5 line spacing for single newlines.
             */
            if (!strlen(n->line)) {
                h += maxh/2;
            } else {
                h += maxh;
            }

            child = wid_new_container(wid_tooltip_container,
                                      "wid tooltip container2");
            wid_set_tl_br(child, tl, br);
            wid_set_text(child, n->line);
            wid_set_font(child, font);
            wid_set_text_fixed_width(child, true);
            wid_set_color(child, WID_COLOR_TEXT, WHITE);
        }

        split_free(&d);
    }

#ifdef SLIDING_TOOLTIP
    wid_move_to_pct_centered(wid_tooltip_window, -2.5, y);
    wid_fade_in(wid_tooltip_window, wid_fade_delay);
    wid_move_to_pct_centered_in(wid_tooltip_window, x, y, wid_swipe_delay);
#else
    wid_fade_in(wid_tooltip_window, wid_fade_delay);
    wid_move_to_pct_centered(wid_tooltip_window, x, y);
#endif
    wid_raise(wid_tooltip_window);

    wid_update(wid_tooltip_window);

    wid_set_do_not_lower(wid_tooltip_window, true);

    return (wid_tooltip_window);
}

widp wid_tooltip_simple (const char *text)
{
    widp w;

    w = wid_tooltip(text,           /* body text */
                    0.5f, 0.5f,     /* x,y postition in percent */
                    0               /* body font */
                    );

    return (w);
}

widp wid_tooltip_transient (const char *text, uint32_t delay)
{
    widp w;

    if (!delay) {
        delay = 2000;
    }

    w = wid_tooltip(text,           /* body text */
                    0.5f, 0.5f,     /* x,y postition in percent */
                    0               /* body font */
                    );

    wid_destroy_in(w, delay);

    return (w);
}

widp wid_tooltip_large_transient (const char *text, uint32_t delay)
{
    widp w;

    if (!delay) {
        delay = 2000;
    }

    w = wid_tooltip(text,           /* body text */
                    0.5f, 0.5f,     /* x,y postition in percent */
                    large_font      /* body font */
                    );

    wid_destroy_in(w, delay);

    return (w);
}
