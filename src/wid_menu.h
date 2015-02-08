/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#pragma once

#define WID_MENU_MAX_ITEMS 20

typedef struct {
    widp w;
    int focus;
    int items;
    fontp focus_font;
    fontp normal_font;
    double focus_height;
    double normal_height;
    widp buttons[WID_MENU_MAX_ITEMS];
    on_mouse_down_t event_handler[WID_MENU_MAX_ITEMS];
} wid_menu_ctx;

widp wid_menu(widp parent,
              fontp focus_font,
              fontp other_font,
              double padding,
              int focus,
              int args, ...);
