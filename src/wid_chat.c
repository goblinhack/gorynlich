/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */


#include "main.h"
#include "wid_chat.h"
#include "wid.h"
#include "color.h"
#include "ttf.h"

static int32_t wid_chat_inited;
static int32_t wid_chat_exiting;
static void wid_chat_wid_create(void);

static widp wid_chat_container;
static widp wid_chat_vert_scroll;
static widp wid_chat_horiz_scroll;

widp wid_chat_input_line;

static float wid_chat_line_height = 0.140f;
static float wid_chat_max_line_height = 0.140f;
static tree_root *tree_wid_chat;

widp wid_chat_window;

typedef struct tree_wid_chat_node_ {
    tree_key_int tree;
    char *buf;
} tree_wid_chat_node;

void wid_chat_fini (void)
{
    tree_wid_chat_node *node;

    FINI_LOG("%s", __FUNCTION__);

    wid_chat_exiting = true;

    if (wid_chat_inited) {
        wid_chat_inited = false;
    }

    /*
     * Flush the logs now the console exists.
     */
    if (tree_wid_chat) {
        TREE_WALK(tree_wid_chat, node) {
            myfree(node->buf);
            tree_remove(tree_wid_chat, &node->tree.node);
            myfree(node);
        }

        myfree(tree_wid_chat);
        tree_wid_chat = 0;
    }
}

uint8_t wid_chat_init (void)
{
    wid_chat_inited = true;

    wid_chat_wid_create();

    return (true);
}

/*
 * Scroll back to the bottom of the screen.
 */
static void wid_chat_reset_scroll (void)
{
    if (!wid_chat_vert_scroll) {
        return;
    }

    wid_move_to_bottom(wid_chat_vert_scroll);
}

/*
 * Log a message to the console
 */
void wid_chat_log (const char *s)
{
    static int32_t log_wid_chat_buffered_lines;
    tree_wid_chat_node *node;

    if (wid_chat_exiting) {
        return;
    }

    wid_chat_reset_scroll();

    /*
     * Before the console is ready, we buffer the logs.
     */
    if (!wid_chat_input_line) {
        if (!tree_wid_chat) {
            tree_wid_chat = tree_alloc(TREE_KEY_INTEGER,
                                          "TREE ROOT: wid_chat lines");
        }

        node = (typeof(node)) myzalloc(sizeof(*node),
                                       "TREE NODE: wid_chat log");
        node->tree.key = log_wid_chat_buffered_lines++;
        node->buf = dupstr(s, "TREE KEY: wid_chat log");

        if (!tree_insert(tree_wid_chat, &node->tree.node)) {
            ERR("insert log %s fail", s);
        }

        return;
    }

    /*
     * Flush the logs now the console exists.
     */
    if (tree_wid_chat) {
        TREE_WALK(tree_wid_chat, node) {
            wid_scroll_with_input(wid_chat_input_line, node->buf);
            myfree(node->buf);
            tree_remove(tree_wid_chat, &node->tree.node);
            myfree(node);
        }

        myfree(tree_wid_chat);
        tree_wid_chat = 0;
    }

    wid_scroll_with_input(wid_chat_input_line, s);
}

/*
 * Key down etc...
 */
static uint8_t wid_chat_receive_input (widp w, const SDL_KEYSYM *key)
{
    wid_chat_reset_scroll();

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
static void wid_chat_wid_create (void)
{
    fontp font = small_font;

    {
        fpoint tl = {0.8, 0.3f};
        fpoint br = {1.0f, 1.0f};
        color c;

        wid_chat_window = wid_new_square_window("wid_chat");

        c = BLACK;
        c.a = 0;
        wid_set_no_shape(wid_chat_window);

        wid_set_mode(wid_chat_window, WID_MODE_NORMAL);
        wid_set_color(wid_chat_window, WID_COLOR_TL, c);
        wid_set_color(wid_chat_window, WID_COLOR_BG, c);
        wid_set_color(wid_chat_window, WID_COLOR_BR, c);
        wid_set_color(wid_chat_window, WID_COLOR_TEXT, CHAT_CURSOR_COLOR);
        wid_set_font(wid_chat_window, font);
        wid_set_name(wid_chat_window, "wid_chat window");

        wid_set_text_bot(wid_chat_window, true);
        wid_set_text_lhs(wid_chat_window, true);
        wid_set_tl_br_pct(wid_chat_window, tl, br);

        fsize sz = {0.0f, 0.0f};
        wid_set_tex_tl(wid_chat_window, sz);

        fsize sz2 = {1.32f, 1.0f};
        wid_set_tex_br(wid_chat_window, sz2);
    }

    {
        fpoint tl = {0.0f, 0.0f};
        fpoint br = {1.0f, 1.0f};

        wid_chat_container = wid_new_container(wid_chat_window,
                                                  "wid console container");

        wid_set_tl_br_pct(wid_chat_container, tl, br);
        wid_set_text_lhs(wid_chat_container, true);
        wid_set_text_bot(wid_chat_container, true);
    }

    {
        double w;
        double h;

        ttf_text_size(&font,
                      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
                      &w, &h, 0, 1.0f, 1.0f,
                      true /* fixed width */);

        wid_chat_max_line_height =
                        (float)h / (float)wid_get_height(wid_chat_container);
    }

    {
        int32_t row;
        float row_bottom = 1.0f;

        widp child = 0;
        widp prev = 0;

        for (row = 0; row < CHAT_HEIGHT; row++) {

            fpoint tl = {
                0.0f,
                row_bottom - (wid_chat_line_height * (float)(row+1))
            };

            fpoint br = {
                2.0f,
                row_bottom - (wid_chat_line_height * ((float)row))
            };

            if (row != 0) {
                tl.y -= wid_chat_max_line_height * 0.4;
                br.y -= wid_chat_max_line_height * 0.4;
            } else {
                tl.y -= wid_chat_max_line_height;
            }

            child = wid_new_container(wid_chat_container, "");

            wid_set_tl_br_pct(child, tl, br);
            wid_set_text_lhs(child, true);
            wid_set_text_bot(child, true);
            wid_set_text_fixed_width(child, true);
            wid_set_font(child, font);

            wid_set_prev(child, prev);
            prev = child;

            if (row == 0) {
                wid_set_on_key_down(child, wid_chat_receive_input);
                wid_set_show_cursor(child, true);
                wid_set_name(child, "chat input");
                wid_chat_input_line = child;

                double chatw;
                double chath;

                ttf_text_size(&font, "chat> ", 
                              &chatw, &chath, 0, 1.0f, 1.0f,
                              false /* fixed width */);

                wid_move_delta(child, chatw, 0);

                widp prefix = wid_new_container(wid_chat_container, "");
                wid_set_tl_br_pct(prefix, tl, br);
                wid_set_text_lhs(prefix, true);
                wid_set_text_bot(prefix, true);
                wid_set_text_fixed_width(prefix, true);
                wid_set_font(prefix, font);
                wid_set_text(prefix, "chat> ");
            } else {
                wid_set_color(child, WID_COLOR_TEXT, CHAT_TEXT_COLOR);
                wid_set_name(child, "chat output");
            }
        }

        wid_raise(wid_chat_input_line);
    }

    wid_chat_vert_scroll =
        wid_new_vert_scroll_bar(wid_chat_window, wid_chat_container);
    wid_chat_horiz_scroll =
        wid_new_horiz_scroll_bar(wid_chat_window, wid_chat_container);

    wid_hide(wid_get_parent(wid_chat_vert_scroll), 0);
    wid_hide(wid_get_parent(wid_chat_horiz_scroll), 0);
    wid_hide(wid_chat_vert_scroll, 0);
    wid_hide(wid_chat_horiz_scroll, 0);
    wid_hide(wid_chat_window, 0);

    wid_update(wid_chat_window);
    wid_chat_reset_scroll();
}

void wid_chat_hide (void)
{
    wid_hide(wid_chat_window, 0);
}

void wid_chat_visible (void)
{
    if (global_config.server_current_players > 1) {
        wid_visible(wid_chat_window, 0);
    }
}
