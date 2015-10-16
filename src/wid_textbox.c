/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid_textbox.h"
#include "wid.h"
#include "color.h"
#include "string_ext.h"
#include "ttf.h"

static const int32_t PAD_X = 80;
static const int32_t PAD_Y = 25;

/*
 * Create the wid_textbox
 */
static widp wid_textbox_internal (widp parent,
                  widp *textbox,
                  const char *text, float x, float y, fontp font,
                  uint8_t fixed_width)
{
    widp wid_textbox_container;
    widp wid_textbox_window;
    tree_string_split_node *n;
    widp firstchild;
    tree_root *d;
    uint32_t maxw = 0;
    uint32_t maxh = 0;
    uint32_t toth = 0;
    uint32_t rows = 0;
    uint32_t chars_per_line;
    uint32_t max_rows;

    firstchild = 0;

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
        double w;
        double h;

        ttf_text_size(&font, n->line, &w, &h, 0, 1.0f, 1.0f,
                      fixed_width);

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
        wid_textbox_window = wid_new_square_button(parent, "wid_textbox");

        wid_set_color(wid_textbox_window, WID_COLOR_TEXT, WHITE);

        color c = STEELBLUE2;
        c.a = 200;
        wid_set_color(wid_textbox_window, WID_COLOR_BG, c);

        c = STEELBLUE;
        c.a = 150;
        wid_set_color(wid_textbox_window, WID_COLOR_TL, c);
        wid_set_color(wid_textbox_window, WID_COLOR_BR, c);
        wid_set_bevel(wid_textbox_window, 4);

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

        wid_set_tl_br(wid_textbox_window, tl, br);
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

        wid_textbox_container = wid_new_container(wid_textbox_window,
                                                  "wid textbox container");
            wid_set_no_shape(wid_textbox_container);
        wid_set_tl_br(wid_textbox_container, tl, br);
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

            child = wid_new_container(wid_textbox_container,
                                      "wid textbox container2");
            wid_set_tl_br(child, tl, br);
            wid_set_text_fixed_width(child, fixed_width);
            wid_set_text(child, n->line);
            wid_set_font(child, font);
            wid_set_text_outline(child, true);
            wid_set_no_shape(child);

            if (!firstchild) {
                firstchild = child;
            }
        }

        split_free(&d);
    }

    wid_move_to_pct_centered(wid_textbox_window, x, y);

    wid_update(wid_textbox_window);

    if (textbox) {
        *textbox = firstchild;
    }

    return (wid_textbox_window);
}

/*
 * Create the wid_textbox
 */
widp wid_textbox (widp parent,
                  widp *textbox,
                  const char *text, float x, float y, fontp font)
{
    return (wid_textbox_internal(parent, textbox, text, x, y, font, false));
}

/*
 * Create the wid_textbox
 */
widp wid_textbox_fixed_width (widp parent,
                  widp *textbox,
                  const char *text, float x, float y, fontp font)
{
    return (wid_textbox_internal(parent, textbox, text, x, y, font, true));
}

widp wid_textbox_simple (widp parent,
                         widp *textbox,
                         const char *text, float x, float y)
{
    widp w;

    w = wid_textbox(parent,
                    textbox,
                    text,           /* body text */
                    0.5f, 0.5f,     /* x,y postition in percent */
                    0               /* body font */
                    );

    return (w);
}
