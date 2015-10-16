/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <errno.h>

#include "main.h"
#include "wid_dirlist.h"
#include "wid.h"
#include "color.h"
#include "ttf.h"
#include "dir.h"
#include "string_util.h"

static const uint32_t MAX_ROWS = 6;
static const int32_t PAD_X = 80;
static const int32_t PAD_Y = 25;
static const int32_t BUTTON_PAD_X = 20;
static const char wid_dirlist_filename_input_str[] =
    "wid dirlist filename input";
static const char wid_dirlist_filelist_container_str[] =
    "wid dirlist filelist container";
static const char wid_dirlist_file_str[] =
    "wid dirlist file";

typedef void (*wid_dirlist_callback)(widp);

typedef struct {
    uint32_t maxw;
    uint32_t maxh;
    uint32_t orig_maxw;
    uint32_t rows;
    const char *include_suffix;
    const char *exclude_suffix;
    uint8_t include_dirs;
    char *dir;
    widp wid_dirlist_vert_scroll;
    widp wid_dirlist_horiz_scroll;
} wid_dirlist_context;

static void wid_dirlist_add_files(widp wid_dirlist_container,
                                  wid_dirlist_context *context,
                                  tree_root *d,
                                  const char *dir);

static void wid_dirlist_destroy (widp w)
{
    wid_dirlist_context *context;

    context = (typeof(context))
                    wid_get_client_context(wid_get_top_parent(w));

    myfree(context->dir);
    myfree(context);
}

static
uint8_t wid_dirlist_wid_noop (widp w, int32_t x, int32_t y, uint32_t button)
{
    return (true);
}

static uint8_t wid_dirlist_button_selected (widp w)
{
    wid_dirlist_callback callback;
    wid_dirlist_context *context;
    widp wid_input;
    char *tmp;

    callback = (typeof(callback)) wid_get_client_context(w);

    if (!callback) {
        return (false);
    }

    context = (typeof(context))
                    wid_get_client_context(wid_get_top_parent(w));
    if (!context) {
        ERR("no wid context found [%s]", wid_dirlist_filename_input_str);
    }

    wid_input = wid_find(wid_get_top_parent(w),
                         wid_dirlist_filename_input_str);
    if (!wid_input) {
        ERR("no wid input found [%s]", wid_dirlist_filename_input_str);
    }

    /*
     * Prepend the directory to the filename so we give the full name
     * for the callback.
     */
    tmp = dynprintf("%s%s", context->dir, wid_get_text(wid_input));
    wid_set_text(wid_input, tmp);
    myfree(tmp);

    (*callback)(wid_input);

    return (true);
}

static uint8_t wid_dirlist_file_event (widp w, int32_t x, int32_t y,
                                       uint32_t button)
{
    widp wid_input = wid_find(wid_get_top_parent(w),
                              wid_dirlist_filename_input_str);

    if (!wid_input) {
        ERR("no wid input found [%s]", wid_dirlist_filename_input_str);
    }

    wid_set_text(wid_input, wid_get_text(w));

    wid_set_cursor(wid_input, (uint32_t) strlen(wid_get_text(w)));

    return (true);
}

static uint8_t wid_dirlist_dir_event (widp w, int32_t x, int32_t y,
                                      uint32_t button)
{
    wid_dirlist_context *context;
    widp wid_input;
    char *new_dir;
    tree_root *d;
    char *tmp;

    context = (typeof(context))
                    wid_get_client_context(wid_get_top_parent(w));
    if (!context) {
        ERR("no wid context found [%s]", wid_dirlist_filename_input_str);
    }

    wid_input = wid_find(wid_get_top_parent(w),
                         wid_dirlist_filename_input_str);
    if (!wid_input) {
        ERR("no wid input found [%s]", wid_dirlist_filename_input_str);
    }

    new_dir = dupstr(wid_get_text(w), __FUNCTION__);

    /*
     * No change in directory ?
     */
    if (!strcasecmp(new_dir, ".")) {
        myfree(new_dir);
        return (true);
    }

    if (!strcasecmp(new_dir, "..")) {
        myfree(new_dir);
        new_dir = dupstr(context->dir, __FUNCTION__);

        /*
         * Get the parent directory, ending in a single /
         */
        tmp = dynprintf("%s" DSEP, dir_dotdot(new_dir));
        myfree(context->dir);
        context->dir = tmp;

        tmp = strsub(context->dir, DSEP DSEP, DSEP);
        myfree(context->dir);
        context->dir = tmp;
    } else {
        tmp = dynprintf("%s%s" DSEP, context->dir, new_dir, __FUNCTION__);
        myfree(context->dir);
        context->dir = tmp;
    }

    /*
     * Remove //
     */
    tmp = strsub(context->dir, DSEP DSEP, DSEP);
    myfree(context->dir);
    context->dir = tmp;

    wid_set_text(wid_input, "");
    wid_set_cursor(wid_input, (uint32_t) strlen(wid_get_text(w)));

    widp wid_dirlist_container = wid_find(wid_get_top_parent(w),
                                          wid_dirlist_filelist_container_str);
    if (!wid_dirlist_container) {
        myfree(new_dir);
        return (false);
    }

    tree_root *r = wid_get_children(wid_dirlist_container);
    if (!r) {
        myfree(new_dir);
        return (false);
    }

    {
        tree_node *n = tree_root_first(r);
        while (n) {
            widp child;

            child = (typeof(child)) n;

            wid_destroy(&child);

            n = tree_get_next(r, r->node, n);
        }
    }

    d = dirlist(context->dir,
                context->include_suffix,
                context->exclude_suffix,
                context->include_dirs,
                false /* include_ramdisk */);
    if (!d) {
        /*
         * Fail
         */
        char *popup_str = dynprintf("Cannot list dir %s: %s", context->dir,
                                  strerror(errno));

        MSG_BOX("%s", popup_str);
        myfree(popup_str);
        myfree(new_dir);

        return (true);
    }

    wid_dirlist_add_files(wid_dirlist_container, context, d, context->dir);
    dirlist_free(&d);

    wid_move_to_right(context->wid_dirlist_horiz_scroll);
    wid_move_to_top(context->wid_dirlist_vert_scroll);
    wid_update(wid_dirlist_container);
    myfree(new_dir);

    return (true);
}

static uint8_t wid_dirlist_button_event (widp w, int32_t x, int32_t y,
                                         uint32_t button)
{
    wid_dirlist_button_selected(w);

    return (true);
}

/*
 * Key down etc...
 */
static uint8_t wid_dirlist_receive_input (widp w, const SDL_KEYSYM *key)
{
    widp button;

    switch (key->sym) {
        case ' ':
        case SDLK_RETURN:
            button = wid_find(wid_get_top_parent(w), "ok");
            if (button) {
                wid_dirlist_button_selected(button);
                return (true);
            }

            button = wid_find(wid_get_top_parent(w), "yes");
            if (button) {
                wid_dirlist_button_selected(button);
                return (true);
            }

            if (wid_dirlist_button_selected(w)) {
                return (true);
            }

            break;

        case SDLK_ESCAPE: {
            button = wid_find(wid_get_top_parent(w), "cancel");
            if (button) {
                wid_dirlist_button_selected(button);
                return (true);
            }

            button = wid_find(wid_get_top_parent(w), "no");
            if (button) {
                wid_dirlist_button_selected(button);
                return (true);
            }

            button = wid_find(wid_get_top_parent(w), "close");
            if (button) {
                wid_dirlist_button_selected(button);
                return (true);
            }

            if (wid_dirlist_button_selected(w)) {
                return (true);
            }

            break;
        }

        case SDLK_TAB: {
                int32_t best = 0;
                int32_t match = 0;
                int32_t len = 0;

                widp container = wid_find(wid_get_top_parent(w),
                                          wid_dirlist_filelist_container_str);
                if (!container) {
                    return (false);
                }

                tree_root *r = wid_get_children(container);
                if (!r) {
                    return (false);
                }

                tree_node *n = tree_root_first(r);
                while (n) {
                    widp child;

                    child = (typeof(child)) n;

                    /*
                     * If a partial match, highlight the widget.
                     */
                    match = strncmp(wid_get_text(child), wid_get_text(w),
                                    strlen(wid_get_text(w)));

                    len = (int32_t) strlen(wid_get_text(child));

                    if (!match && (len > best)) {
                        wid_set_text(w, wid_get_text(child));

                        wid_set_cursor(w, len);

                        best = (int) strlen(wid_get_text(child));
                    }

                    n = tree_get_next(r, r->node, n);
                }
            }
            break;

        default:
            break;
    }

    /*
     * Feed to the general input handler
     */
    int32_t ret = wid_receive_input(w, key);

    /*
     * Update the dirlist to show only matching files.
     */
    widp container = wid_find(wid_get_top_parent(w),
                              wid_dirlist_filelist_container_str);
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
        double child_height;
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

static uint8_t wid_dirlist_ignore_input (widp w, const SDL_KEYSYM *key)
{
    return (true);
}

/*
 * Create the wid_dirlist
 */
static void wid_dirlist_add_files (widp wid_dirlist_container,
                                   wid_dirlist_context *context,
                                   tree_root *d,
                                   const char *dir)
{
    tree_file_node *n;
    char *parent_dir;
    char *tmp;

    int32_t row = 0;
    widp child;

    TREE_WALK(d, n) {

        if (!strcasecmp(n->tree.key, ".")) {
#ifndef ENABLE_DIR_TRAVERSAL_IN_WIDGETS
            if (/* DISABLES CODE */ (1)) {
                continue;
            }
#endif
            wid_set_tooltip(child, dir, 0 /* font */);
        }

        if (!strcasecmp(n->tree.key, "..")) {
#ifndef ENABLE_DIR_TRAVERSAL_IN_WIDGETS
            if (1) {
                continue;
            }
#endif
            /*
             * Get the parent directory, ending in a single /
             */
            tmp = dupstr(dir, __FUNCTION__);
            parent_dir = dynprintf("%s" DSEP, dir_dotdot(tmp));
            myfree(tmp);

            tmp = strsub(parent_dir, DSEP DSEP, DSEP);
            myfree(parent_dir);
            parent_dir = tmp;

            wid_set_tooltip(child, parent_dir, 0 /* font */);
            myfree(parent_dir);
        }
        fpoint tl = {0.0f, 0.0f};
        fpoint br = {0.0f, 0.0f};

        tl.x = 0;
        tl.y = (row * context->maxh);
        row++;
        br.x = context->orig_maxw;
        br.y = (row * context->maxh);

        child = wid_new_square_button(wid_dirlist_container,
                                      wid_dirlist_file_str);
        color c;

        wid_set_mode(child, WID_MODE_NORMAL);
        c = BLACK;
        c.a = 100;
        wid_set_color(child, WID_COLOR_BG, c);
        wid_set_color(child, WID_COLOR_TL, c);
        wid_set_color(child, WID_COLOR_BR, c);

        wid_set_mode(child, WID_MODE_OVER);
        c = STEELBLUE;
        c.a = 50;
        wid_set_color(child, WID_COLOR_BG, c);
        wid_set_color(child, WID_COLOR_TL, c);
        wid_set_color(child, WID_COLOR_BR, c);

        wid_set_mode(child, WID_MODE_ACTIVE);
        c = STEELBLUE;
        c.a = 60;
        wid_set_color(child, WID_COLOR_BG, c);
        wid_set_color(child, WID_COLOR_TL, c);
        wid_set_color(child, WID_COLOR_BR, c);

        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_bevelled(child, false);
        wid_set_tl_br(child, tl, br);
        wid_set_text(child, n->tree.key);
        wid_set_text_rhs(child, true);
        wid_set_font(child, small_font);
        wid_set_text_outline(child, true);

        wid_set_on_mouse_down(child, wid_dirlist_wid_noop);

        if (n->is_file) {
            wid_set_on_mouse_down(child, wid_dirlist_file_event);
        } else {
            wid_set_mode(child, WID_MODE_NORMAL);
            c = GRAY50;
            wid_set_color(child, WID_COLOR_TEXT, c);
            wid_set_on_mouse_down(child, wid_dirlist_dir_event);
        }
    }
}

/*
 * Create the wid_dirlist
 */
widp wid_dirlist (const char *dir,
                  const char *include_suffix,
                  const char *exclude_suffix,
                  const char *title,
                  float x, float y, int32_t args, ...)
{
    widp wid_dirlist_container;
    widp wid_dirlist_title;
    widp wid_dirlist_window;
    widp wid_dirlist_textbox;
    tree_file_node *n;
    tree_root *d;
    double title_h;
    uint32_t toth = 0;
    uint32_t maxbuttonw = 0;
    uint32_t maxbuttonh = 0;
    uint32_t button_y;
    va_list ap;
    const char *button_names[args];
    uint8_t need_horiz_scroll = false;
    uint8_t need_vert_scroll = false;
    wid_dirlist_context *context;

    context = (typeof(context))
                    myzalloc(sizeof(*context), "wid dirlist context");
    context->include_suffix = include_suffix;
    context->exclude_suffix = exclude_suffix;
    context->include_dirs = true;
    context->dir = dupstr(dir, __FUNCTION__);

    wid_dirlist_callback button_callback[args];
    memset(button_callback, 0, sizeof(button_callback));

    d = dirlist(context->dir,
                context->include_suffix,
                context->exclude_suffix,
                context->include_dirs,
                false /* include_ramdisk */);
    if (!d) {
        /*
         * Fail
         */
        char *popup_str = dynprintf("Cannot list dir %s: %s", dir,
                                  strerror(errno));

        MSG_BOX("%s", popup_str);
        myfree(popup_str);

        return (0);
    }

    {
        double w;

        fontp font = large_font;
        ttf_text_size(&font,
                      "TITLE", &w, &title_h, 0, 1.0f, 1.0f,
                      false /* fixed width */);

        toth += title_h;
    }

    {
        double w;
        double h;

        fontp font = small_font;
        ttf_text_size(&font,
                      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
                      &w, &h, 0, 1.0f, 1.0f,
                      false /* fixed width */);

        context->maxw = max(w, context->maxw);
        context->maxh = max(h, context->maxh);
    }

    { TREE_WALK(d, n) {
        double w;
        double h;

        fontp font = small_font;
        ttf_text_size(&font,
                      n->tree.key, &w, &h, 0, 1.0f, 1.0f,
                      false /* fixed width */);

        context->maxw = max(w, context->maxw);
        context->maxh = max(h, context->maxh);
        toth += h;
        context->rows++;
    } }

    /*
     * Check for the need to show scrollase.
     */
    if (context->rows > MAX_ROWS) {
        need_vert_scroll = true;
    }

    if (context->maxw > (uint32_t) ((global_config.video_gl_width * 3) / 4)) {
        need_horiz_scroll = true;
    }

    context->orig_maxw = context->maxw;
    context->maxw =
        min(context->maxw, (uint32_t) ((global_config.video_gl_width * 3) / 4));

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
            button_callback[args - n - 1] = va_arg(ap, wid_dirlist_callback);
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

            fontp font = med_font;
            ttf_text_size(&font, button_name, &w, &h, 0, 1.0f, 1.0f,
                          false /* fixed width */);

            w += BUTTON_PAD_X;

            maxbuttonw = max(w, maxbuttonw);
            maxbuttonh = max(h, maxbuttonh);
        }
    }

    context->maxw = max(context->maxw,
                        ((maxbuttonw + BUTTON_PAD_X) * args - BUTTON_PAD_X));

    {
        wid_dirlist_window = wid_new_rounded_window("wid_dirlist");
        wid_set_client_context(wid_dirlist_window, context);
        wid_set_on_destroy(wid_dirlist_window, wid_dirlist_destroy);

        wid_set_color(wid_dirlist_window, WID_COLOR_TEXT, WHITE);

        color c = BLACK;
        c.a = 200;
        wid_set_color(wid_dirlist_window, WID_COLOR_BG, c);

        c = STEELBLUE;
        c.a = 150;
        wid_set_color(wid_dirlist_window, WID_COLOR_TL, c);
        wid_set_color(wid_dirlist_window, WID_COLOR_BR, c);
        wid_set_bevel(wid_dirlist_window, 4);

        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        br.x += context->maxw;

        if (context->rows > MAX_ROWS) {
            br.y += context->maxh * MAX_ROWS;
        } else {
            br.y += toth;
        }

        br.x += PAD_X;

        /*
         * Space for input box.
         */
        br.y += maxbuttonh;
        br.y += PAD_Y/2;

        button_y = br.y;
        br.y += maxbuttonh;
        br.y += PAD_Y/2;

        if (title) {
            /*
             * Add space for title.
             */
            br.y += title_h;
            button_y += title_h;
        }

        wid_set_tl_br(wid_dirlist_window, tl, br);
    }

    if (title) {
        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        br.x += context->maxw + PAD_X;
        br.y += title_h;

        wid_dirlist_title = wid_new_container(wid_dirlist_window,
                                              "wid dirlist container1");
        wid_set_tl_br(wid_dirlist_title, tl, br);

        wid_set_font(wid_dirlist_title, large_font);
        wid_set_text(wid_dirlist_title, title);
    }

    {
        fpoint tl = {0, 0};
        fpoint br = {0, 0};

        br.x = context->maxw;

        if (context->rows > MAX_ROWS) {
            br.y += context->maxh * MAX_ROWS;
        } else {
            br.y += toth;
        }

        tl.x += PAD_X/2;
        br.x += PAD_X/2;

        if (title) {
            /*
             * Add space for title.
             */
            tl.y += title_h;
            br.y += title_h;
        }

        wid_dirlist_container =
                        wid_new_container(wid_dirlist_window,
                                          wid_dirlist_filelist_container_str);
        wid_set_tl_br(wid_dirlist_container, tl, br);
    }

    /*
     * Individual files
     */
    wid_dirlist_add_files(wid_dirlist_container, context, d, dir);
    dirlist_free(&d);

    {
        widp child;

        fpoint tl = {0.0f, 0.0f};
        fpoint br = {0.0f, 0.0f};

        tl.x = PAD_X / 2;
        tl.y = button_y - maxbuttonh - PAD_Y / 2;
        br.x = context->maxw + PAD_X / 2;
        br.y = tl.y + maxbuttonh;

        child = wid_new_square_button(wid_dirlist_window,
                                      wid_dirlist_filename_input_str);

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

        wid_set_on_key_down(child, wid_dirlist_receive_input);
        wid_set_on_key_up(child, wid_dirlist_ignore_input);
        wid_set_show_cursor(child, true);
        wid_set_focusable(child, args + 2);

        wid_dirlist_textbox = child;
    }

    {
        int32_t x = 0;
        int32_t n = args;
        int32_t focus_order = args + 1;

        x += context->maxw;
        x += PAD_X/2;

        while (n--) {
            double w;
            double h;
            fontp font = med_font;

            const char *button_name = button_names[n];
            ttf_text_size(&font, button_name, &w, &h, 0, 1.0f, 1.0f,
                          false /* fixed width */);

            widp child;
            child = wid_new_rounded_small_button(wid_dirlist_window,
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
            wid_set_font(child, font);

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
            c.a = 200;
            wid_set_mode(child, WID_MODE_NORMAL);
            wid_set_color(child, WID_COLOR_BG, c);

            c.a = 255;
            wid_set_mode(child, WID_MODE_OVER);
            wid_set_color(child, WID_COLOR_BG, c);

            wid_set_mode(child, WID_MODE_FOCUS);

            wid_set_mode(child, WID_MODE_NORMAL);
            wid_set_focusable(child, focus_order--);

            wid_set_on_mouse_down(child, wid_dirlist_button_event);

            wid_set_client_context(child, (void*)button_callback[n]);
        }
    }

    context->wid_dirlist_vert_scroll =
        wid_new_vert_scroll_bar(wid_dirlist_window, wid_dirlist_container);

    context->wid_dirlist_horiz_scroll =
        wid_new_horiz_scroll_bar(wid_dirlist_window, wid_dirlist_container);

    if (need_vert_scroll) {
        wid_visible(wid_get_parent(context->wid_dirlist_vert_scroll),
                    wid_visible_delay);
        wid_visible(context->wid_dirlist_vert_scroll,
                    wid_visible_delay);
    }

    if (need_horiz_scroll) {
        wid_visible(wid_get_parent(context->wid_dirlist_horiz_scroll),
                    wid_visible_delay);
        wid_visible(context->wid_dirlist_horiz_scroll,
                    wid_visible_delay);
    }

    wid_move_to_right(context->wid_dirlist_horiz_scroll);
    wid_move_to_top(context->wid_dirlist_vert_scroll);

    wid_fade_in(wid_dirlist_window, wid_fade_delay);
    wid_move_to_pct_centered(wid_dirlist_window, x, y);
    wid_raise(wid_dirlist_window);

    wid_update(wid_dirlist_window);
    wid_focus_lock(wid_dirlist_window);

    wid_set_tex(wid_dirlist_window, 0, "window_gothic");
    wid_set_square(wid_dirlist_window);

    wid_move_to_pct_centered(wid_dirlist_window, 0.5, 0.5 - 1.0);
    wid_move_to_pct_centered_in(wid_dirlist_window, 0.5, 0.5, 200);

    return (wid_dirlist_textbox);
}
