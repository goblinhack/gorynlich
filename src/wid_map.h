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

    /*
     * Used for level preview.
     */
    tpp tiles[MAP_WIDTH][MAP_HEIGHT][MAP_DEPTH];
} wid_map_level;

typedef void(*wid_map_event_t)(widp);
typedef void(*on_selected_t)(const level_pos_t);
typedef void(*on_cancelled_t)(void);

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

    /*
     * Current level being loaded.
     */
    int loading_x;
    int loading_y;

    on_selected_t on_selected;
    on_cancelled_t on_cancelled;

} wid_map_ctx;

widp wid_map(const char *title, on_selected_t, on_cancelled_t);

widp wid_editor_level_map_thing_replace_template(widp w,
                                                 double x,
                                                 double y,
                                                 thingp t,
                                                 tpp tp,
                                                 itemp item,
                                                 thing_statsp stats);
extern void wid_map_cell_play(void);
extern void wid_map_cell_load(void);
extern void wid_map_visible(void);
extern void wid_map_preview(widp);
extern void wid_map_preview_thumbnail(widp);
