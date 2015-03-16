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

#define WID_EDITOR_UNDO                 1000

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

typedef struct wid_editor_map_grid_ {
    wid_editor_map_tile tile[MAP_WIDTH][MAP_HEIGHT][MAP_DEPTH];
} wid_editor_map_grid;

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

    wid_editor_map_grid map;
    wid_editor_map_grid map_undo[WID_EDITOR_UNDO];
    uint8_t valid_undo[WID_EDITOR_UNDO];

    /*
     * For line drawing.
     */
    int map_highlight[MAP_WIDTH][MAP_HEIGHT];

    /*
     * For joined up walls etc.
     */
    tilep map_tile[MAP_WIDTH][MAP_HEIGHT];

    /*
     * Just created?
     */
    int is_new;

    /*
     * Current level.
     */
    levelp level;
    level_pos_t level_pos;

    /*
     * Current level being loaded.
     */
    int loading_x;
    int loading_y;

    int map_x;
    int map_y;

    /*
     * Where in the undo array are we
     */
    int undo_at;

    int tile_mode;

    /*
     * Drawing or lines or?
     */
    int edit_mode;
    int old_edit_mode;

    /*
     * Line drawing state.
     */
    int line_start_x;
    int line_start_y;
    int got_line_start;
} wid_editor_ctx;

enum {
    WID_EDITOR_MODE_TOGGLE,
    WID_EDITOR_MODE_DRAW,
    WID_EDITOR_MODE_LINE,
    WID_EDITOR_MODE_FILL,
    WID_EDITOR_MODE_DEL,
    WID_EDITOR_MODE_UNDO,
    WID_EDITOR_MODE_REDO,
    WID_EDITOR_MODE_SAVE,
    WID_EDITOR_MODE_UNUSED_1,
    WID_EDITOR_MODE_UNUSED_2,
    WID_EDITOR_MODE_UNUSED_3,
    WID_EDITOR_MODE_UNUSED_4,
    WID_EDITOR_MODE_UNUSED_5,
    WID_EDITOR_MODE_UNUSED_6,
    WID_EDITOR_MODE_UNUSED_7,
    WID_EDITOR_MODE_UNUSED_8,
    WID_EDITOR_MODE_UNUSED_9,
    WID_EDITOR_MODE_UNUSED_10,
    WID_EDITOR_MODE_UNUSED_11,
    WID_EDITOR_MODE_UNUSED_12,
    WID_EDITOR_MODE_UNUSED_13,
    WID_EDITOR_MODE_UNUSED_14,
    WID_EDITOR_MODE_UNUSED_15,
    WID_EDITOR_MODE_UNUSED_16,
    WID_EDITOR_MODE_UNUSED_17,
    WID_EDITOR_MODE_NUKE,
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
