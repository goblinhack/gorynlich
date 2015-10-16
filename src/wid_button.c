/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */


#include "wid.h"
#include "color.h"
#include "ttf.h"
#include "wid_button.h"

static const int32_t PAD_X = 80;
static const int32_t PAD_Y = 25;

/*
 * Create the wid_button
 */
static widp
wid_button_internal (const char *text, float x, float y, fontp font,
                     uint32_t text_scale)
{
    widp wid_button_window;
    double maxw = 0;
    double maxh = 0;
    int32_t mx, my;

    /*
     * Make sure the button does not obscure the mouse.
     */
    SDL_GetMouseState(&mx, &my);

    mx *= global_config.xscale;
    my *= global_config.yscale;

    if ((my >= (int32_t) ((global_config.video_gl_height / 5) * 2)) &&
        (my <= (int32_t) ((global_config.video_gl_height / 5) * 4))) {
        /*
         * Mouse in middle
         */
        y = 0.2;
    }

    if (!font) {
        font = small_font;
    }

    ttf_text_size(&font, text, &maxw, &maxh, 0, 1.0f, 1.0f,
                  false /* fixed width */);

    {
        wid_button_window = wid_new_square_window("wid_button");
        wid_set_text(wid_button_window, text);
        wid_set_font(wid_button_window, med_font);

        wid_set_color(wid_button_window, WID_COLOR_TEXT, WHITE);

        color c = STEELBLUE2;
        c.a = 200;
        wid_set_color(wid_button_window, WID_COLOR_BG, c);

        c = BLACK;
        c.a = 150;
        wid_set_color(wid_button_window, WID_COLOR_TL, c);
        wid_set_color(wid_button_window, WID_COLOR_BR, c);
        wid_set_text_outline(wid_button_window, true);

        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        br.x += maxw;
        br.y += maxh;

        br.x += PAD_X;
        br.y += PAD_Y;

        wid_set_tl_br(wid_button_window, tl, br);

        if (text_scale) {
            wid_scale_immediate(wid_button_window, text_scale);
        }
    }

    wid_move_to_pct_centered(wid_button_window, -0.5, y);
    wid_fade_in(wid_button_window, wid_fade_delay);
    wid_move_to_pct_centered(wid_button_window, x, y);
    wid_raise(wid_button_window);

    wid_update(wid_button_window);

    wid_set_do_not_lower(wid_button_window, true);

    return (wid_button_window);
}

widp wid_button (const char *text, float x, float y, fontp font)
{
    return (wid_button_internal(text, x, y, font, 1));
}

widp wid_button_simple (const char *text)
{
    widp w;

    w = wid_button(text,           /* body text */
                    0.5f, 0.5f,    /* x,y postition in percent */
                    0);            /* body font */

    return (w);
}

widp wid_button_transient (const char *text, uint32_t delay)
{
    widp w;

    if (!delay) {
        delay = 1500;
    }

    w = wid_button_internal(text,           /* body text */
                    0.5f, 0.5f,    /* x,y postition in percent */
                    large_font,     /* body font */
                    1               /* text scale */
                    );

    wid_destroy_in(w, delay);

    wid_raise(w);
    wid_set_do_not_lower(w, true);

    return (w);
}

widp wid_button_large_transient (const char *text, uint32_t delay)
{
    widp w;

    if (!delay) {
        delay = 1500;
    }

    w = wid_button_internal(text,           /* body text */
                            0.5f, 0.5f,     /* x,y postition in percent */
                            large_font,     /* body font */
                            2               /* text scale */
                            );

    wid_destroy_in(w, delay);

    wid_raise(w);
    wid_set_do_not_lower(w, true);

    return (w);
}

widp wid_button_large (const char *text)
{
    widp w;

    w = wid_button_internal(text,           /* body text */
                            0.5f, 0.5f,     /* x,y postition in percent */
                            large_font,     /* body font */
                            2               /* text scale */
                            );

    return (w);
}
