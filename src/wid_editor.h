/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#pragma once

#define WID_EDITOR_MENU_CELLS_ACROSS    26
#define WID_EDITOR_MENU_CELLS_DOWN      16

#define WID_EDITOR_MENU_MAP_ACROSS      25
#define WID_EDITOR_MENU_MAP_DOWN        15

#define WID_EDITOR_MENU_TILES_ACROSS    20
#define WID_EDITOR_MENU_TILES_DOWN      14

typedef struct wid_editor_tile_ {
    int x;
    int y;

    /*
     * Used for level preview.
     */
    tpp tile_tp;

    widp button;
} wid_editor_tile;

typedef struct wid_editor_map_tile_ {
    tpp tp;
} wid_editor_map_tile;

typedef void(*wid_editor_event_t)(widp);

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
     * When we last changed from tile to map mode.
     */
    uint32_t mode_toggled;

    /*
     * Items in the map
     */
    wid_editor_tile 
        tile[WID_EDITOR_MENU_CELLS_ACROSS][WID_EDITOR_MENU_CELLS_DOWN];
    wid_editor_map_tile 
        map_tile[MAP_WIDTH][MAP_HEIGHT][MAP_DEPTH];

    /*
     * Just created?
     */
    int is_new;

    /*
     * Current level.
     */
    levelp level;

    /*
     * Current level being loaded.
     */
    int loading_x;
    int loading_y;

    int map_x;
    int map_y;

    int tile_mode;

    /*
     * Drawing or lines or?
     */
    int edit_mode;

    /*
     * Line drawing state.
     */
    int line_start_x;
    int line_start_y;
    int got_line_start;
} wid_editor_ctx;

enum {
    WID_EDITOR_MODE_DRAW,
    WID_EDITOR_MODE_LINE,
    WID_EDITOR_MODE_FILL,
    WID_EDITOR_MODE_DEL,
    WID_EDITOR_MODE_MAX,
};

void wid_editor(level_pos_t);

widp wid_editor_replace_template(widp w,
                                 double x,
                                 double y,
                                 thingp t,
                                 tpp tp,
                                 itemp item,
                                 thing_statsp stats);
extern void wid_editor_preview(widp);
extern void wid_editor_preview_thumbnail(widp);
