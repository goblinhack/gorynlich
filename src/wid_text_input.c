/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <stdlib.h>
#include <errno.h>
#include <SDL.h>
#include "slre.h"

#include "main.h"
#include "wid_text_input.h"
#include "gl.h"
#include "command.h"
#include "wid.h"
#include "tex.h"
#include "color.h"
#include "ttf.h"
#include "string.h"
#include "wid_popup.h"
#include "config.h"

static const int32_t LINE_WIDTH = 25;
static const uint32_t MAX_ROWS = 6;
static const int32_t PAD_X = 80;
static const int32_t PAD_Y = 25;
static const int32_t BUTTON_PAD_X = 20;
static const int32_t BUTTON_PAD_Y = 10;
static const char wid_text_input_filename_input_str[] =
    "wid text_input filename input";
static const char wid_text_input_filelist_container_str[] =
    "wid text_input filelist container";
static const char wid_text_input_file_str[] =
    "wid text_input file";

typedef void (*wid_text_input_callback)(widp);

static
boolean wid_text_input_wid_noop (widp w, int32_t x, int32_t y, uint32_t button)
{
    return (true);
}

static boolean wid_text_input_button_selected (widp w)
{
    wid_text_input_callback callback;
    widp wid_input;

    callback = (typeof(callback)) wid_get_client_context(w);

    if (!callback) {
        return (false);
    }

    wid_input = wid_find(wid_get_top_parent(w),
                         wid_text_input_filename_input_str);
    if (!wid_input) {
        DIE("no wid input found [%s]", wid_text_input_filename_input_str);
    }

    (*callback)(wid_input);

    return (true);
}

static boolean wid_text_input_button_event (widp w, int32_t x, int32_t y,
                                         uint32_t button)
{
    wid_text_input_button_selected(w);

    return (true);
}

/*
 * Key down etc...
 */
static boolean wid_text_input_receive_input (widp w, const SDL_keysym *key)
{
    widp button;

    switch (key->sym) {
        case SDLK_RETURN:
            button = wid_find(wid_get_top_parent(w), "ok");
            if (button) {
                wid_text_input_button_selected(button);
                return (true);
            }

            button = wid_find(wid_get_top_parent(w), "yes");
            if (button) {
                wid_text_input_button_selected(button);
                return (true);
            }

            if (wid_text_input_button_selected(w)) {
                return (true);
            }

            break;

        case SDLK_ESCAPE: {
            button = wid_find(wid_get_top_parent(w), "cancel");
            if (button) {
                wid_text_input_button_selected(button);
                return (true);
            }

            button = wid_find(wid_get_top_parent(w), "no");
            if (button) {
                wid_text_input_button_selected(button);
                return (true);
            }

            button = wid_find(wid_get_top_parent(w), "close");
            if (button) {
                wid_text_input_button_selected(button);
                return (true);
            }

            if (wid_text_input_button_selected(w)) {
                return (true);
            }

            break;
        }

        default:
            break;
    }

    /*
     * Feed to the general input handler
     */
    int32_t ret = wid_receive_input(w, key);

    /*
     * Update the text_input to show only matching files.
     */
    widp container = wid_find(wid_get_top_parent(w),
                              wid_text_input_filelist_container_str);
    if (!container) {
        return (ret);
    }

    tree_root *r = wid_get_children(container);
    if (!r) {
        return (ret);
    }

    tree_node *n = tree_root_first(r);
    while (n) {
        fpoint container_tl;
        fpoint container_br;
        fpoint trough_tl;
        fpoint trough_br;
        float child_height;
        widp child;
        fpoint tl;
        fpoint br;

        child = (typeof(child)) n;

        /*
         * If a partial match, highlight the widget.
         */
        if (strncmp(wid_get_text(child), wid_get_text(w),
                    strlen(wid_get_text(w)))) {

            n = tree_get_next(r, r->node, n);
            continue;
        }

        wid_set_mode(child, WID_MODE_ACTIVE);

        /*
         * Find where the child is with respect to the parent.
         */
        wid_get_tl_br(child, &tl, &br);
        wid_get_tl_br(container, &container_tl, &container_br);

        /*
         * Compare within the overall height of all children.
         */
        wid_get_children_size(container, 0, &child_height);

        /*
         * What percentage of the way down.
         */
        float pct = (float)(tl.y - container_tl.y) / (float)child_height;

        /*
         * Now adjust the scrollbar.
         */
        widp scrollbar = wid_get_scrollbar_vert(container);

        /*
         * But we need the trough height.
         */
        wid_get_tl_br(wid_get_parent(scrollbar), &trough_tl, &trough_br);

        float trough_height =
                wid_get_height(wid_get_parent(scrollbar));

        /*
         * Now apply the same percentage as a wid move.
         */
        wid_move_to_abs_in(scrollbar, trough_tl.x,
                           trough_tl.y + pct * trough_height, 100);

        return (ret);
    }

    return (ret);
}

/*
 * Create the wid_text_input
 */
widp wid_text_input (const char *title, float x, float y, int32_t args, ...)
{
    widp wid_text_input_container;
    widp wid_text_input_title;
    widp wid_text_input_window;
    widp wid_text_input_textbox;
    uint32_t title_h;
    uint32_t toth = 0;
    uint32_t maxbuttonw = 0;
    uint32_t maxbuttonh = 0;
    uint32_t button_y;
    va_list ap;
    const char *button_names[args];
    uint32_t maxw;
    uint32_t maxh;

    wid_text_input_callback button_callback[args];

    {
        uint32_t w;

        ttf_text_size(large_font,
                      "TITLE", &w, &title_h, 0, 1.0f, 1.0f,
                      false /* fixed width */);

        toth += title_h;
    }

    {
        ttf_text_size(small_font,
                      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
                      &maxw, &maxh, 0, 1.0f, 1.0f,
                      false /* fixed width */);
    }

    maxw = min(maxw, (uint32_t) ((global_config.video_gl_width * 3) / 4));

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
            button_callback[args - n - 1] = va_arg(ap, wid_text_input_callback);
        }

        va_end(ap);
    }

    {
        int32_t n = args;

        while (n--) {
            uint32_t w;
            uint32_t h;

            const char *button_name = button_names[n];

            if (!button_name){
                button_name = "<bug>";
            }

            ttf_text_size(med_font, button_name, &w, &h, 0, 1.0f, 1.0f,
                          false /* fixed width */);

            w += BUTTON_PAD_X;

            maxbuttonw = max(w, maxbuttonw);
            maxbuttonh = max(h, maxbuttonh);
        }
    }

    {
        wid_text_input_window = wid_new_rounded_window("wid_text_input");

        wid_set_color(wid_text_input_window, WID_COLOR_TEXT, WHITE);

        color c = DARKRED;
        c.a = 200;
        wid_set_color(wid_text_input_window, WID_COLOR_BG, c);

        c = RED;
        c.a = 150;
        wid_set_color(wid_text_input_window, WID_COLOR_TL, c);
        wid_set_color(wid_text_input_window, WID_COLOR_BR, c);
        wid_set_bevel(wid_text_input_window, 4);

        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        br.x += maxw;
        br.y += toth;

        br.x += PAD_X;

        /*
         * Space for input box.
         */
        br.y += maxbuttonh;

        button_y = br.y;
        br.y += maxbuttonh;

        wid_set_tl_br(wid_text_input_window, tl, br);
    }

    if (title) {
        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        br.x += maxw + PAD_X;
        br.y += title_h;

        wid_text_input_title = wid_new_container(wid_text_input_window,
                                              "wid text_input container1");
        wid_set_tl_br(wid_text_input_title, tl, br);

        wid_set_font(wid_text_input_title, large_font);
        wid_set_text(wid_text_input_title, title);
    }

    {
        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        br.x = maxw;
        br.y += toth;

        tl.x += PAD_X/2;
        br.x += PAD_X/2;

        if (title) {
            /*
             * Add space for title.
             */
            tl.y += title_h;
            br.y += title_h;
        }

        wid_text_input_container =
                        wid_new_container(wid_text_input_window,
                                          wid_text_input_filelist_container_str);
        wid_set_tl_br(wid_text_input_container, tl, br);
    }

    {
        widp child;

        fpoint tl = {0.0f, 0.0f};
        fpoint br = {0.0f, 0.0f};

        tl.x = PAD_X / 2;
        tl.y = button_y - maxbuttonh - PAD_Y / 2;
        br.x = maxw + PAD_X / 2;
        br.y = tl.y + maxbuttonh;

        child = wid_new_square_button(wid_text_input_window,
                                      wid_text_input_filename_input_str);

        wid_set_color(child, WID_COLOR_BG, BLACK);
        wid_set_color(child, WID_COLOR_TL, WHITE);
        wid_set_color(child, WID_COLOR_BR, WHITE);

        color c;
        c = GREEN;

        wid_set_mode(child, WID_MODE_NORMAL);
        c.a = 100;
        wid_set_color(child, WID_COLOR_BG, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 50;
        wid_set_color(child, WID_COLOR_BG, c);

        wid_set_mode(child, WID_MODE_ACTIVE);
        c.a = 60;
        wid_set_color(child, WID_COLOR_BG, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_tl_br(child, tl, br);
        wid_set_text_lhs(child, true);
        wid_set_font(child, small_font);
        wid_set_text_outline(child, true);

        wid_set_on_key_down(child, wid_text_input_receive_input);
        wid_set_show_cursor(child, true);
        wid_set_focusable(child, args + 2);

        wid_text_input_textbox = child;
    }

    {
        int32_t x = 0;
        int32_t n = args;
        int32_t focus_order = args + 1;

        x += maxw;
        x += PAD_X/2;

        while (n--) {
            uint32_t w;
            uint32_t h;

            const char *button_name = button_names[n];
            ttf_text_size(med_font, button_name, &w, &h, 0, 1.0f, 1.0f,
                          false /* fixed width */);

            widp child;
            child = wid_new_rounded_small_button(wid_text_input_window,
                                                 button_name);

            fpoint tl;
            fpoint br;

            tl.y = button_y;
            br.y = tl.y + maxbuttonh;
            br.x = x;
            tl.x = br.x - maxbuttonw;

            x = tl.x;
            x -= BUTTON_PAD_X;

            wid_set_tl_br(child, tl, br);
            wid_set_text(child, button_name);
            wid_set_font(child, med_font);

            color c;
            if (focus_order == 1) {
                c = GREEN;
            } else if (focus_order == 2) {
                c = RED2;
            } else if (focus_order == 3) {
                c = CYAN;
            } else {
                c = GRAY;
            }

            wid_set_mode(child, WID_MODE_NORMAL);
            c.a = 100;
            wid_set_color(child, WID_COLOR_BG, c);

            wid_set_mode(child, WID_MODE_OVER);
            c.a = 250;
            wid_set_color(child, WID_COLOR_BG, c);

            wid_set_mode(child, WID_MODE_NORMAL);

            wid_set_focusable(child, focus_order--);

            wid_set_on_mouse_down(child, wid_text_input_wid_noop);
            wid_set_on_mouse_up(child, wid_text_input_button_event);

            wid_set_client_context(child, (void*)button_callback[n]);
        }
    }

    wid_move_to_pct_centered(wid_text_input_window, -2.5, y);
    wid_fade_in(wid_text_input_window, wid_fade_delay);
    wid_move_to_pct_centered_in(wid_text_input_window, x, y, wid_swipe_delay);
    wid_raise(wid_text_input_window);
    wid_focus_lock(wid_text_input_window);

    wid_update(wid_text_input_window);

    return (wid_text_input_textbox);
}

/*
 * Create the wid_text_input
 */
widp wid_large_text_input (const char *title, float x, float y, int32_t args, ...)
{
    widp wid_text_input_container;
    widp wid_text_input_title;
    widp wid_text_input_window;
    widp wid_text_input_textbox;
    uint32_t title_h;
    uint32_t toth = 0;
    uint32_t maxbuttonw = 0;
    uint32_t maxbuttonh = 0;
    uint32_t button_y;
    va_list ap;
    const char *button_names[args];
    uint32_t maxw;
    uint32_t maxh;

    wid_text_input_callback button_callback[args];

    {
        uint32_t w;

        ttf_text_size(large_font,
                      "TITLE", &w, &title_h, 0, 1.0f, 1.0f,
                      false /* fixed width */);

        toth += title_h;
    }

    {
        ttf_text_size(large_font,
                      "xxxxxxxxxxxxxxxx",
                      &maxw, &maxh, 0, 1.0f, 1.0f,
                      false /* fixed width */);
    }

    maxw = min(maxw, (uint32_t) ((global_config.video_gl_width * 3) / 4));

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
            button_callback[args - n - 1] = va_arg(ap, wid_text_input_callback);
        }

        va_end(ap);
    }

    {
        int32_t n = args;

        while (n--) {
            uint32_t w;
            uint32_t h;

            const char *button_name = button_names[n];

            if (!button_name){
                button_name = "<bug>";
            }

            ttf_text_size(med_font, button_name, &w, &h, 0, 1.0f, 1.0f,
                          false /* fixed width */);

            w += BUTTON_PAD_X;

            maxbuttonw = max(w, maxbuttonw);
            maxbuttonh = max(h, maxbuttonh);
        }
    }

    {
        wid_text_input_window = wid_new_rounded_window("wid_text_input");

        wid_set_color(wid_text_input_window, WID_COLOR_TEXT, WHITE);

        color c = DARKRED;
        c.a = 200;
        wid_set_color(wid_text_input_window, WID_COLOR_BG, c);

        c = RED;
        c.a = 150;
        wid_set_color(wid_text_input_window, WID_COLOR_TL, c);
        wid_set_color(wid_text_input_window, WID_COLOR_BR, c);
        wid_set_bevel(wid_text_input_window, 4);

        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        br.x += maxw;
        br.y += toth;
        br.y += toth;
        br.y += toth;

        br.x += PAD_X;

        /*
         * Space for input box.
         */
        br.y += maxbuttonh;

        button_y = br.y;
        br.y += maxbuttonh;

        wid_set_tl_br(wid_text_input_window, tl, br);
    }

    if (title) {
        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        br.x += maxw + PAD_X;
        br.y += title_h;
        tl.y += title_h/2;
        br.y += title_h/2;

        wid_text_input_title = wid_new_container(wid_text_input_window,
                                              "wid text_input container1");
        wid_set_tl_br(wid_text_input_title, tl, br);

        wid_set_font(wid_text_input_title, large_font);
        wid_set_text(wid_text_input_title, title);
    }

    {
        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        br.x = maxw;
        br.y += toth;

        tl.x += PAD_X/2;
        br.x += PAD_X/2;

        if (title) {
            /*
             * Add space for title.
             */
            tl.y += title_h;
            br.y += title_h;
        }

        wid_text_input_container =
                        wid_new_container(wid_text_input_window,
                                          wid_text_input_filelist_container_str);
        wid_set_tl_br(wid_text_input_container, tl, br);
    }

    {
        widp child;

        fpoint tl = {0.0f, 0.0f};
        fpoint br = {0.0f, 0.0f};

        tl.x = PAD_X / 2;
        tl.y = button_y - maxbuttonh - PAD_Y * 4;
        br.x = maxw + PAD_X / 2;
        br.y = tl.y + maxbuttonh * 4 - PAD_Y * 2;

        child = wid_new_square_button(wid_text_input_window,
                                      wid_text_input_filename_input_str);

        wid_set_color(child, WID_COLOR_BG, BLACK);
        wid_set_color(child, WID_COLOR_TL, RED);
        wid_set_color(child, WID_COLOR_BR, RED);

        color c;
        c = DARKRED;

        wid_set_mode(child, WID_MODE_NORMAL);
        c.a = 100;
        wid_set_color(child, WID_COLOR_BG, c);

        wid_set_mode(child, WID_MODE_OVER);
        c.a = 50;
        wid_set_color(child, WID_COLOR_BG, c);

        wid_set_mode(child, WID_MODE_ACTIVE);
        c.a = 60;
        wid_set_color(child, WID_COLOR_BG, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_color(child, WID_COLOR_TEXT, GREEN);

        wid_set_tl_br(child, tl, br);
        wid_set_font(child, large_font);
        wid_set_text_outline(child, true);

        wid_set_on_key_down(child, wid_text_input_receive_input);
        wid_set_show_cursor(child, true);
        wid_set_focusable(child, args + 2);

        wid_text_input_textbox = child;
    }

    {
        int32_t x = 0;
        int32_t n = args;
        int32_t focus_order = args + 1;

        x += maxw;
        x += PAD_X/2;

        while (n--) {
            uint32_t w;
            uint32_t h;

            const char *button_name = button_names[n];
            ttf_text_size(med_font, button_name, &w, &h, 0, 1.0f, 1.0f,
                          false /* fixed width */);

            widp child;
            child = wid_new_rounded_small_button(wid_text_input_window,
                                                 button_name);

            fpoint tl;
            fpoint br;

            tl.y = button_y;
            br.y = tl.y + maxbuttonh;
            br.x = x;
            tl.x = br.x - maxbuttonw;

            x = tl.x;
            x -= BUTTON_PAD_X;

            wid_set_tl_br(child, tl, br);
            wid_set_text(child, button_name);
            wid_set_font(child, med_font);

            color c;
            if (focus_order == 1) {
                c = GREEN;
            } else if (focus_order == 2) {
                c = RED2;
            } else if (focus_order == 3) {
                c = CYAN;
            } else {
                c = GRAY;
            }

            wid_set_mode(child, WID_MODE_NORMAL);
            c.a = 100;
            wid_set_color(child, WID_COLOR_BG, c);

            wid_set_mode(child, WID_MODE_OVER);
            c.a = 250;
            wid_set_color(child, WID_COLOR_BG, c);

            wid_set_mode(child, WID_MODE_NORMAL);

            wid_set_focusable(child, focus_order--);

            wid_set_on_mouse_down(child, wid_text_input_wid_noop);
            wid_set_on_mouse_up(child, wid_text_input_button_event);

            wid_set_client_context(child, (void*)button_callback[n]);
        }
    }

    wid_move_to_pct_centered(wid_text_input_window, -2.5, y);
    wid_fade_in(wid_text_input_window, wid_fade_delay);
    wid_move_to_pct_centered_in(wid_text_input_window, x, y, wid_swipe_delay);
    wid_raise(wid_text_input_window);
    wid_focus_lock(wid_text_input_window);

    wid_update(wid_text_input_window);

    return (wid_text_input_textbox);
}
