/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
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

#define WID_EDITOR_UNDO                 60

enum {
    WID_EDITOR_MODE_TOGGLE,
    WID_EDITOR_MODE_COLOR,
    WID_EDITOR_MODE_DRAW,
    WID_EDITOR_MODE_PAINT,
    WID_EDITOR_MODE_LINE,
    WID_EDITOR_MODE_SQUARE,
    WID_EDITOR_MODE_CIRCLE,
    WID_EDITOR_MODE_OCTAGON,
    WID_EDITOR_MODE_FILL,
    WID_EDITOR_MODE_DEL,
    WID_EDITOR_MODE_YANK,
    WID_EDITOR_MODE_EDIT,
    WID_EDITOR_MODE_CUT,
    WID_EDITOR_MODE_COPY,
    WID_EDITOR_MODE_PASTE,
    WID_EDITOR_MODE_UNDO,
    WID_EDITOR_MODE_REDO,
    WID_EDITOR_MODE_SAVE,
    WID_EDITOR_MODE_UNUSED_2,
    WID_EDITOR_MODE_UNUSED_3,
    WID_EDITOR_MODE_VFLIP,
    WID_EDITOR_MODE_HFLIP,
    WID_EDITOR_MODE_ROTATE,
    WID_EDITOR_MODE_TEST,
    WID_EDITOR_MODE_STYLE,
    WID_EDITOR_MODE_RANDOM,
    WID_EDITOR_MODE_MAX,
};

enum {
    WID_EDITOR_MODE2_TITLE,
    WID_EDITOR_MODE2_64x64,
    WID_EDITOR_MODE2_32x32,
    WID_EDITOR_MODE2_UNUSED_3,
    WID_EDITOR_MODE2_UNUSED_4,
    WID_EDITOR_MODE2_UNUSED_5,
    WID_EDITOR_MODE2_UNUSED_6,
    WID_EDITOR_MODE2_UNUSED_7,
    WID_EDITOR_MODE2_UNUSED_8,
    WID_EDITOR_MODE2_FILTER_PLAYER,
    WID_EDITOR_MODE2_FILTER_ACTIONS,
    WID_EDITOR_MODE2_FILTER_MONST_AND_OBJ,
    WID_EDITOR_MODE2_FILTER_WALLS,
    WID_EDITOR_MODE2_FILTER_FLOOR,
    WID_EDITOR_MODE2_FILTER_ALL,
    WID_EDITOR_MODE2_MAX,
};

enum {
    WID_TILE_MODE_NONE,
    WID_TILE_MODE_WALLS,
    WID_TILE_MODE_DOORS,
    WID_TILE_MODE_FLOORS,
    WID_TILE_MODE_MOB_SPAWNERS,
    WID_TILE_MODE_MONST,
    WID_TILE_MODE_WEAPONS,
    WID_TILE_MODE_FOOD,
    WID_TILE_MODE_TREASURE,
    WID_TILE_MODE_POTION,
    /*
     * Add to here, update WID_TILE_POOL_MAX
     */
    WID_TILE_MODE_KEYS,
    WID_TILE_MODE_TRAP,
    WID_TILE_MODE_ITEMS,
    WID_TILE_MODE_PLAYER,
    WID_TILE_MODE_EXITS,
    WID_TILE_MODE_ACTIONS,
    WID_TILE_MODE_LAST,
    /*
     * Add to here, update WID_TILE_POOL_MAX
     */
};
#define WID_TILE_POOL_MAX 16

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

    /*
     * Data associated with individual tiles.
     */
    thing_template_data data;
} wid_editor_map_tile;

typedef struct wid_editor_map_grid_ {
    wid_editor_map_tile tile[MAP_WIDTH][MAP_HEIGHT][MAP_DEPTH_MAX];

    /*
     * For joined up walls etc.
     */
    tilep map_tile[MAP_WIDTH][MAP_HEIGHT];
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
     * Current color
     */
    char *col_name;
    color col;

    /*
     * Entered text.
     */
    char text[MAXSTR];

    /*
     * Item currently in focus
     */
    int focus_x;
    int focus_y;

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

    /*
     * Tile pools
     */
    wid_editor_tile tile_pools[WID_TILE_POOL_MAX][THING_MAX];
    int tile_count[WID_TILE_POOL_MAX];

    /*
     * Map tiles
     */
    wid_editor_map_grid map;
    wid_editor_map_grid map_undo[WID_EDITOR_UNDO];
    wid_editor_map_grid map_copy;
    wid_editor_map_grid map_tmp;

    /*
     * When we change layer, this holds onto all the unmodified layers.
     */
    wid_editor_map_grid map_preserved_layers;

    uint8_t valid_undo[WID_EDITOR_UNDO];

    /*
     * For line drawing.
     */
    int map_highlight[MAP_WIDTH][MAP_HEIGHT];

    /*
     * Just created?
     */
    int is_new;

    /*
     * Modified and need to save?
     */
    int save_needed;

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
    int tile_pool;

    /*
     * Which slice are we editing.
     */
    int layer_mode;

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

    int square_start_x;
    int square_start_y;
    int got_square_start;

    int cut_start_x;
    int cut_start_y;
    int cut_end_x;
    int cut_end_y;
    int got_cut_start;

} wid_editor_ctx;

void wid_editor(level_pos_t);

widp wid_editor_replace_template(widp w,
                                 double x,
                                 double y,
                                 thingp t,
                                 tpp tp,
                                 tpp_data,
                                 itemp item,
                                 thing_statsp stats);

extern void wid_editor_preview(widp);
extern void wid_editor_preview_thumbnail(widp);
