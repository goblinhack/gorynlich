/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "string_ext.h"
#include "ttf.h"
#include "wid_popup.h"

static const int32_t PAD_X = 80;
static const int32_t PAD_Y = 45;
static const int32_t BUTTON_PAD_X = 30;

typedef void (*wid_popup_callback)(widp);

static uint8_t wid_popup_button_selected (widp w)
{
    wid_popup_callback callback;

    callback = (typeof(callback)) wid_get_client_context(w);

    if (!callback) {
        return (false);
    }

    (*callback)(w);

    return (true);
}

static uint8_t wid_popup_wid_mouse_event (widp w, int32_t x, int32_t y,
                                          uint32_t button)
{
    wid_popup_button_selected(w);

    return (true);
}

static uint8_t wid_popup_wid_key_event (widp w, const SDL_KEYSYM *key)
{
    widp button;
    widp focus;

    switch (key->sym) {
        case ' ':
        case 'y':
        case SDLK_RETURN:

            focus = wid_get_current_focus();
            if (focus) {
                if (wid_get_top_parent(w) == wid_get_top_parent(focus)) {
                    wid_popup_button_selected(w);
                    return (true);
                }
            }

            button = wid_find(wid_get_top_parent(w), "ok");
            if (button) {
                wid_popup_button_selected(button);
                return (true);
            }

            button = wid_find(wid_get_top_parent(w), "yes");
            if (button) {
                wid_popup_button_selected(button);
                return (true);
            }

            if (wid_popup_button_selected(w)) {
                return (true);
            }

            break;

        case SDLK_ESCAPE:
        case 'n':
        case 'b':
            button = wid_find(wid_get_top_parent(w), "cancel");
            if (button) {
                wid_popup_button_selected(button);
                return (true);
            }

            button = wid_find(wid_get_top_parent(w), "no");
            if (button) {
                wid_popup_button_selected(button);
                return (true);
            }

            button = wid_find(wid_get_top_parent(w), "close");
            if (button) {
                wid_popup_button_selected(button);
                return (true);
            }

            if (wid_popup_button_selected(w)) {
                return (true);
            }

            break;

        default:
            break;
    }

    return (false);
}

/*
 * Create the wid_popup
 */
widp wid_popup (const char *text, const char *title,
                float x, float y,
                fontp title_font,
                fontp body_font,
                fontp button_font,
                int32_t args, ...)
{
    widp wid_popup_horiz_scroll;
    widp wid_popup_vert_scroll;
    widp wid_popup_container;
    widp wid_popup_title;
    widp wid_popup_window;
    tree_string_split_node *n;
    tree_root *d;
    double titlew = 0;
    double titleh = 0;
    uint32_t maxw = 0;
    uint32_t maxh = 0;
    uint32_t toth = 0;
    uint32_t maxbuttonw = 0;
    uint32_t maxbuttonh = 0;
    uint32_t rows = 0;
    uint32_t button_y;
    va_list ap;
    const char *button_names[args];
    uint32_t chars_per_line;
    uint32_t max_rows;

    if (!title_font) {
        title_font = small_font;
    }

    if (!body_font) {
        body_font = small_font;
    }

    if (!button_font) {
        button_font = small_font;
    }

    if (body_font == large_font) {
        chars_per_line = 18;
        max_rows = 100;
    } else if (body_font == med_font) {
        chars_per_line = 50;
        max_rows = 8;
    } else {
        chars_per_line = 60;
        max_rows = 25;
    }

    d = split(text, chars_per_line);

    wid_popup_callback button_callback[args];
    memset(button_callback, 0, sizeof(button_callback));

    /*
     * Get some rough sizes for the title font.
     */
    if (title) {
        ttf_text_size(&title_font, title, &titlew, &titleh, 0, 1.0f, 1.0f,
                      false /* fixed width */);
    }

    /*
     * Get some rough sizes for the font.
     */
    { TREE_WALK(d, n) {
        double w;
        double h;

        ttf_text_size(&body_font, n->line, &w, &h, 0, 1.0f, 1.0f,
                      false /* fixed width */);

        /*
         * .5 line spacing for single newlines.
         */
        if (!strlen(n->line)) {
            if (!maxh) {
                ttf_text_size(&body_font, "Test", &w, &h, 0, 1.0f, 1.0f,
                              false /* fixed width */);

                maxh = h;
            }

            h = maxh/2;
        }

        maxw = max(w, maxw);
        maxh = max(h, maxh);

        toth += h;
        rows++;
    } }

    maxw = (((double)maxw) * 1.2);

    {
        int32_t n = args;

        while (n--) {
            button_names[n] = 0;
        }
    }

    {
        va_start(ap, args);

        int32_t n = args;

        while (n--) {
            button_names[args - n - 1] = va_arg(ap, const char*);
            button_callback[args - n - 1] = va_arg(ap, wid_popup_callback);
        }

        va_end(ap);
    }

    {
        int32_t n = args;

        while (n--) {
            double w;
            double h;

            const char *button_name = button_names[n];

            if (!button_name){
                button_name = "<bug>";
            }

            ttf_text_size(&button_font, button_name, &w, &h, 0, 1.0f, 1.0f,
                          false /* fixed width */);

            w += BUTTON_PAD_X;
            h += h / 2;

            maxbuttonw = max(w, maxbuttonw);
            maxbuttonh = max(h, maxbuttonh);
            maxbuttonh += maxbuttonh / 2;
        }
    }

    if (args) {
        maxw = max(maxw, ((maxbuttonw + BUTTON_PAD_X) * args - BUTTON_PAD_X));
    }

    {
        wid_popup_window = wid_new_rounded_window("wid_popup");

        wid_set_color(wid_popup_window, WID_COLOR_TEXT, WHITE);

        color c = WHITE;

        /*
         * Make this too weak and the credit/about screen looks odd
         */
        c.a = 255;

        wid_set_color(wid_popup_window, WID_COLOR_BG, c);
        wid_set_color(wid_popup_window, WID_COLOR_TL, c);
        wid_set_color(wid_popup_window, WID_COLOR_BR, c);

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
        button_y = br.y;
        br.y += maxbuttonh;
        br.y += PAD_Y;

        if (title) {
            /*
             * Add space for title.
             */
            br.y += maxh;
            button_y += maxh;
        }

        wid_set_tl_br(wid_popup_window, tl, br);
    }

    if (title) {
        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        br.x += maxw + PAD_X;
        br.y += titleh;

        tl.y += PAD_Y * 0.8;
        br.y += PAD_Y * 0.8;

        wid_popup_title = wid_new_container(wid_popup_window,
                                            "wid popup container");
        wid_set_tl_br(wid_popup_title, tl, br);

        wid_set_text(wid_popup_title, title);
        wid_set_font(wid_popup_title, title_font);
        wid_set_text_outline(wid_popup_title, false);
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
        br.y += PAD_Y/4;

        tl.y += PAD_Y * 0.8;
        br.y += PAD_Y * 0.8;

        wid_popup_container = wid_new_container(wid_popup_window,
                                                "wid popup container2");
        wid_set_tl_br(wid_popup_container, tl, br);

        wid_set_color(wid_popup_container, WID_COLOR_TEXT, WHITE);

#ifdef OUTLINE_FOR_TEXT
        color c;
        c = WHITE;
        c.a = 0;
        wid_set_color(wid_popup_container, WID_COLOR_BG, c);

        c = WHITE;
        c.a = 50;
        wid_set_color(wid_popup_container, WID_COLOR_TL, c);

        c = WHITE;
        c.a = 10;
        wid_set_color(wid_popup_container, WID_COLOR_BR, c);

        wid_set_font(wid_popup_container, body_font);
        wid_set_square(wid_popup_container);
        wid_set_bevel(wid_popup_container, 2);
        wid_set_bevelled(wid_popup_container, true);
#endif
    }

    {
        int32_t row = 0;
        widp child;
        int32_t h = 0;

        {

            fpoint tl = {0.0f, 0.0f};
            fpoint br = {0.0f, 0.0f};

            tl.x = 0;
            tl.y = h;
            row++;
            br.x = maxw;
            br.y = maxh / 2;
                h += maxh/2;

            child = wid_new_container(wid_popup_container,
                                          "wid popup container3");

            wid_set_no_shape(child);
            wid_set_tl_br(child, tl, br);
            wid_set_text(child, "");
            wid_set_font(child, body_font);
            wid_set_text_outline(child, true);
        }

        TREE_WALK(d, n) {
            fpoint tl = {0.0f, 0.0f};
            fpoint br = {0.0f, 0.0f};

            tl.x = 0;
            tl.y = h;
            row++;
            br.x = maxw;
            br.y = h + maxh;

            child = wid_new_container(wid_popup_container,
                                          "wid popup container3");

            wid_set_no_shape(child);
            wid_set_tl_br(child, tl, br);
            wid_set_text(child, n->line);
            wid_set_font(child, body_font);
            wid_set_text_outline(child, false);

            /*
             * .5 line spacing for single newlines.
             */
            if (!strlen(n->line)) {
                h += maxh/2;
            } else {
                h += maxh;
            }
        }

        split_free(&d);
    }

    if (1)
    {
        int32_t x = 0;
        int32_t n = args;
        int32_t focus_order = args + 1;

        x += maxw;
        x += PAD_X/2;

        while (n--) {
            double w;
            double h;

            const char *button_name = button_names[n];
            ttf_text_size(&body_font, button_name, &w, &h, 0, 1.0f, 1.0f,
                          false /* fixed width */);

            widp child;
            child = wid_new_rounded_small_button(wid_popup_window,
                                                 button_name);

            fpoint tl;
            fpoint br;

            tl.y = button_y;
            tl.y += PAD_Y / 3;

            br.y = tl.y + maxbuttonh;
            br.x = x;
            tl.x = br.x - maxbuttonw;

            x = tl.x;
            x -= BUTTON_PAD_X;

            wid_set_tl_br(child, tl, br);
            wid_set_text(child, button_name);
            wid_set_text_centerx(child, true);
            wid_set_font(child, button_font);

            color c;
            if (focus_order == 1) {
                c = DARKGREEN;
                wid_set_tex(child, 0, "button_black");
            } else if (focus_order == 2) {
                c = STEELBLUE2;
                wid_set_tex(child, 0, "button_black");
            } else if (focus_order == 3) {
                c = STEELBLUE;
                wid_set_tex(child, 0, "button_black");
            } else {
                wid_set_tex(child, 0, "button_black");
                c = GRAY;
            }

            wid_set_square(child);

            c = WHITE;
            c.a = 255;
            wid_set_mode(child, WID_MODE_NORMAL);
            wid_set_color(child, WID_COLOR_BG, c);

            c.a = 255;
            wid_set_mode(child, WID_MODE_OVER);
            wid_set_color(child, WID_COLOR_BG, c);

            wid_set_mode(child, WID_MODE_FOCUS);

            wid_set_mode(child, WID_MODE_NORMAL);
            wid_set_focusable(child, focus_order--);

            wid_set_on_mouse_down(child, wid_popup_wid_mouse_event);
            wid_set_on_key_down(child, wid_popup_wid_key_event);

            wid_set_client_context(child, (void*)button_callback[n]);
        }
    }

    wid_popup_vert_scroll =
        wid_new_vert_scroll_bar(wid_popup_window, wid_popup_container);
    wid_popup_horiz_scroll =
        wid_new_horiz_scroll_bar(wid_popup_window, wid_popup_container);

#if 0
    if (rows > max_rows) {
        wid_visible(wid_get_parent(wid_popup_vert_scroll), wid_visible_delay);
        wid_visible(wid_popup_vert_scroll, wid_visible_delay);
    } else {
#endif
        wid_hide(wid_get_parent(wid_popup_vert_scroll), 0);
        wid_hide(wid_popup_vert_scroll, 0);
#if 0
    }
#endif

    wid_hide(wid_get_parent(wid_popup_horiz_scroll), 0);
    wid_hide(wid_popup_horiz_scroll, 0);

    wid_move_to_pct_centered(wid_popup_window, x, y - 1.0);
    wid_move_to_pct_centered_in(wid_popup_window, x, y, 500);

    wid_raise(wid_popup_window);

    /*
     * If there are some buttons then lock the focus and this then makes stuff 
     * fade out in the background.
     */
    if (args) {
        wid_focus_lock(wid_popup_window);
    }

    return (wid_popup_window);
}

static void wid_popup_error_close (widp wid)
{
    wid = wid_get_top_parent(wid);

    wid_destroy(&wid);
}

static uint8_t wid_popup_error_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'q':
        case SDLK_ESCAPE:
            wid_popup_error_close(w);
            return (true);

        default:
            break;
    }

    return (false);
}

widp wid_popup_error (const char *text)
{
    widp w;

    if (!sdl_init_video) {
        return (0);
    }

    w = wid_popup(text ? text : "", /* body text */
                  "",               /* title */
                  0.5f, 0.5f,       /* x,y postition in percent */
                  0,                /* title font */
                  0,                /* body font */
                  0,                /* button font */
                  1,                /* number args */
                  "error", wid_popup_error_close);

    wid_set_on_key_down(w, wid_popup_error_key_event);

    wid_set_tex(w, 0, "gothic_wide");
    wid_set_square(w);

    wid_set_color(w, WID_COLOR_BG, BLACK);

    return (w);
}

static void wid_popup_ok_close (widp wid)
{
    wid = wid_get_top_parent(wid);

    wid_destroy(&wid);
}

static uint8_t wid_popup_ok_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case 'q':
        case SDLK_ESCAPE:
            wid_popup_ok_close(w);
            return (true);

        default:
            break;
    }

    return (false);
}

widp wid_popup_ok (const char *text)
{
    widp w;

    if (!sdl_init_video) {
        return (0);
    }

    w = wid_popup(text ? text : "", /* body text */
                  "",               /* title */
                  0.5f, 0.5f,       /* x,y postition in percent */
                  0,                /* title font */
                  0,                /* body font */
                  0,                /* button font */
                  1,                /* number args */
                  "ok", wid_popup_ok_close);

    wid_set_on_key_down(w, wid_popup_ok_key_event);

    wid_set_tex(w, 0, "gothic_wide");
    wid_set_square(w);

    wid_set_color(w, WID_COLOR_BG, BLACK);

    return (w);
}
