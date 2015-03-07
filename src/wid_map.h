/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#pragma once

#define WID_MAP_ACROSS      20
#define WID_MAP_DOWN        20
#define WID_MAP_CELL_EXITS  4

typedef struct wid_map_cell_ {
    struct wid_map_cell_ *exit[WID_MAP_CELL_EXITS];
    char *name;
    int x;
    int y;
} wid_map_cell;

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
    widp buttons[WID_MAP_DOWN][WID_MAP_ACROSS];
    wid_map_cell cell[WID_MAP_DOWN][WID_MAP_ACROSS];

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
