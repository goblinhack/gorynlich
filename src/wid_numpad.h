/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#pragma once

typedef void(*wid_numpad_event_t)(widp, const char *text);

#define WID_NUMPAD_ACROSS 4
#define WID_NUMPAD_DOWN   4

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
     * When the numpad was made.
     */
    uint32_t created;

    /*
     * Items in the numpad
     */
    widp buttons[WID_NUMPAD_DOWN][WID_NUMPAD_ACROSS];

    /*
     * What to call on events
     */
    wid_numpad_event_t selected;
    wid_numpad_event_t cancelled;

    /*
     * Just created?
     */
    int is_new;

} wid_numpad_ctx;

widp wid_numpad (const char *text,
                   const char *title,
                   wid_numpad_event_t selected,
                   wid_numpad_event_t cancelled);

extern int wid_numpad_visible;
