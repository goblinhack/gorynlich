/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#pragma once

#define WID_MENU_MAX_ITEMS 20

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
    widp buttons[WID_MENU_MAX_ITEMS];

    /*
     * What to call on events
     */
    on_mouse_down_t event_handler[WID_MENU_MAX_ITEMS];

    /*
     * Shortcut key
     */
    char shortcut[WID_MENU_MAX_ITEMS];

} wid_menu_ctx;

widp wid_menu(widp parent,
              fontp focus_font,
              fontp other_font,
              double padding,
              int focus,
              int args, ...);

extern int wid_menu_visible;
