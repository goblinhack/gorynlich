/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include "slre.h"

#include "main.h"
#include "wid_console.h"
#include "command.h"
#include "wid.h"
#include "color.h"
#include "ttf.h"

static int32_t wid_console_inited;
static int32_t wid_console_exiting;
static void wid_console_wid_create(void);

static widp wid_console_container;
static widp wid_console_vert_scroll;
static widp wid_console_horiz_scroll;

widp wid_console_input_line;

static float wid_console_line_height = 0.050f;
static float wid_console_max_line_height = 0.050f;
static tree_root *tree_wid_console;

widp wid_console_window;

typedef struct tree_wid_console_node_ {
    tree_key_int tree;
    char *buf;
} tree_wid_console_node;

void wid_console_fini (void)
{
    tree_wid_console_node *node;

    FINI_LOG("%s", __FUNCTION__);

    wid_console_exiting = true;

    if (wid_console_inited) {
        wid_console_inited = false;
    }

    /*
     * Flush the logs now the console exists.
     */
    if (tree_wid_console) {
        TREE_WALK(tree_wid_console, node) {
            myfree(node->buf);
            tree_remove(tree_wid_console, &node->tree.node);
            myfree(node);
        }

        myfree(tree_wid_console);
        tree_wid_console = 0;
    }
}

uint8_t wid_console_init (void)
{
    wid_console_inited = true;

    command_add(debug_enable, "set debug [01]", "enable/disable debug mode");

    if (is_client) {
        if (!HEADLESS) {
            command_add(fps_enable, "set fps [01]", "frames per sec counter");
        }
    }

    command_add(sdl_user_exit, "quit", "exit the server or gam");

    wid_console_wid_create();

    return (true);
}

/*
 * Console initialization
 */
void wid_console_hello (void)
{
    CON(" ");
    CON("%%%%fg=red$          Welcome to the Gorynlich console!%%%%fg=reset$");
    CON(" ");
    CON("Press %%%%fg=red$<tab>%%%%fg=reset$ to complete commands.");

    if (!HEADLESS) {
        CON("Press %%%%fg=red$?%%%%fg=reset$ to show command options.");
    }
}

/*
 * Scroll back to the bottom of the screen.
 */
static void wid_console_reset_scroll (void)
{
    if (!wid_console_vert_scroll) {
        return;
    }

    wid_move_to_bottom(wid_console_vert_scroll);
}

/*
 * Log a message to the console
 */
void wid_console_log (const char *s)
{
    static int32_t log_wid_console_buffered_lines;
    tree_wid_console_node *node;

    if (wid_console_exiting) {
        return;
    }

    wid_console_reset_scroll();

    /*
     * Before the console is ready, we buffer the logs.
     */
    if (!wid_console_input_line) {
        if (!tree_wid_console) {
            tree_wid_console = tree_alloc(TREE_KEY_INTEGER,
                                          "TREE ROOT: wid_console lines");
        }

        node = (typeof(node)) myzalloc(sizeof(*node),
                                       "TREE NODE: wid_console log");
        node->tree.key = log_wid_console_buffered_lines++;
        node->buf = dupstr(s, "TREE KEY: wid_console log");

        if (!tree_insert(tree_wid_console, &node->tree.node)) {
            ERR("insert log %s fail", s);
        }

        return;
    }

    /*
     * Flush the logs now the console exists.
     */
    if (tree_wid_console) {
        TREE_WALK(tree_wid_console, node) {
            wid_scroll_with_input(wid_console_input_line, node->buf);
            myfree(node->buf);
            tree_remove(tree_wid_console, &node->tree.node);
            myfree(node);
        }

        myfree(tree_wid_console);
        tree_wid_console = 0;
    }

    wid_scroll_with_input(wid_console_input_line, s);
}

/*
 * Key down etc...
 */
static uint8_t wid_console_receive_input (widp w, const SDL_KEYSYM *key)
{
    wid_console_reset_scroll();

    switch (key->sym) {
        case SDLK_RETURN: {
            break;
        }

        default:
            break;
    }

    /*
     * Feed to the general input handler
     */
    return (wid_receive_input(w, key));
}

/*
 * Create the console
 */
static void wid_console_wid_create (void)
{
    fontp font = fixed_font;

    {
        fpoint tl = {0.0f, 0.0f};
        fpoint br = {1.0f, 0.5f};
        color c;

        wid_console_window = wid_new_square_window("wid_console");

        c = BLACK;
        c.a = 100;

        wid_set_mode(wid_console_window, WID_MODE_NORMAL);
        wid_set_color(wid_console_window, WID_COLOR_TL, c);
        wid_set_color(wid_console_window, WID_COLOR_BG, c);
        wid_set_color(wid_console_window, WID_COLOR_BR, c);
        wid_set_color(wid_console_window, WID_COLOR_TEXT,
                         CONSOLE_CURSOR_COLOR);
        wid_set_font(wid_console_window, font);
        wid_set_name(wid_console_window, "wid_console window");

        wid_set_text_bot(wid_console_window, true);
        wid_set_text_lhs(wid_console_window, true);
        wid_set_tl_br_pct(wid_console_window, tl, br);
    }

    {
        fpoint tl = {0.0f, 0.0f};
        fpoint br = {1.0f, 1.0f};

        wid_console_container = wid_new_container(wid_console_window,
                                                  "wid console container");

        wid_set_tl_br_pct(wid_console_container, tl, br);
        wid_set_text_lhs(wid_console_container, true);
        wid_set_text_bot(wid_console_container, true);
    }

    {
        double w;
        double h;

        ttf_text_size(&font,
                      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
                      &w, &h, 0, 1.0f, 1.0f,
                      true /* fixed width */);

        wid_console_max_line_height =
                        (float)h / (float)wid_get_height(wid_console_container);
    }

    {
        int32_t row;
        float row_bottom = 1.0f;

        widp child = 0;
        widp prev = 0;

        for (row = 0; row < CONSOLE_HEIGHT; row++) {

            fpoint tl = {
                0.0f,
                row_bottom - (wid_console_line_height * (float)(row+1))
            };

            fpoint br = {
                2.0f,
                row_bottom - (wid_console_line_height * ((float)row))
            };

            if (row != 0) {
                tl.y -= wid_console_max_line_height * 0.4;
                br.y -= wid_console_max_line_height * 0.4;
            } else {
                tl.y -= wid_console_max_line_height;
            }

            child = wid_new_container(wid_console_container, "");

            wid_set_tl_br_pct(child, tl, br);
            wid_set_text_lhs(child, true);
            wid_set_text_bot(child, true);
            wid_set_text_fixed_width(child, true);
            wid_set_text_outline(child, true);
            wid_set_font(child, font);

            wid_set_prev(child, prev);
            prev = child;

            if (row == 0) {
                wid_set_on_key_down(child, wid_console_receive_input);
                wid_set_show_cursor(child, true);
                wid_set_name(child, "console input");
                wid_set_focusable(child, 1);
                wid_console_input_line = child;
                wid_move_delta(child, 15, 0);

                widp prefix = wid_new_container(wid_console_container, "");
                wid_set_tl_br_pct(prefix, tl, br);
                wid_set_text_lhs(prefix, true);
                wid_set_text_bot(prefix, true);
                wid_set_text_fixed_width(prefix, true);
                wid_set_text_outline(prefix, true);
                wid_set_font(prefix, font);
                wid_set_text(prefix, ">");

            } else {
                wid_set_color(child, WID_COLOR_TEXT, CONSOLE_TEXT_COLOR);
                wid_set_name(child, "console output");
            }
        }

        wid_raise(wid_console_input_line);
    }

    wid_console_vert_scroll =
        wid_new_vert_scroll_bar(wid_console_window, wid_console_container);
    wid_console_horiz_scroll =
        wid_new_horiz_scroll_bar(wid_console_window, wid_console_container);

    wid_visible(wid_get_parent(wid_console_vert_scroll), 0);
    wid_visible(wid_get_parent(wid_console_horiz_scroll), 0);
    wid_visible(wid_console_vert_scroll, 0);
    wid_visible(wid_console_horiz_scroll, 0);

    wid_hide(wid_console_window, 0);

    color c = BLACK;
    c.a = 150;
    wid_set_color(wid_console_window, WID_COLOR_BG, c);
    wid_set_color(wid_console_window, WID_COLOR_TL, c);
    wid_set_color(wid_console_window, WID_COLOR_BR, c);
    wid_set_square(wid_console_window);
}
