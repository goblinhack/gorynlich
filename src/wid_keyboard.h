/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

typedef void(*wid_keyboard_event_t)(widp, const char *text);

#define WID_KEYBOARD_ACROSS 12
#define WID_KEYBOARD_DOWN   5

typedef struct {
    /*
     * Parent widget
     */
    widp w;

    /*
     * Current button
     */
    widp b;

    /*
     * Text input widget
     */
    widp input;

    /*
     * Entered text.
     */
    char text[MAXSTR];

    /*
     * Item currently in focus
     */
    int focusx;
    int focusy;

    /*
     * When the keyboard was made.
     */
    uint32_t created;

    /*
     * Items in the keyboard
     */
    widp buttons[WID_KEYBOARD_DOWN][WID_KEYBOARD_ACROSS];

    /*
     * What to call on events
     */
    wid_keyboard_event_t selected;
    wid_keyboard_event_t cancelled;

    /*
     * Just created?
     */
    int is_new;

} wid_keyboard_ctx;

widp wid_keyboard (const char *text,
                   const char *title,
                   wid_keyboard_event_t selected,
                   wid_keyboard_event_t cancelled);

extern int wid_keyboard_visible;
