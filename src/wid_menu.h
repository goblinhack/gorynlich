/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

#define WID_MENU_MAX_ITEMS 100
#define WID_MENU_MAX_COLS  10

typedef void(*on_update_t)(widp, int focus);

typedef struct {
    /*
     * Parent widget
     */
    widp w;

    /*
     * Item currently in focus
     */
    int focus;

    /*
     * size of the menu
     */
    int items;

    /*
     * columns
     */
    int cols;

    /*
     * How large the focus line is
     */
    fontp focus_font;
    double focus_height;

    /*
     * How large the normal line is
     */
    fontp normal_font;
    double normal_height;

    /*
     * When the menu was made.
     */
    uint32_t created;

    /*
     * Items in the menu
     */
    widp buttons[WID_MENU_MAX_ITEMS][WID_MENU_MAX_COLS];

    /*
     * What to call on events
     */
    on_mouse_down_t event_handler[WID_MENU_MAX_ITEMS];

    /*
     * Percentage width of each column.
     */
    double col_width[WID_MENU_MAX_COLS];
    double total_col_width;

    /*
     * For items drawn as a bar and not text.
     */
    widp bar[WID_MENU_MAX_ITEMS][WID_MENU_MAX_COLS];
    double bar_width[WID_MENU_MAX_ITEMS][WID_MENU_MAX_COLS];

    /*
     * Shortcut key
     */
    char shortcut[WID_MENU_MAX_ITEMS];

    on_update_t on_update;

} wid_menu_ctx;

widp wid_menu(widp parent,
              fontp focus_font,
              fontp other_font,
              on_update_t on_update,
              double x,
              double y,
              int cols,
              int focus,
              int args, ...);

extern int wid_menu_visible;
