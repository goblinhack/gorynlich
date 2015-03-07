/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#pragma once

typedef struct wid_map_cell_ {
    levelp level;
    char *name;
    int x;
    int y;
} wid_map_level;

typedef void(*wid_map_event_t)(widp);

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
     * Entered text.
     */
    char text[MAXSTR];

    /*
     * Item currently in focus
     */
    int focusx;
    int focusy;

    /*
     * When the map was made.
     */
    uint32_t created;

    /*
     * Items in the map
     */
    widp buttons[LEVELS_ACROSS][LEVELS_DOWN];
    wid_map_level levels[LEVELS_ACROSS][LEVELS_DOWN];

    /*
     * What to call on events
     */
    wid_map_event_t selected;
    wid_map_event_t cancelled;

    /*
     * Just created?
     */
    int is_new;

} wid_map_ctx;

widp wid_map (void);

extern int wid_map_visible;
