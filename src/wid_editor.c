/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license.
 */

#include <SDL.h>
#include <errno.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "string_ext.h"
#include "ttf.h"
#include "thing_template.h"
#include "wid_editor.h"
#include "time_util.h"
#include "timer.h"
#include "level.h"
#include "math_util.h"
#include "thing_template.h"
#include "thing_tile.h"
#include "wid_tooltip.h"
#include "wid_intro.h"
#include "map_jigsaw.h"
#include "tile.h"
#include "marshal.h"
#include "wid_map.h"
#include "wid_cmap.h"
#include "wid_menu.h"
#include "wid_keyboard.h"
#include "string_util.h"
#include "wid_console.h"
#include "bits.h"
#include "thing.h"

static void wid_editor_hide(void);
static void wid_editor_tile_right_button_pressed(int x, int y);
static void wid_editor_set_focus(wid_editor_ctx *ctx, int focus_x, int focus_y);
static void wid_editor_map_scroll(int dx, int dy);
static void wid_editor_undo_save(void);
static void wid_editor_save_level(void);
static void wid_editor_save(const char *dir_and_file, int is_test_level);
static void wid_editor_button_animate(widp b, tpp tp);
static void wid_editor_tile_fill(int x, int y);
static void map_editor_fixup(void);
static void wid_editor_center(void);
static void wid_editor_96x96(void);
static void wid_editor_64x64(void);

static widp wid_editor_save_popup; // edit wid_editor_tick if you add more
static widp wid_editor_map_dialog;
static widp wid_choose_color_dialog;
static widp wid_choose_title_dialog;
static widp wid_choose_text_dialog;
static widp wid_editor_window;
static widp wid_editor_background;
static wid_editor_ctx *wid_editor_window_ctx;

static level_pos_t saved_level_pos = {-1,-1};
static int saved_focus_x = -1;
static int saved_focus_y = -1;
static int saved_map_x = -1;
static int saved_map_y = -1;
static tpp wid_editor_chosen_tile[WID_TILE_POOL_MAX];

static void wid_editor_set_mode (int edit_mode)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    ctx->edit_mode = edit_mode;

    memset(ctx->map_highlight, 0, sizeof(ctx->map_highlight));

    ctx->got_line_start = 0;
    ctx->got_square_start = 0;
    ctx->got_cut_start = 0;
}

static int layer_to_depth (int layer)
{
    switch (layer) {
    case WID_EDITOR_MODE2_FILTER_OBJ:
        return (MAP_DEPTH_OBJ);

    case WID_EDITOR_MODE2_FILTER_WALL:
        return (MAP_DEPTH_WALL);

    case WID_EDITOR_MODE2_FILTER_FLOOR:
        return (MAP_DEPTH_FLOOR);

    case WID_EDITOR_MODE2_FILTER_ACTIONS:
        return (MAP_DEPTH_ACTIONS);

    default:
        ERR("bad layer %d", layer);
    }

    return (MAP_DEPTH_ACTIONS);
}

/*
 * Force a rewrite of all layers that are not being editted so they in effect 
 * cannot be overwritten.
 */
static void wid_editor_overwrite_inactive_layers (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    if (ctx->layer_mode == WID_EDITOR_MODE2_FILTER_ALL) {
        return;
    }

    int x, y, z;
    int z_preserve = layer_to_depth(ctx->layer_mode);

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            for (z = 0; z < MAP_DEPTH_MAX; z++) {
                if (z == z_preserve) {
                    continue;
                }

                memcpy(&ctx->map.tile[x][y][z], 
                       &ctx->map_preserved_layers.tile[x][y][z], 
                       sizeof(wid_editor_map_tile));
            }
        }
    }

    map_editor_fixup();
}

static void wid_editor_set_layer_mode (int layer_mode)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    if (ctx->layer_mode == WID_EDITOR_MODE2_FILTER_ALL) {
        /*
         * Save all layers.
         */
        memcpy(&ctx->map_preserved_layers, &ctx->map, sizeof(ctx->map));
    } else {
        /*
         * Move the layers back before changing mode.
         */
        wid_editor_overwrite_inactive_layers();

        /*
         * Now save the merged layers.
         */
        memcpy(&ctx->map_preserved_layers, &ctx->map, sizeof(ctx->map));
    }

    ctx->layer_mode = layer_mode;
}

static void wid_editor_set_new_tp (int x, int y, int z, 
                                   tpp tp,
                                   tpp_data data)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    memset(&ctx->map.tile[x][y][z], 0, sizeof(wid_editor_map_tile));
    ctx->map.tile[x][y][z].tp = tp;

    thing_template_data d;

    if (data) {
        memcpy(&d, data, sizeof(d));
    } else {
        memset(&d, 0, sizeof(d));

        d.col_name = (char*) ctx->col_name;
        d.col = ctx->col;
    }

    ctx->map.tile[x][y][z].data = d;
}

/*
 * Replace or place a tile.
 */
widp wid_editor_replace_template (widp w,
                                  double x,
                                  double y,
                                  thingp t,
                                  tpp tp,
                                  tpp_data data,
                                  itemp item,
                                  thing_statsp stats)
{
    /*
     * Can't use w as it has the level set as its context now.
     */
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    int ix = (int)x;
    int iy = (int)y;
    int z = tp_get_z_depth(tp);

    if ((ix >= MAP_WIDTH) || (iy >= MAP_HEIGHT) || 
        (ix < 0) || (iy < 0) ||
        (z > MAP_DEPTH_MAX)) {
        ERR("overflow in reading position "
            "(%f,%f,%d) -> (%d,%d) in level %u.%u, "
            "map bounds (%d,%d) -> (%d,%d)", 
            x, y, z, ix, iy, ctx->loading_x, ctx->loading_y,
            0, 0, MAP_DEPTH_MAX, MAP_HEIGHT);
        return (0);
    }

    if (tp_is_player(tp)) {
        ctx->map_x = ix;
        ctx->map_y = iy;
        ctx->map_x -= WID_EDITOR_MENU_MAP_ACROSS / 2;
        ctx->map_y -= WID_EDITOR_MENU_MAP_DOWN / 2;

        while (ctx->map_x < 0) {
            ctx->map_x++;
        }

        while (ctx->map_y < 0) {
            ctx->map_y++;
        }

        while (ctx->map_x + WID_EDITOR_MENU_MAP_ACROSS >= MAP_WIDTH - 1) {
            ctx->map_x--;
        }

        while (ctx->map_y + WID_EDITOR_MENU_MAP_DOWN >= MAP_HEIGHT - 1) {
            ctx->map_y--;
        }
    }

    wid_editor_set_new_tp(ix, iy, z, tp, data);

    return (0);
}

static tpp map_editor_find_wall_at (int x, int y)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    if ((x < 0) || (y < 0) || (x >= MAP_WIDTH) || (y >= MAP_HEIGHT)) {
        return (0);
    }

    tpp tp = ctx->map.tile[x][y][MAP_DEPTH_WALL].tp;
    if (tp && tp_is_wall(tp)) {
        return (tp);
    }

    return (0);
}

static tpp map_editor_find_door_at (int x, int y)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    if ((x < 0) || (y < 0) || (x >= MAP_WIDTH) || (y >= MAP_HEIGHT)) {
        return (0);
    }

    tpp tp = ctx->map.tile[x][y][MAP_DEPTH_WALL].tp;
    if (tp && tp_is_door(tp)) {
        return (tp);
    }

    return (0);
}

static void map_editor_fixup (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    int32_t index;
    tilep tile;
    int32_t x;
    int32_t y;
    int32_t dx;
    int32_t dy;
    tpp nbrs[3][3];

    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {

            ctx->map.map_tile[x][y] = 0;

            int ok = false;

            if (map_editor_find_wall_at(x, y)) {
                ok = true;
            } else if (map_editor_find_door_at(x, y)) {
                ok = true;
            }

            if (!ok) {
                continue;
            }

            memset(nbrs, 0, sizeof(nbrs));

            for (dx = -1; dx <= 1; dx++) {
                for (dy = -1; dy <= 1; dy++) {

                    if (map_editor_find_wall_at(x, y)) {
                        nbrs[dx + 1][dy + 1] = map_editor_find_wall_at(x + dx, y + dy);
                    }

                    if (map_editor_find_door_at(x, y)) {
                        nbrs[dx + 1][dy + 1] = map_editor_find_door_at(x + dx, y + dy);
                    }
                }
            }

            tpp a = nbrs[0][0];
            tpp b = nbrs[1][0];
            tpp c = nbrs[2][0];
            tpp d = nbrs[0][1];
            tpp e = nbrs[1][1];
            tpp f = nbrs[2][1];
            tpp g = nbrs[0][2];
            tpp h = nbrs[1][2];
            tpp i = nbrs[2][2];

            uint8_t A = (a != 0) ? 1 : 0;
            uint8_t B = (b != 0) ? 1 : 0;
            uint8_t C = (c != 0) ? 1 : 0;
            uint8_t D = (d != 0) ? 1 : 0;
            uint8_t E = (e != 0) ? 1 : 0;
            uint8_t F = (f != 0) ? 1 : 0;
            uint8_t G = (g != 0) ? 1 : 0;
            uint8_t H = (h != 0) ? 1 : 0;
            uint8_t I = (i != 0) ? 1 : 0;

            const uint16_t omask =
                (I << 8) | (H << 7) | (G << 6) | (F << 5) |
                (E << 4) | (D << 3) | (C << 2) | (B << 1) |
                (A << 0);

            uint8_t score;
            uint8_t best = 0;

            index = -1;

            uint16_t mask;

#define BLOCK(a, b, c, d, e, f, g, h, i, _index_)                               \
            mask =                                                      \
                (i << 8) | (h << 7) | (g << 6) | (f << 5) |             \
                (e << 4) | (d << 3) | (c << 2) | (b << 1) |             \
                (a << 0);                                               \
                                                                        \
            if ((mask & omask) == mask) {                               \
                uint32_t difference = mask ^ omask;                     \
                BITCOUNT(difference);                                   \
                score = 32 - difference;                                \
                if (score > best) {                                     \
                    best = score;                                       \
                    index = _index_;                                    \
                }                                                       \
            }                                                           \

            BLOCK(1,1,1,1,1,1,1,1,1, IS_JOIN_BLOCK)
            BLOCK(0,0,0,0,1,0,0,0,0, IS_JOIN_NODE)
            BLOCK(0,0,0,0,1,1,0,0,0, IS_JOIN_LEFT)
            BLOCK(0,0,0,0,1,0,0,1,0, IS_JOIN_TOP)
            BLOCK(0,0,0,1,1,0,0,0,0, IS_JOIN_RIGHT)
            BLOCK(0,1,0,0,1,0,0,0,0, IS_JOIN_BOT)
            BLOCK(0,0,0,1,1,1,0,0,0, IS_JOIN_HORIZ)
            BLOCK(0,1,0,0,1,0,0,1,0, IS_JOIN_VERT)
            BLOCK(0,0,0,0,1,1,0,1,1, IS_JOIN_TL2)
            BLOCK(0,1,1,0,1,1,0,0,0, IS_JOIN_BL2)
            BLOCK(1,1,0,1,1,0,0,0,0, IS_JOIN_BR2)
            BLOCK(0,0,0,1,1,0,1,1,0, IS_JOIN_TR2)
            BLOCK(0,0,0,0,1,1,0,1,0, IS_JOIN_TL)
            BLOCK(0,1,0,0,1,1,0,0,0, IS_JOIN_BL)
            BLOCK(0,1,0,1,1,0,0,0,0, IS_JOIN_BR)
            BLOCK(0,0,0,1,1,0,0,1,0, IS_JOIN_TR)
            BLOCK(1,1,0,1,1,0,1,1,0, IS_JOIN_T90_3)
            BLOCK(1,1,1,1,1,1,0,0,0, IS_JOIN_T180_3)
            BLOCK(0,1,1,0,1,1,0,1,1, IS_JOIN_T270_3)
            BLOCK(0,0,0,1,1,1,1,1,1, IS_JOIN_T_3)
            BLOCK(0,1,0,0,1,1,0,1,0, IS_JOIN_T270)
            BLOCK(0,1,0,1,1,1,0,0,0, IS_JOIN_T180)
            BLOCK(0,1,0,1,1,0,0,1,0, IS_JOIN_T90)
            BLOCK(0,0,0,1,1,1,0,1,0, IS_JOIN_T)
            BLOCK(0,1,1,0,1,1,0,1,0, IS_JOIN_T270_2)
            BLOCK(1,1,0,1,1,1,0,0,0, IS_JOIN_T180_2)
            BLOCK(0,1,0,1,1,0,1,1,0, IS_JOIN_T90_2)
            BLOCK(0,0,0,1,1,1,0,1,1, IS_JOIN_T_2)
            BLOCK(0,1,0,0,1,1,0,1,1, IS_JOIN_T270_1)
            BLOCK(0,1,1,1,1,1,0,0,0, IS_JOIN_T180_1)
            BLOCK(1,1,0,1,1,0,0,1,0, IS_JOIN_T90_1)
            BLOCK(0,0,0,1,1,1,1,1,0, IS_JOIN_T_1)
            BLOCK(0,1,0,1,1,1,0,1,0, IS_JOIN_X)
            BLOCK(0,1,0,1,1,1,0,1,1, IS_JOIN_X1)
            BLOCK(0,1,1,1,1,1,0,1,0, IS_JOIN_X1_270)
            BLOCK(1,1,0,1,1,1,0,1,0, IS_JOIN_X1_180)
            BLOCK(0,1,0,1,1,1,1,1,0, IS_JOIN_X1_90)
            BLOCK(0,1,0,1,1,1,1,1,1, IS_JOIN_X2)
            BLOCK(0,1,1,1,1,1,0,1,1, IS_JOIN_X2_270)
            BLOCK(1,1,1,1,1,1,0,1,0, IS_JOIN_X2_180)
            BLOCK(1,1,0,1,1,1,1,1,0, IS_JOIN_X2_90)
            BLOCK(0,1,1,1,1,1,1,1,0, IS_JOIN_X3)
            BLOCK(1,1,0,1,1,1,0,1,1, IS_JOIN_X3_180)
            BLOCK(0,1,1,1,1,1,1,1,1, IS_JOIN_X4)
            BLOCK(1,1,1,1,1,1,0,1,1, IS_JOIN_X4_270)
            BLOCK(1,1,1,1,1,1,1,1,0, IS_JOIN_X4_180)
            BLOCK(1,1,0,1,1,1,1,1,1, IS_JOIN_X4_90)

            /*
             * Single node doors need to join onto walls.
             */
            if (index == IS_JOIN_NODE) {
                if (map_editor_find_door_at(x, y)) {
                    if ( map_editor_find_wall_at(x - 1, y) &&
                         map_editor_find_wall_at(x + 1, y) &&
                        !map_editor_find_wall_at(x, y - 1) &&
                        !map_editor_find_wall_at(x, y + 1)) {
                        index = IS_JOIN_HORIZ2;
                    }
                }
            }

            if (index == IS_JOIN_NODE) {
                if (map_editor_find_door_at(x, y)) {
                    if (!map_editor_find_wall_at(x - 1, y) &&
                        !map_editor_find_wall_at(x + 1, y) &&
                         map_editor_find_wall_at(x, y - 1) &&
                         map_editor_find_wall_at(x, y + 1)) {
                        index = IS_JOIN_VERT2;
                    }
                }
            }

            if (index == -1) {
                ERR("%u%u%u %u%u%u %u%u%u not handled",
                    a ? 1 : 0,
                    b ? 1 : 0,
                    c ? 1 : 0,
                    d ? 1 : 0,
                    e ? 1 : 0,
                    f ? 1 : 0,
                    g ? 1 : 0,
                    h ? 1 : 0,
                    i ? 1 : 0);
            }

            tpp t = e;

            thing_tilep thing_tile = thing_tile_find(t, index, &tile);
            if (!thing_tile) {
                index = IS_JOIN_BLOCK;

                thing_tile = thing_tile_find(t, index, &tile);
                if (!thing_tile) {
                    index = IS_JOIN_NODE;

                    thing_tile = thing_tile_find(t, index, &tile);
                    if (!thing_tile) {
                        ERR("no joinable tile for %s", tp_name(t));
                    }
                }
            }

            if (!tile) {
                ERR("no tile for %s", tp_name(t));
            }

            const char *tilename = thing_tile_name(thing_tile);

            if (!tilename) {
                ERR("no tilename for %s", tp_name(e));
            }

            tilep tile = tile_find(tilename);
            if (!tile) {
                ERR("cannot find tilep for tile %s", tilename);
            }

            ctx->map.map_tile[x][y] = tile;
        }
    }
}

/*
 * Set the layer focus.
 */
static void wid_editor_update_layer_mode_buttons (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    /*
     * Reset all buttons.
     */
    int i;
    for (i = 0; i < WID_EDITOR_MODE2_MAX; i++) {
        widp b = ctx->tile[WID_EDITOR_MENU_CELLS_ACROSS - 1][i].button;
        if (!b) {
            continue;
        }

        color c = CYAN;
        c.a = 50;
        wid_set_color(b, WID_COLOR_BG, c);

        c = WHITE;
        c.a = 100;
        wid_set_color(b, WID_COLOR_TL, c);

        c = GRAY;
        c.a = 100;
        wid_set_color(b, WID_COLOR_BR, c);

        wid_set_color(b, WID_COLOR_TEXT, GRAY);

        switch (i) {
        case WID_EDITOR_MODE2_96x96:
        case WID_EDITOR_MODE2_64x64:
        case WID_EDITOR_MODE2_32x32:
            c.a = 255;
            wid_set_color(b, WID_COLOR_TL, RED);
            wid_set_color(b, WID_COLOR_BR, RED);
            wid_set_color(b, WID_COLOR_BG, BLACK);
            break;
        case WID_EDITOR_MODE2_CENTER:
        case WID_EDITOR_MODE2_FILTER_OBJ:
        case WID_EDITOR_MODE2_FILTER_WALL:
        case WID_EDITOR_MODE2_FILTER_ACTIONS:
        case WID_EDITOR_MODE2_FILTER_ALL:
            c = GREEN;
            c.a = 50;
            wid_set_color(b, WID_COLOR_BG, c);
            c.a = 100;
            wid_set_color(b, WID_COLOR_TL, c);
            wid_set_color(b, WID_COLOR_BR, c);
            break;
        }
    }

    /*
     * Set the current focus.
     */
    widp b = ctx->tile[WID_EDITOR_MENU_CELLS_ACROSS - 1][ctx->layer_mode].button;
    if (!b) {
        ERR("no layer button at %d", ctx->layer_mode);
    }

    color c = RED;
    c.a = 100;
    wid_set_color(b, WID_COLOR_BG, c);
    c.a = 255;
    wid_set_color(b, WID_COLOR_TL, RED);
    wid_set_color(b, WID_COLOR_BR, RED);
    wid_set_color(b, WID_COLOR_TEXT, GREEN);
}

/*
 * Set the edit mode focus.
 */
static void wid_editor_update_edit_mode_buttons (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    /*
     * Reset all buttons.
     */
    int i;
    for (i = 0; i < WID_EDITOR_MODE_MAX; i++) {
        widp b = ctx->tile[i][WID_EDITOR_MENU_CELLS_DOWN - 1].button;
        if (!b) {
            continue;
        }

        color c = CYAN;
        c.a = 50;
        wid_set_color(b, WID_COLOR_BG, c);

        c = WHITE;
        c.a = 100;
        wid_set_color(b, WID_COLOR_TL, c);

        c = GRAY;
        c.a = 100;
        wid_set_color(b, WID_COLOR_BR, c);

        wid_set_color(b, WID_COLOR_TEXT, GRAY);
    }

    int x;
    for (x = 0; x < WID_EDITOR_MODE_MAX; x++) {
        widp b = ctx->tile[x][WID_EDITOR_MENU_CELLS_DOWN - 1].button;
        if (!b) {
            continue;
        }

        color c = BLACK;
        c.a = 100;
        wid_set_color(b, WID_COLOR_BG, c);

        switch (x) {
        case WID_EDITOR_MODE_DRAW:
        case WID_EDITOR_MODE_PAINT:
        case WID_EDITOR_MODE_LINE:
        case WID_EDITOR_MODE_FILL:
        case WID_EDITOR_MODE_DEL:
        case WID_EDITOR_MODE_UNDO:
        case WID_EDITOR_MODE_REDO:
            break;

        case WID_EDITOR_MODE_TOGGLE:
            if (wid_editor_chosen_tile[ctx->tile_pool]) {
                tpp tp = wid_editor_chosen_tile[ctx->tile_pool];

                wid_set_thing_template(b, tp);
                wid_editor_button_animate(b, tp);

                wid_set_color(b, WID_COLOR_TL, RED);
                wid_set_color(b, WID_COLOR_BR, RED);
            }
            break;

        case WID_EDITOR_MODE_COLOR:
            wid_set_color(b, WID_COLOR_BG, ctx->col);
            break;
        }
    }

    /*
     * Set the current focus.
     */
    widp b = ctx->tile[ctx->edit_mode][WID_EDITOR_MENU_CELLS_DOWN - 1].button;
    color c = RED;
    c.a = 100;
    wid_set_color(b, WID_COLOR_BG, c);
    c.a = 255;
    wid_set_color(b, WID_COLOR_TL, RED);
    wid_set_color(b, WID_COLOR_BR, RED);
    wid_set_color(b, WID_COLOR_TEXT, GREEN);
}

static tilep wid_editor_tp_to_tile (tpp tp)
{
    if (!tp) {
        return (0);
    }

    int tick = time_get_time_ms() / 100;
    int which = tick % 8;

    const char *tn = tp_raw_name(tp);

    char tilename[40];

    switch (which) {
        case 0: snprintf(tilename, sizeof(tilename) - 1, "%s-right", tn); break;
        case 1: snprintf(tilename, sizeof(tilename) - 1, "%s-br", tn); break;
        case 2: snprintf(tilename, sizeof(tilename) - 1, "%s-down", tn); break;
        case 3: snprintf(tilename, sizeof(tilename) - 1, "%s-bl", tn); break;
        case 4: snprintf(tilename, sizeof(tilename) - 1, "%s-left", tn); break;
        case 5: snprintf(tilename, sizeof(tilename) - 1, "%s-tl", tn); break;
        case 6: snprintf(tilename, sizeof(tilename) - 1, "%s-up", tn); break;
        case 7: snprintf(tilename, sizeof(tilename) - 1, "%s-tr", tn); break;
    }

    tilep tile = tile_find(tilename);
    if (tile) {
        return (tile);
    }

    switch (which) {
        case 0: snprintf(tilename, sizeof(tilename) - 1, "%s1a-right", tn); break;
        case 1: snprintf(tilename, sizeof(tilename) - 1, "%s1a-br", tn); break;
        case 2: snprintf(tilename, sizeof(tilename) - 1, "%s1a-down", tn); break;
        case 3: snprintf(tilename, sizeof(tilename) - 1, "%s1a-bl", tn); break;
        case 4: snprintf(tilename, sizeof(tilename) - 1, "%s1a-left", tn); break;
        case 5: snprintf(tilename, sizeof(tilename) - 1, "%s1a-tl", tn); break;
        case 6: snprintf(tilename, sizeof(tilename) - 1, "%s1a-up", tn); break;
        case 7: snprintf(tilename, sizeof(tilename) - 1, "%s1a-tr", tn); break;
    }

    tile = tile_find(tilename);
    if (tile) {
        return (tile);
    }

    switch (which) {
        case 0: snprintf(tilename, sizeof(tilename) - 1, "%sa-right", tn); break;
        case 1: snprintf(tilename, sizeof(tilename) - 1, "%sa-br", tn); break;
        case 2: snprintf(tilename, sizeof(tilename) - 1, "%sa-down", tn); break;
        case 3: snprintf(tilename, sizeof(tilename) - 1, "%sa-bl", tn); break;
        case 4: snprintf(tilename, sizeof(tilename) - 1, "%sa-left", tn); break;
        case 5: snprintf(tilename, sizeof(tilename) - 1, "%sa-tl", tn); break;
        case 6: snprintf(tilename, sizeof(tilename) - 1, "%sa-up", tn); break;
        case 7: snprintf(tilename, sizeof(tilename) - 1, "%sa-tr", tn); break;
    }

    tile = tile_find(tilename);
    if (tile) {
        return (tile);
    }

    /*
     * Just find the first tile.
     */
    thing_tilep thing_tile;
    tree_rootp tiles;

    tiles = tp_get_tiles(tp);
    if (!tiles) {
        return (0);
    }

    thing_tile = thing_tile_first(tiles);
    if (!thing_tile) {
        return (0);
    }

    {
        const char *tilename = thing_tile_name(thing_tile);
        if (!tilename) {
            ERR("cannot find tile %s", tilename);
            return (0);
        }

        tilep tile = tile_find(tilename);

        return (tile);
    }
}

static void wid_editor_button_animate (widp b, tpp tp)
{
    if (!tp) {
        return;
    }

    wid_set_thing_template(b, tp);

    tilep tile = wid_editor_tp_to_tile(tp);
    if (tile) {
        wid_set_tile(b, tile);
    }

    wid_set_animate(b, false);
}

/*
 * Set the edit mode focus.
 */
static void wid_editor_update_tile_mode_buttons (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    if (!ctx->tile_mode) {
        int i;

        for (i = 0; i < WID_TILE_POOL_MAX; i++) {
            widp b = ctx->tile[i][0].button;
            if (!b) {
                continue;
            }
            wid_set_text(b, 0);
        }

        return;
    }

    {
        int x, y;

        for (x = 0; x < WID_EDITOR_MENU_CELLS_ACROSS; x++) {
            for (y = 0; y < WID_EDITOR_MENU_CELLS_DOWN; y++) {
                widp b = ctx->tile[x][y].button;
                if (!b) {
                    continue;
                }

                wid_set_thing_template(b, 0);
            }
        }
    }

    int x = 0;
    int y = 1;
    int i;

    for (i = 0; i < THING_MAX_ID; i++) {
        tpp tp = ctx->tile_pools[ctx->tile_pool][i].tile_tp;
        if (!tp) {
            continue;
        }

        x++;
        if (x >= WID_EDITOR_MENU_TILES_ACROSS) {
            x = 0;
            y++;
        }

        widp b = ctx->tile[x][y].button;
        if (!b) {
            continue;
        }

        wid_set_color(b, WID_COLOR_BG, BLACK);

        if ((x == ctx->focus_x) && (y == ctx->focus_y)) {
            wid_set_color(b, WID_COLOR_TL, RED);
            wid_set_color(b, WID_COLOR_BR, RED);
        }

        if (tp == wid_editor_chosen_tile[ctx->tile_pool]) {
            color c = RED;
            c.a = 100;
            wid_set_color(b, WID_COLOR_BG, c);
            wid_set_color(b, WID_COLOR_TL, RED);
            wid_set_color(b, WID_COLOR_BR, RED);
        }

        wid_editor_button_animate(b, tp);

        wid_set_tooltip(b, tp_get_tooltip(tp), vsmall_font);
    }

    for (i = 0; i < WID_TILE_POOL_MAX; i++) {

        widp b = ctx->tile[i][0].button;
        if (!b) {
            continue;
        }

        switch (i) {
        case WID_TILE_MODE_WALLS:
            wid_set_text(b, "Walls");
            break;
        case WID_TILE_MODE_FLOORS:
            wid_set_text(b, "Floor");
            break;
        case WID_TILE_MODE_DOORS:
            wid_set_text(b, "Doors");
            break;
        case WID_TILE_MODE_MONST:
            wid_set_text(b, "Monst");
            break;
        case WID_TILE_MODE_EXITS:
            wid_set_text(b, "Exits");
            break;
        case WID_TILE_MODE_WEAPONS:
            wid_set_text(b, "Weapons");
            break;
        case WID_TILE_MODE_FOOD:
            wid_set_text(b, "Food");
            break;
        case WID_TILE_MODE_ACTIONS:
            wid_set_text(b, "Actions");
            break;
        case WID_TILE_MODE_WORLD:
            wid_set_text(b, "World");
            break;
        case WID_TILE_MODE_MOB_SPAWNERS:
            wid_set_text(b, "Mobs");
            break;
        case WID_TILE_MODE_TREASURE:
            wid_set_text(b, "Loot");
            break;
        case WID_TILE_MODE_POTION:
            wid_set_text(b, "Potions");
            break;
        case WID_TILE_MODE_KEYS:
            wid_set_text(b, "Keys");
            break;
        case WID_TILE_MODE_TRAP:
            wid_set_text(b, "Traps");
            break;
        case WID_TILE_MODE_ITEMS:
            wid_set_text(b, "Items");
            break;
        case WID_TILE_MODE_PLAYER:
            wid_set_text(b, "Heroes");
            break;
        }

        if (i == ctx->tile_pool) {
            wid_set_color(b, WID_COLOR_TL, GRAY70);
            wid_set_color(b, WID_COLOR_BR, GRAY40);
            wid_set_color(b, WID_COLOR_TEXT, WHITE);
            wid_set_color(b, WID_COLOR_TEXT, RED);
        } else if (i) {
            wid_set_color(b, WID_COLOR_TL, GRAY70);
            wid_set_color(b, WID_COLOR_BR, GRAY40);
            wid_set_color(b, WID_COLOR_BG, GRAY50);
            wid_set_color(b, WID_COLOR_BG, GRAY10);
        }

        wid_set_font(b, vsmall_font);
    }
}

static void wid_editor_update_buttons (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    if (!ctx) {
        return;
    }

    double width = 1.0 / (double)(WID_EDITOR_MENU_CELLS_ACROSS);
    double height = 1.0 / (double)(WID_EDITOR_MENU_CELLS_DOWN);

    int x, y;

    ctx->b = 0;

    for (x = 0; x < WID_EDITOR_MENU_CELLS_ACROSS; x++) {
    for (y = 0; y < WID_EDITOR_MENU_CELLS_DOWN; y++) {

        widp b = ctx->tile[x][y].button;
        if (!b) {
            continue;
        }

        verify(b);

        fpoint tl;
        fpoint br;
        fontp font;
        color c;

        tl.x = ((double) x) * width;
        tl.y = ((double) y) * height;
        br.x = ((double) (x+1)) * width;
        br.y = ((double) (y+1)) * height;

        font = vsmall_font;

        int is_a_map_tile = 0;

        int mx = x + ctx->map_x;
        int my = y + ctx->map_y;

        if (!ctx->tile_mode) {
            int z;

            if ((x < WID_EDITOR_MENU_MAP_ACROSS) && 
                (y < WID_EDITOR_MENU_MAP_DOWN)) {
                for (z = 0; z < MAP_DEPTH_MAX; z++) {
                    if (ctx->map.tile[mx][my][z].tp) {
                        is_a_map_tile = 1;
                        break;
                    }
                }
            }
        }

        double zoom = 0.002;
        if ((x == ctx->focus_x) && (y == ctx->focus_y)) {
            if (!is_a_map_tile) {
                tl.x -= zoom;
                tl.y -= zoom;
                br.x += zoom * 2.0;
                br.y += zoom * 2.0;
            }

            c = GREEN;
            ctx->b = b;
        } else {
            c = GRAY70;
        }

        wid_set_tl_br_pct(b, tl, br);
        wid_set_color(b, WID_COLOR_TEXT, c);
        wid_set_font(b, font);
        wid_set_bevel(b, 1);

        c = WHITE;
        c.a = 10;
        wid_set_color(b, WID_COLOR_TL, c);
        wid_set_color(b, WID_COLOR_BR, c);

        if ((x < WID_EDITOR_MENU_MAP_ACROSS) && 
            (y < WID_EDITOR_MENU_MAP_DOWN) &&
            (ctx->map_highlight[mx][my])) {
            color c = GREEN;
            c.a = 100;
            wid_set_color(b, WID_COLOR_BG, c);
        } else if ((x == ctx->focus_x) && (y == ctx->focus_y)) {
            color c = RED;
            c.a = 100;
            wid_set_color(b, WID_COLOR_BG, c);
        } else if (is_a_map_tile) {
            color c = WHITE;
            c.a = 0;
            wid_set_color(b, WID_COLOR_BG, c);
        } else {
            color c = GRAY;
            c.a = 50;
            wid_set_color(b, WID_COLOR_BG, c);
        }
    } }

    wid_editor_update_layer_mode_buttons();
    wid_editor_update_edit_mode_buttons();
    wid_editor_update_tile_mode_buttons();
    wid_update(wid_editor_window);
}

static void wid_editor_button_display (widp w, fpoint tl, fpoint br)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    if (wid_choose_title_dialog) {
        return;
    }

    int xy = (typeof(xy)) (uintptr_t) wid_get_client_context2(w);
    int x = (xy & 0xff);
    int y = (xy & 0xff00) >> 8;

    if (ctx->tile_mode) {
        tpp tp = ctx->tile[x][y].tile_tp;
        if (!tp) {
            return;
        }

        tilep tile = wid_editor_tp_to_tile(tp);
        if (!tp) {
            return;
        }

        blit_init();
        glcolor(WHITE);
        tile_blit_fat(tile, 0, tl, br);
        blit_flush();
        return;
    }

    if (x >= WID_EDITOR_MENU_MAP_ACROSS) {
        return;
    }

    if (y >= WID_EDITOR_MENU_MAP_DOWN) {
        return;
    }

    x += ctx->map_x;
    y += ctx->map_y;

    if ((x < 0) || (x >= MAP_WIDTH) || (y < 0) || (y >= MAP_HEIGHT)) {
        ERR("overflow on map coords (%d, %d)", x, y);
    }

    double width = br.x - tl.x;
    double height = br.y - tl.y;

    br.x = tl.x + width;
    tl.y = br.y - height;

    blit_init();

    int z;
    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        tpp tp = ctx->map.tile[x][y][z].tp;
        if (!tp) {
            continue;
        }

        if (ctx->layer_mode != WID_EDITOR_MODE2_FILTER_ALL) {
            if (z != layer_to_depth(ctx->layer_mode)) {
                continue;
            }
        }

        fpoint btl = tl;
        fpoint bbr = br;

        if (tp_is_wall(tp) || tp_is_door(tp)) {
            btl.y -= height / 3.0;
            bbr.x += width / 3.0;
        }

        if (z == MAP_DEPTH_WALL) {
            tilep tile = ctx->map.map_tile[x][y];
            if (tile) {
                glcolor(WHITE);
                tile_blit_fat(tile, 0, btl, bbr);
                continue;
            }
        }

        tilep tile = wid_editor_tp_to_tile(tp);
        if (!tp) {
            continue;
        }

        thing_template_data *data = &ctx->map.tile[x][y][z].data;
        color c = data->col;
        c.a = 255;
        glcolor(c);

        tile_blit_fat(tile, 0, btl, bbr);
    }

    blit_flush();

    char *tmp = dynprintf("top left: %%%%fg=red$(%d, %d)", ctx->map_x, ctx->map_y);
    ttf_puts(small_font, tmp, 0, 40, 1.0, 1.0, false);
    myfree(tmp);

    tmp = dynprintf("window: %%%%fg=yellow$(%d, %d)", ctx->focus_x, ctx->focus_y);
    ttf_puts(small_font, tmp, 0, 60, 1.0, 1.0, false);
    myfree(tmp);

    tmp = dynprintf("map: %%%%fg=yellow$(%d, %d)", ctx->map_x + ctx->focus_x, ctx->map_y + ctx->focus_y);
    ttf_puts(small_font, tmp, 0, 80, 1.0, 1.0, false);
    myfree(tmp);
}

static void wid_editor_focus_right (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    if (!ctx) {
        return;
    }

    ctx->focus_x++;
    if (ctx->focus_x > WID_EDITOR_MENU_CELLS_ACROSS - 1) {
        ctx->focus_x = WID_EDITOR_MENU_CELLS_ACROSS - 1;
        wid_editor_map_scroll(1, 0);
    }
}

static void wid_editor_focus_left (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    if (!ctx) {
        return;
    }

    ctx->focus_x--;
    if (ctx->focus_x < 0) {
        ctx->focus_x = 0;
        wid_editor_map_scroll(-1, 0);
    }
}

static void wid_editor_focus_down (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    if (!ctx) {
        return;
    }

    ctx->focus_y++;
    if (ctx->focus_y > WID_EDITOR_MENU_CELLS_DOWN - 1) {
        ctx->focus_y = WID_EDITOR_MENU_CELLS_DOWN - 1;
        wid_editor_map_scroll(0, 1);
    }
}

static void wid_editor_focus_up (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    if (!ctx) {
        return;
    }

    ctx->focus_y--;
    if (ctx->focus_y < 0) {
        ctx->focus_y = 0;
        wid_editor_map_scroll(0, -1);
    }
}

static void wid_editor_set_focus (wid_editor_ctx *ctx, int focus_x, int focus_y)
{
    ctx->focus_x = focus_x;
    ctx->focus_y = focus_y;

    wid_editor_update_buttons();
}

static void wid_editor_tile_pool_find_focus (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    /*
     * If in tile mode now, find the old focus.
     */
    if (!ctx->tile_mode) {
        return;
    }

    wid_editor_update_buttons();

    int x, y;

    for (x = 0; x < WID_EDITOR_MENU_TILES_ACROSS; x++) {
        for (y = 0; y < WID_EDITOR_MENU_TILES_DOWN; y++) {
            widp b = ctx->tile[x][y].button;
            if (!b) {
                continue;
            }

            tpp tp = wid_get_thing_template(b);
            if (!tp) {
                continue;
            }

            if (tp == wid_editor_chosen_tile[ctx->tile_pool]) {
                ctx->focus_x = x;
                ctx->focus_y = y;
                return;
            }
        }
    }
}

static void wid_editor_tile_mode_toggle (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    if (time_get_time_ms() - ctx->mode_toggled < 100) {
        return;
    }

    ctx->mode_toggled = time_get_time_ms();
    ctx->tile_mode = !ctx->tile_mode;

    /*
     * If in tile mode now, find the old focus.
     */
    if (ctx->tile_mode) {
        saved_focus_x = ctx->focus_x;
        saved_focus_y = ctx->focus_y;
        saved_map_x = ctx->map_x;
        saved_map_y = ctx->map_y;

        wid_editor_tile_pool_find_focus();
    } else {
        /*
         * Now in map mode. Clear out the tile buttons.
         */
        int x, y;

        for (x = 0; x < WID_EDITOR_MENU_TILES_ACROSS; x++) {
            for (y = 0; y < WID_EDITOR_MENU_TILES_DOWN; y++) {
                widp b = ctx->tile[x][y].button;
                if (b) {
                    wid_set_thing_template(b, 0);
                }
            }
        }

        ctx->focus_x = saved_focus_x;
        ctx->focus_y = saved_focus_y;
        ctx->map_x = saved_map_x;
        ctx->map_y = saved_map_y;
    }
}

static void wid_editor_color_selected (const char *color)
{
    wid_choose_color_dialog = 0;

    wid_editor_ctx *ctx = wid_editor_window_ctx;

    ctx->col_name = (char*) color;
    ctx->col = color_find(ctx->col_name);
}

static void wid_editor_color_cancelled (void)
{
    wid_choose_color_dialog = 0;
}

static void wid_editor_color_choose (void)
{
    if (wid_choose_color_dialog) {
        return;
    }

    wid_choose_color_dialog = wid_cmap("Choose color",
                                       wid_editor_color_selected, 
                                       wid_editor_color_cancelled);

    wid_editor_set_mode(WID_EDITOR_MODE_PAINT);
}

static void wid_editor_tile_mode_set (int val)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    if (ctx->tile_mode == val) {
        return;
    }

    wid_editor_tile_mode_toggle();
}

static void wid_editor_map_scroll (int dx, int dy)
{
    static int recursion;

    if (recursion) {
        return;
    }
    recursion = 1;

    wid_editor_ctx *ctx = wid_editor_window_ctx;
    if (!ctx) {
        recursion = 0;
        return;
    }

    verify(ctx);
    verify(ctx->w);

    if (ctx->tile_mode) {
        recursion = 0;
        return;
    }

    if (dx > 0) {
        if (ctx->map_x + WID_EDITOR_MENU_MAP_ACROSS < MAP_WIDTH) {
            ctx->map_x++;
        } else {
            wid_editor_focus_right();
        }
    } else if (dx < 0) {
        if (ctx->map_x > 0) {
            ctx->map_x--;
        } else {
            wid_editor_focus_left();
        }
    }

    if (dy > 0) {
        if (ctx->map_y + WID_EDITOR_MENU_MAP_DOWN < MAP_HEIGHT) {
            ctx->map_y++;
        } else {
            wid_editor_focus_down();
        }
    } else if (dy < 0) {
        if (ctx->map_y > 0) {
            ctx->map_y--;
        } else {
            wid_editor_focus_up();
        }
    }

    recursion = 0;
}

static void wid_editor_mx_my (int *mx, int *my)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    *mx = ctx->focus_x + ctx->map_x;
    *my = ctx->focus_y + ctx->map_y;

    if (*mx < 0) {
        *mx = 0;
    }

    if (*my < 0) {
        *my = 0;
    }

    if (*mx >= MAP_WIDTH) {
        *mx = MAP_WIDTH - 1;
    }

    if (*my > MAP_HEIGHT) {
        *my = MAP_HEIGHT - 1;
    }
}
                              
static void wid_editor_exit_selected (level_pos_t p)
{
    wid_editor_map_dialog = 0;

    wid_editor_ctx *ctx = wid_editor_window_ctx;
    int mx;
    int my;
    int z;

    wid_editor_mx_my(&mx, &my);

    /*
     * Find the exit and update its position.
     */
    for (z = MAP_DEPTH_MAX - 1; z >= 0; z--) {
        tpp tp = ctx->map.tile[mx][my][z].tp;
        if (!tp) {
            continue;
        }

        if (tp_is_exit(tp)) {
            wid_editor_map_tile *tile = &ctx->map.tile[mx][my][z];
            thing_template_data *data = &tile->data;

            data->exit = p;
            data->exit_set = true;
        }
    }

    map_editor_fixup();

    wid_editor_undo_save();
}

static void wid_editor_exit_cancelled (void)
{
    wid_editor_map_dialog = 0;
}

static void wid_editor_text_selected (widp w, const char *text)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    int mx;
    int my;
    int z;

    wid_editor_mx_my(&mx, &my);

    /*
     * Find the text and update its position.
     */
    for (z = MAP_DEPTH_MAX - 1; z >= 0; z--) {
        tpp tp = ctx->map.tile[mx][my][z].tp;
        if (!tp) {
            continue;
        }

        if (tp_is_action_text(tp)) {
            wid_editor_map_tile *tile = &ctx->map.tile[mx][my][z];
            thing_template_data *data = &tile->data;

            memset(data->text, 0, sizeof(data->text));
            strncpy(data->text, text, sizeof(data->text) - 1);

            LOG("Set action text as [%s]",data->text);
        }
    }

    wid_editor_undo_save();

    wid_destroy(&wid_choose_text_dialog);
}

static const char *wid_editor_get_action_text (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    int mx;
    int my;
    int z;

    wid_editor_mx_my(&mx, &my);

    /*
     * Find the text and update its position.
     */
    for (z = MAP_DEPTH_MAX - 1; z >= 0; z--) {
        tpp tp = ctx->map.tile[mx][my][z].tp;
        if (!tp) {
            continue;
        }

        if (tp_is_action_text(tp)) {
            wid_editor_map_tile *tile = &ctx->map.tile[mx][my][z];
            thing_template_data *data = &tile->data;

            return (data->text);
        }
    }

    return (0);
}

static void wid_editor_map_thing_replace (int x, int y, int interactive)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    if ((x < 0) || (y < 0) ||
        (x >= MAP_WIDTH) ||
        (y >= MAP_HEIGHT)) {
        ERR("wid_editor_map_thing_replace, bad map coord %d,%d", x, y);
        return;
    }

    tpp tp = wid_editor_chosen_tile[ctx->tile_pool];
    if (!tp) {
        return;
    }

    int z = tp_get_z_depth(tp);

    if (tp_is_exit(tp)) {
        if (wid_editor_map_dialog) {
            return;
        }

        wid_editor_set_new_tp(x, y, z, tp, 0);

        wid_editor_map_dialog = wid_map("Choose optional destination",
                                        wid_editor_exit_selected, 
                                        wid_editor_exit_cancelled);
    } else if (tp_is_action_text(tp)) {

        if (wid_choose_text_dialog) {
            return;
        }

        const char *wid_text = wid_editor_get_action_text();
        char *existing_text = wid_text ? dupstr(wid_text, "tmp") : 0;

        wid_editor_set_new_tp(x, y, z, tp, 0);

        wid_choose_text_dialog = wid_keyboard(existing_text,
                                              "Enter text to appear",
                                              wid_editor_text_selected,
                                              wid_editor_text_selected);
        myfree(existing_text);

    } else {
        wid_editor_set_new_tp(x, y, z, tp, 0);
    }
}

static void wid_editor_map_thing_paint (int x, int y)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    if ((x < 0) || (y < 0) ||
        (x >= MAP_WIDTH) ||
        (y >= MAP_HEIGHT)) {
        ERR("bad map coord %d,%d", x, y);
        return;
    }

    int z;

    for (z = MAP_DEPTH_MAX - 1; z >= 0; z--) {
        tpp tp = ctx->map.tile[x][y][z].tp;
        if (tp) {
            ctx->map.tile[x][y][z].data.col = ctx->col;
            ctx->map.tile[x][y][z].data.col_name = ctx->col_name;
            return;
        }
    }
}

static tpp wid_editor_map_thing_get (int x, int y,
                                     color *col,
                                     const char **col_name)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    if ((x < 0) || (y < 0) ||
        (x >= MAP_WIDTH) ||
        (y >= MAP_HEIGHT)) {
        return (0);
    }

    int z;
    for (z = MAP_DEPTH_MAX - 1; z >= 0; z--) {
        if (ctx->layer_mode != WID_EDITOR_MODE2_FILTER_ALL) {
            if (z != layer_to_depth(ctx->layer_mode)) {
                continue;
            }
        }

        tpp tp = ctx->map.tile[x][y][z].tp;
        if (tp) {
            *col = ctx->map.tile[x][y][z].data.col;
            *col_name = ctx->map.tile[x][y][z].data.col_name;
            return (tp);
        }
    }

    return (0);
}

static void wid_editor_map_highlight_replace (int x, int y)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    if ((x < 0) || (y < 0) ||
        (x >= MAP_WIDTH) ||
        (y >= MAP_HEIGHT)) {
        ERR("wid_editor_map_highlight_replace, bad map coord %d,%d", x, y);
        return;
    }

    ctx->map_highlight[x][y] = 1;
}

static void do_wid_editor_line (int x0_in, 
                                int y0_in, 
                                int x1_in, 
                                int y1_in, 
                                int flag)
{
    double temp;
    double dx;
    double dy;
    double tdy;
    double dydx;
    double p;
    double x;
    double y;
    double i;

    double x0 = x0_in;
    double y0 = y0_in;
    double x1 = x1_in;
    double y1 = y1_in;

    if (x0 > x1) {
        temp = x0;
        x0 = x1;
        x1 = temp;

        temp = y0;
        y0 = y1;
        y1 = temp;
    }

    dx = x1 - x0;
    dy = y1 - y0;

    tdy = 2.0 * dy;
    dydx = tdy - (2.0 * dx);

    p = tdy - dx;
    x = x0;
    y = y0;

    if (flag == 0) {
        wid_editor_map_thing_replace((int)x, (int)y, false /* interactive */);
    } else if (flag == 1) {
        wid_editor_map_thing_replace((int)y, (int)x, false /* interactive */);
    } else if (flag == 2) {
        wid_editor_map_thing_replace((int)y, (int)-x, false /* interactive */);
    } else if (flag == 3) {
        wid_editor_map_thing_replace((int)x, (int)-y, false /* interactive */);
    }

    for (i = 1; i <= dx; i++){
        x++;

        if (p < 0) {
            p += tdy;
        } else {
            p += dydx;
            y++;
        }

        if (flag == 0) {
            wid_editor_map_thing_replace((int)x, (int)y, false /* interactive */);
        } else if (flag == 1) {
            wid_editor_map_thing_replace((int)y, (int)x, false /* interactive */);
        } else if (flag == 2) {
            wid_editor_map_thing_replace((int)y, (int)-x, false /* interactive */);
        } else if (flag == 3) {
            wid_editor_map_thing_replace((int)x, (int)-y, false /* interactive */);
        }
    }
}

static void wid_editor_draw_line (int x0, int y0, int x1, int y1)
{
    double slope = 100.0;

    if (x0 != x1) {
        slope = (y1 - y0) * (1.0 / (x1 - x0));
    }

    if ((0 <= slope) && (slope <= 1)) {
        do_wid_editor_line(x0, y0, x1, y1, 0);
    } else if ((-1 <= slope) && (slope <= 0)) {
        do_wid_editor_line(x0, -y0, x1, -y1, 3);
    } else if (slope > 1) {
        do_wid_editor_line(y0, x0, y1, x1, 1);
    } else {
        do_wid_editor_line(-y0, x0, -y1, x1, 2);
    }
}

static void do_wid_editor_highlight_line (int x0_in, 
                                          int y0_in, 
                                          int x1_in, 
                                          int y1_in, 
                                          int flag)
{
    double temp;
    double dx;
    double dy;
    double tdy;
    double dydx;
    double p;
    double x;
    double y;
    double i;

    double x0 = x0_in;
    double y0 = y0_in;
    double x1 = x1_in;
    double y1 = y1_in;

    if (x0 > x1) {
        temp = x0;
        x0 = x1;
        x1 = temp;

        temp = y0;
        y0 = y1;
        y1 = temp;
    }

    dx = x1 - x0;
    dy = y1 - y0;

    tdy = 2.0 * dy;
    dydx = tdy - (2.0 * dx);

    p = tdy - dx;
    x = x0;
    y = y0;

    if (flag == 0) {
        wid_editor_map_highlight_replace((int)x, (int)y);
    } else if (flag == 1) {
        wid_editor_map_highlight_replace((int)y, (int)x);
    } else if (flag == 2) {
        wid_editor_map_highlight_replace((int)y, (int)-x);
    } else if (flag == 3) {
        wid_editor_map_highlight_replace((int)x, (int)-y);
    }

    for (i = 1; i <= dx; i++){
        x++;

        if (p < 0) {
            p += tdy;
        } else {
            p += dydx;
            y++;
        }

        if (flag == 0) {
            wid_editor_map_highlight_replace((int)x, (int)y);
        } else if (flag == 1) {
            wid_editor_map_highlight_replace((int)y, (int)x);
        } else if (flag == 2) {
            wid_editor_map_highlight_replace((int)y, (int)-x);
        } else if (flag == 3) {
            wid_editor_map_highlight_replace((int)x, (int)-y);
        }
    }
}

static void wid_editor_draw_shape (int x0, int y0, int x1, int y1, int sides)
{
    double height = fabs((double)(y1 - y0)) / 2.0;
    double width = fabs((double)(x1 - x0)) / 2.0;

    if ((width == 0.0) || (height == 0.0)) {
        return;
    }

    double cx = ((x0 + x1) / 2) + 0.5;
    double cy = ((y0 + y1) / 2) + 0.5;

    double step = RAD_360 / (double) sides;
    double rad;

    for (rad = 0; rad < RAD_360; rad += step) {

        double x0 = cos(rad);
        double y0 = sin(rad);
        double x1 = cos(rad + step);
        double y1 = sin(rad + step);

        x0 *= width;
        y0 *= height;
        x1 *= width;
        y1 *= height;

        x0 += cx;
        y0 += cy;

        x1 += cx;
        y1 += cy;

        wid_editor_draw_line(x0, y0, x1, y1);
    }
}

static void wid_editor_draw_square (int x0, int y0, int x1, int y1)
{
    wid_editor_draw_line(x0, y0, x1, y0);
    wid_editor_draw_line(x0, y0, x0, y1);
    wid_editor_draw_line(x1, y1, x1, y0);
    wid_editor_draw_line(x1, y1, x0, y1);

    map_editor_fixup();
    wid_editor_undo_save();
}

static void wid_editor_draw_circle (int x0, int y0, int x1, int y1)
{
    wid_editor_draw_shape(x0, y0, x1, y1, 256);
    map_editor_fixup();
    wid_editor_undo_save();
}

static void wid_editor_draw_octagon (int x0, int y0, int x1, int y1)
{
    wid_editor_draw_shape(x0, y0, x1, y1, 8);
    map_editor_fixup();
    wid_editor_undo_save();
}

static void wid_editor_paste (int mx, int my)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    int x0 = ctx->cut_start_x;
    int x1 = ctx->cut_end_x;

    int y0 = ctx->cut_start_y;
    int y1 = ctx->cut_end_y;

    if (x0 > x1) {
        swap(x0, x1);
    }
    if (y0 > y1) {
        swap(y0, y1);
    }

    int x, y, z;

    for (x = x0; x <= x1; x++) {
        for (y = y0; y <= y1; y++) {
            for (z = 0; z < MAP_DEPTH_MAX; z++) {
                tpp tp = ctx->map_copy.tile[x][y][z].tp;
                if (!tp) {
                    continue;
                }

                int px = mx + (x - x0);
                int py = my + (y - y0);

                if ((px < 0) || (py < 0) ||
                    (px >= MAP_WIDTH) ||
                    (py >= MAP_HEIGHT)) {
                    continue;
                }

                wid_editor_set_new_tp(px, py, z, tp, 0);
            }
        }
    }

    map_editor_fixup();

    wid_editor_undo_save();
}

static void wid_editor_cut (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    int x0 = ctx->cut_start_x;
    int x1 = ctx->cut_end_x;

    int y0 = ctx->cut_start_y;
    int y1 = ctx->cut_end_y;

    if (x0 > x1) {
        swap(x0, x1);
    }
    if (y0 > y1) {
        swap(y0, y1);
    }

    int x, y, z;

    for (x = x0; x <= x1; x++) {
        for (y = y0; y <= y1; y++) {
            for (z = 0; z < MAP_DEPTH_MAX; z++) {

                if ((x < 0) || (y < 0) ||
                    (x >= MAP_WIDTH) ||
                    (y >= MAP_HEIGHT)) {
                    continue;
                }

                wid_editor_set_new_tp(x, y, z, 0, 0);
            }
        }
    }

    map_editor_fixup();

    wid_editor_undo_save();
}

static void wid_editor_draw_highlight_square (int x0, int y0, int x1, int y1)
{
    int x, y;

    if (x0 > x1) {
        swap(x0, x1);
    }
    if (y0 > y1) {
        swap(y0, y1);
    }

    for (x = x0; x <= x1; x++) {
        wid_editor_map_highlight_replace(x, y0);
        wid_editor_map_highlight_replace(x, y1);
    }

    for (y = y0; y <= y1; y++) {
        wid_editor_map_highlight_replace(x0, y);
        wid_editor_map_highlight_replace(x1, y);
    }
}

static void wid_editor_draw_highlight_cut (int x0, int y0, int x1, int y1)
{
    int x, y;

    if (x0 > x1) {
        swap(x0, x1);
    }
    if (y0 > y1) {
        swap(y0, y1);
    }

    for (x = x0; x <= x1; x++) {
        for (y = y0; y <= y1; y++) {
            wid_editor_map_highlight_replace(x, y);
        }
    }
}

static void wid_editor_draw_highlight_line (int x0, int y0, int x1, int y1)
{
    if ((x0 == -1) || (y0 == -1)) {
        return;
    }

    if ((x1 == -1) || (y1 == -1)) {
        return;
    }

    double slope = 100.0;

    if (x0 != x1) {
        slope = (y1 - y0) * (1.0 / (x1 - x0));
    }

    if ((0 <= slope) && (slope <= 1)) {
        do_wid_editor_highlight_line(x0, y0, x1, y1, 0);
    } else if ((-1 <= slope) && (slope <= 0)) {
        do_wid_editor_highlight_line(x0, -y0, x1, -y1, 3);
    } else if (slope > 1) {
        do_wid_editor_highlight_line(y0, x0, y1, x1, 1);
    } else {
        do_wid_editor_highlight_line(-y0, x0, -y1, x1, 2);
    }
}

static void wid_editor_undo_save (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    /*
     * Check there is a change
     */
    if (ctx->undo_at != -1) {
        if (!memcmp(&ctx->map_undo[ctx->undo_at], &ctx->map, 
                    sizeof(ctx->map))) {
            return;
        }
    }

    ctx->undo_at++;
    if (ctx->undo_at >= WID_EDITOR_UNDO) {
        ctx->undo_at--;

        /*
         * Reached the undo limit, drop an entry off the head.
         */
        int i;

        for (i = 0; i < ctx->undo_at; i++) {
            memcpy(&ctx->map_undo[i], &ctx->map_undo[i+1], sizeof(ctx->map));
        }
    }

    if (ctx->undo_at > 0) {
        ctx->save_needed = true;
    }

    memcpy(&ctx->map_undo[ctx->undo_at], &ctx->map, sizeof(ctx->map));
    ctx->valid_undo[ctx->undo_at] = 1;
}

static void wid_editor_undo (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    int old = ctx->undo_at;

    ctx->undo_at--;
    if (ctx->undo_at < 0) {
        ctx->undo_at = 0;
    }

    if (!ctx->valid_undo[ctx->undo_at]) {
        ctx->undo_at = old;
        return;
    }

    memcpy(&ctx->map, &ctx->map_undo[ctx->undo_at], sizeof(ctx->map));

    map_editor_fixup();
}

static void wid_editor_redo (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    int old = ctx->undo_at;

    ctx->undo_at++;
    if (ctx->undo_at > WID_EDITOR_UNDO) {
        ctx->undo_at = 0;
    }

    if (!ctx->valid_undo[ctx->undo_at]) {
        ctx->undo_at = old;
        return;
    }

    memcpy(&ctx->map, &ctx->map_undo[ctx->undo_at], sizeof(ctx->map));

    map_editor_fixup();
}

static void wid_editor_nuke (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    memset(&ctx->map, 0, sizeof(ctx->map));

    map_editor_fixup();
}

static void wid_editor_test (void)
{
    level_pos_t level_pos;
    level_pos.x = 66;
    level_pos.y = 66;

    char *tmp = dynprintf("%s%d.%d", LEVELS_PATH, level_pos.y, level_pos.x);
    LOG("Client: Save test level %s", tmp);
    wid_editor_save(tmp, true /* is_test_level */);
    myfree(tmp);

    wid_destroy(&wid_editor_background);
    wid_destroy(&wid_editor_window);

    LOG("Client: Test selected level %d.%d", level_pos.y, level_pos.x);
    global_config.stats.level_pos = level_pos;
    wid_intro_single_play_selected(level_pos);
}

static void wid_editor_32x32 (void)
{
    tpp rock;
    tpp floor;
    tpp wall;

    for (;;) {
        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_shop_floor(tp)) {
            continue;
        }

        if (tp_is_light_source(tp)) {
            continue;
        }

        if (tp_is_dungeon_floor(tp)) {
            floor = tp;
            break;
        }
    }

    for (;;) {
        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);
        if (tp_is_rock(tp)) {
            rock = tp;
            break;
        }
    }

    for (;;) {
        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);
        if (tp_is_wall(tp)) {
            wall = tp;
            break;
        }
    }

    int x, y, z;

    for (x = 0; x < 32; x++) {
        z = MAP_DEPTH_WALL;
        wid_editor_set_new_tp(x, 0, z, wall, 0);
        wid_editor_set_new_tp(x, 32-1, z, wall, 0);
    }

    for (y = 0; y < 32; y++) {
        z = MAP_DEPTH_WALL;
        wid_editor_set_new_tp(0, y, z, wall, 0);
        wid_editor_set_new_tp(32-1, y, z, wall, 0);
    }

    for (x = 0; x < 32; x++) {
        for (y = 0; y < 32; y++) {
            z = MAP_DEPTH_FLOOR;
            wid_editor_set_new_tp(x, y, z, floor, 0);
        }
    }

    map_editor_fixup();

    wid_editor_center();
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    wid_editor_chosen_tile[ctx->tile_pool] = rock;
    wid_editor_tile_fill(0, 0);
    wid_editor_undo_save();
}

static void wid_editor_64x64 (void)
{
    tpp rock;
    tpp floor;
    tpp wall;

    for (;;) {
        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_shop_floor(tp)) {
            continue;
        }

        if (tp_is_light_source(tp)) {
            continue;
        }

        if (tp_is_dungeon_floor(tp)) {
            floor = tp;
            break;
        }
    }

    for (;;) {
        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);
        if (tp_is_rock(tp)) {
            rock = tp;
            break;
        }
    }

    for (;;) {
        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);
        if (tp_is_wall(tp)) {
            wall = tp;
            break;
        }
    }

    int x, y, z;

    for (x = 0; x < 64; x++) {
        z = MAP_DEPTH_WALL;
        wid_editor_set_new_tp(x, 0, z, wall, 0);
        wid_editor_set_new_tp(x, 64-1, z, wall, 0);
    }

    for (y = 0; y < 64; y++) {
        z = MAP_DEPTH_WALL;
        wid_editor_set_new_tp(0, y, z, wall, 0);
        wid_editor_set_new_tp(64-1, y, z, wall, 0);
    }

    for (x = 0; x < 64; x++) {
        for (y = 0; y < 64; y++) {
            z = MAP_DEPTH_FLOOR;
            wid_editor_set_new_tp(x, y, z, floor, 0);
        }
    }

    map_editor_fixup();

    wid_editor_center();
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    wid_editor_chosen_tile[ctx->tile_pool] = rock;
    wid_editor_tile_fill(0, 0);
    wid_editor_undo_save();
}

static void wid_editor_96x96 (void)
{
    tpp floor;
    tpp wall;

    for (;;) {
        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_shop_floor(tp)) {
            continue;
        }

        if (tp_is_light_source(tp)) {
            continue;
        }

        if (tp_is_dungeon_floor(tp)) {
            floor = tp;
            break;
        }
    }

    for (;;) {
        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);
        if (tp_is_wall(tp)) {
            /*
             * Exclude lit walls as we run out of light sources
             */
            if (tp_is_light_source(tp)) {
                continue;
            }

            wall = tp;
            break;
        }
    }

    int x, y, z;

    for (x = 0; x < MAP_WIDTH; x++) {
        z = MAP_DEPTH_WALL;
        wid_editor_set_new_tp(x, 0, z, wall, 0);
        wid_editor_set_new_tp(x, MAP_HEIGHT-1, z, wall, 0);
    }

    for (y = 0; y < MAP_HEIGHT; y++) {
        z = MAP_DEPTH_WALL;
        wid_editor_set_new_tp(0, y, z, wall, 0);
        wid_editor_set_new_tp(MAP_WIDTH-1, y-1, z, wall, 0);
    }

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            z = MAP_DEPTH_FLOOR;
            wid_editor_set_new_tp(x, y, z, floor, 0);
        }
    }

    map_editor_fixup();

    wid_editor_undo_save();
}

static void wid_editor_center (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    int minx = -1;
    int x, y, z;

    for (x = 0; (x < MAP_WIDTH) && (minx == -1); x++) {
        for (y = 0; (y < MAP_HEIGHT) && (minx == -1); y++) {
            for (z = 0; (z < MAP_DEPTH_MAX) && (minx == -1); z++) {
                if (ctx->map.tile[x][y][z].tp) {
                    minx = x;
                }
            }
        }
    }

    if (minx == -1) {
        return;
    }

    int maxx = -1;

    for (x = MAP_WIDTH - 1; (x >= 0) && (maxx == -1); x--) {
        for (y = 0; (y < MAP_HEIGHT) && (maxx == -1); y++) {
            for (z = 0; (z < MAP_DEPTH_MAX) && (maxx == -1); z++) {
                if (ctx->map.tile[x][y][z].tp) {
                    maxx = x;
                }
            }
        }
    }

    if (maxx == -1) {
        return;
    }

    int miny = -1;

    for (y = 0; (y < MAP_HEIGHT) && (miny == -1); y++) {
        for (x = 0; (x < MAP_WIDTH) && (miny == -1); x++) {
            for (z = 0; (z < MAP_DEPTH_MAX) && (miny == -1); z++) {
                if (ctx->map.tile[x][y][z].tp) {
                    miny = y;
                }
            }
        }
    }

    if (miny == -1) {
        return;
    }

    int maxy = -1;

    for (y = (MAP_HEIGHT - 1); (y >= 0) && (maxy == -1); y--) {
        for (x = 0; (x < MAP_WIDTH) && (maxy == -1); x++) {
            for (z = 0; (z < MAP_DEPTH_MAX) && (maxy == -1); z++) {
                if (ctx->map.tile[x][y][z].tp) {
                    maxy = y;
                }
            }
        }
    }

    if (maxy == -1) {
        return;
    }

    ctx->cut_start_x = minx;
    ctx->cut_start_y = miny;
    ctx->cut_end_x = maxx;
    ctx->cut_end_y = maxy;

    memcpy(&ctx->map_copy, &ctx->map, sizeof(ctx->map));
    wid_editor_cut();

    int mx = (minx + (MAP_WIDTH - maxx)) / 2;
    int my = (miny + (MAP_HEIGHT - maxy)) / 2;

    wid_editor_paste(mx, my);
}

static void wid_editor_style (void)
{
    tpp floor;
    tpp wall;

    for (;;) {
        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);

        if (tp_is_shop_floor(tp)) {
            continue;
        }

        if (tp_is_light_source(tp)) {
            /*
             * No light embers
             */
            continue;
        }

        if (tp_is_dungeon_floor(tp)) {
            floor = tp;
            break;
        }
    }

    for (;;) {
        uint16_t id = myrand() % THING_MAX_ID;

        tpp tp = id_to_tp(id);
        if (tp_is_wall(tp)) {
            /*
             * Exclude lit walls as we run out of light sources
             */
            if (tp_is_light_source(tp)) {
                continue;
            }

            wall = tp;
            break;
        }
    }

    wid_editor_ctx *ctx = wid_editor_window_ctx;

    int x, y, z;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            for (z = 0; z < MAP_DEPTH_MAX; z++) {
                tpp tp = ctx->map.tile[x][y][z].tp;
                if (!tp) {
                    continue;
                }

                if (tp_is_wall(tp)) {
                    wid_editor_set_new_tp(x, y, z, wall, 0);
                }

                if (tp_is_dungeon_floor(tp)) {
                    wid_editor_set_new_tp(x, y, z, floor, 0);
                }
            }
        }
    }

    map_editor_fixup();

    wid_editor_undo_save();
}

static void wid_editor_random (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    memset(&ctx->map, 0, sizeof(ctx->map));

    int depth = ((ctx->level_pos.y - 1) * LEVELS_ACROSS) + ctx->level_pos.x;

    map_jigsaw_generate(0, 0, depth, wid_editor_replace_template);

    map_editor_fixup();

    wid_editor_undo_save();
}

static void wid_editor_title_selected (widp w, const char *title)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    level_set_title(ctx->level, title);

    wid_destroy(&wid_choose_title_dialog);

    wid_editor_save_level();
}

static void wid_editor_title_cancelled (widp w, const char *title)
{
    wid_destroy(&wid_choose_title_dialog);
}

static void wid_editor_title_choose (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    if (wid_choose_title_dialog) {
        return;
    }

    wid_choose_title_dialog = wid_keyboard(level_get_title(ctx->level),
                                           "Choose title",
                                           wid_editor_title_selected,
                                           wid_editor_title_cancelled);
}

static void wid_editor_hflip (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    memcpy(&ctx->map_tmp, &ctx->map, sizeof(ctx->map));
    memset(&ctx->map, 0, sizeof(ctx->map));

    int x, y, z;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            for (z = 0; z < MAP_DEPTH_MAX; z++) {
                memcpy(&ctx->map.tile[MAP_WIDTH - 1 - x][y][z],
                       &ctx->map_tmp.tile[x][y][z],
                       sizeof(wid_editor_map_tile));
            }
        }
    }

    map_editor_fixup();

    wid_editor_undo_save();
}

static void wid_editor_vflip (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    memcpy(&ctx->map_tmp, &ctx->map, sizeof(ctx->map));
    memset(&ctx->map, 0, sizeof(ctx->map));

    int x, y, z;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            for (z = 0; z < MAP_DEPTH_MAX; z++) {
                memcpy(&ctx->map.tile[x][MAP_HEIGHT - 1 - y][z],
                       &ctx->map_tmp.tile[x][y][z],
                       sizeof(wid_editor_map_tile));
            }
        }
    }

    map_editor_fixup();

    wid_editor_undo_save();
}

static void wid_editor_rotate (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    memcpy(&ctx->map_tmp, &ctx->map, sizeof(ctx->map));
    memset(&ctx->map, 0, sizeof(ctx->map));

    int x, y, z;

    for (x = 0; x < MAP_WIDTH; x++) {
        for (y = 0; y < MAP_HEIGHT; y++) {
            for (z = 0; z < MAP_DEPTH_MAX; z++) {
                memcpy(&ctx->map.tile[x][y][z],
                       &ctx->map_tmp.tile[y][MAP_WIDTH - 1 - x][z],
                       sizeof(wid_editor_map_tile));
            }
        }
    }

    map_editor_fixup();

    wid_editor_undo_save();
}

static void wid_editor_tile_fill_ (int x, int y)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    if (!wid_editor_chosen_tile[ctx->tile_pool]) {
        return;
    }

    if ((x < 0) || (y < 0) ||
        (x >= MAP_WIDTH) ||
        (y >= MAP_HEIGHT)) {
        return;
    }

    /*
     * Check to see there is nothing here blocking us.
     */
    uint8_t min_z;
    uint8_t z;

    /*
     * Bound certain things by others. e.g. flood fill ghosts limited by 
     * walls.
     */
    tpp tp = wid_editor_chosen_tile[ctx->tile_pool];
    switch (tp_get_z_depth(tp)) {
        case MAP_DEPTH_FLOOR: 
            min_z = MAP_DEPTH_FLOOR; 
            break;
        case MAP_DEPTH_WALL: 
            min_z = MAP_DEPTH_WALL; 
            break;
        case MAP_DEPTH_OBJ: 
            min_z = MAP_DEPTH_OBJ; 
            break;
        default:
            min_z = 0;
            break;
    }

    if (tp_is_rock(tp)) {
        min_z = MAP_DEPTH_WALL; 
    }

    for (z = min_z; z < MAP_DEPTH_MAX; z++) {
        if (ctx->map.tile[x][y][z].tp) {
            return;
        }
    }

    z = tp_get_z_depth(tp);
    wid_editor_set_new_tp(x, y, z, tp, 0);

    wid_editor_tile_fill_(x + 1, y);
    wid_editor_tile_fill_(x - 1, y);
    wid_editor_tile_fill_(x, y + 1);
    wid_editor_tile_fill_(x, y - 1);
}

static void wid_editor_tile_fill (int x, int y)
{
    if ((x == -1) || (y == -1)) {
        return;
    }

    wid_editor_tile_fill_(x, y);

    map_editor_fixup();

    wid_editor_undo_save();
}

static void wid_editor_tile_left_button_pressed (int x, int y)
{
    if ((x == -1) || (y == -1)) {
        return;
    }

    wid_editor_ctx *ctx = wid_editor_window_ctx;
    int mx = -1;
    int my = -1;

    if (time_get_time_ms() - ctx->mode_toggled < 100) {
        return;
    }

    if (!ctx->tile_mode) {
        mx = x + ctx->map_x;
        my = y + ctx->map_y;
    }

    if (ctx->tile_mode) {

        widp b = ctx->tile[x][y].button;
        if (b) {
            tpp tp = wid_get_thing_template(b);
            if (tp) {
                wid_editor_chosen_tile[ctx->tile_pool] = tp;

                wid_editor_tile_mode_toggle();

                wid_editor_set_mode(WID_EDITOR_MODE_DRAW);
                return;
            }
        }

        if (y == 0) {
            if (x < WID_TILE_POOL_MAX) {
                ctx->tile_pool = x;
                wid_editor_tile_pool_find_focus();
                return;
            }
        }

    } else {
        if (wid_editor_chosen_tile[ctx->tile_pool]) {
            if ((x < WID_EDITOR_MENU_MAP_ACROSS) && 
                (y < WID_EDITOR_MENU_MAP_DOWN)) {
                switch (ctx->edit_mode) {
                case WID_EDITOR_MODE_DRAW:
                    wid_editor_map_thing_replace(mx, my, true /* interactive */);
                    map_editor_fixup();

                    wid_editor_undo_save();
                    break;

                case WID_EDITOR_MODE_PAINT:
                    wid_editor_map_thing_paint(mx, my);
                    wid_editor_undo_save();
                    break;

                case WID_EDITOR_MODE_LINE:
                    if (!ctx->got_line_start) {
                        ctx->got_line_start = true;
                        ctx->line_start_x = mx;
                        ctx->line_start_y = my;

                    } else if ((ctx->line_start_x != mx) ||
                               (ctx->line_start_y != my)) {

                        wid_editor_draw_line(ctx->line_start_x,
                                             ctx->line_start_y, mx, my);

                        map_editor_fixup();

                        wid_editor_undo_save();

                        ctx->got_line_start = false;
                    }
                    break;

                case WID_EDITOR_MODE_SQUARE:
                    if (!ctx->got_square_start) {
                        ctx->got_square_start = true;
                        ctx->square_start_x = mx;
                        ctx->square_start_y = my;

                    } else if ((ctx->square_start_x != mx) ||
                               (ctx->square_start_y != my)) {

                        wid_editor_draw_square(ctx->square_start_x,
                                               ctx->square_start_y, mx, my);
                        ctx->got_square_start = false;
                    }
                    break;

                case WID_EDITOR_MODE_CIRCLE:
                    if (!ctx->got_square_start) {
                        ctx->got_square_start = true;
                        ctx->square_start_x = mx;
                        ctx->square_start_y = my;

                    } else if ((ctx->square_start_x != mx) ||
                               (ctx->square_start_y != my)) {

                        wid_editor_draw_circle(ctx->square_start_x,
                                               ctx->square_start_y, mx, my);
                        ctx->got_square_start = false;
                    }
                    break;

                case WID_EDITOR_MODE_OCTAGON:
                    if (!ctx->got_square_start) {
                        ctx->got_square_start = true;
                        ctx->square_start_x = mx;
                        ctx->square_start_y = my;

                    } else if ((ctx->square_start_x != mx) ||
                               (ctx->square_start_y != my)) {

                        wid_editor_draw_octagon(ctx->square_start_x,
                                                ctx->square_start_y, mx, my);
                        ctx->got_square_start = false;
                    }
                    break;

                case WID_EDITOR_MODE_CUT:
                    if (!ctx->got_cut_start) {
                        ctx->got_cut_start = true;
                        ctx->cut_start_x = mx;
                        ctx->cut_start_y = my;

                    } else if ((ctx->cut_start_x != mx) ||
                               (ctx->cut_start_y != my)) {

                        /*
                         * Only do the cut if we've actually moved. Avoids
                         * double clicks messing up.
                         */
                        ctx->got_cut_start = false;
                        ctx->cut_end_x = mx;
                        ctx->cut_end_y = my;
                        wid_editor_cut();
                    }
                    break;

                case WID_EDITOR_MODE_COPY:
                    if (!ctx->got_cut_start) {
                        ctx->got_cut_start = true;
                        ctx->cut_start_x = mx;
                        ctx->cut_start_y = my;

                    } else if ((ctx->cut_start_x != mx) ||
                               (ctx->cut_start_y != my)) {

                        memcpy(&ctx->map_copy, &ctx->map, sizeof(ctx->map));
                        ctx->got_cut_start = false;
                        ctx->cut_end_x = mx;
                        ctx->cut_end_y = my;
                    }
                    break;

                case WID_EDITOR_MODE_PASTE:
                    wid_editor_paste(mx, my);
                    break;

                case WID_EDITOR_MODE_EDIT:
                case WID_EDITOR_MODE_YANK:
                    {
                        tpp tp = wid_editor_map_thing_get(mx, my, 
                                                          &ctx->col,
                                                          &ctx->col_name);
                        if (tp) {
                            wid_editor_chosen_tile[ctx->tile_pool] = tp;

                            /*
                             * Fake a cut so a 'p' can put this back.
                             */
                            memcpy(&ctx->map_copy, &ctx->map, sizeof(ctx->map));
                            ctx->cut_start_x = mx;
                            ctx->cut_end_x = mx;
                            ctx->cut_start_y = my;
                            ctx->cut_end_y = my;
                        }
                    }

                    if (ctx->edit_mode == WID_EDITOR_MODE_EDIT) {
                        wid_editor_map_thing_replace(mx, my, true /* interactive */);
                        map_editor_fixup();

                        wid_editor_undo_save();
                    }

                    wid_editor_set_mode(WID_EDITOR_MODE_DRAW);
                    break;

                case WID_EDITOR_MODE_FILL:
                    wid_editor_tile_fill(mx, my);
                    break;

                case WID_EDITOR_MODE_DEL:
                    wid_editor_tile_right_button_pressed(x, y);
                    break;
                }
            }
        }
    }

    if (x == WID_EDITOR_MENU_CELLS_ACROSS - 1) {
        switch (y) {
        case WID_EDITOR_MODE2_TITLE:
            wid_editor_title_choose();
            break;

        case WID_EDITOR_MODE2_UNUSED_5:
            break;
        case WID_EDITOR_MODE2_UNUSED_6:
            break;
        case WID_EDITOR_MODE2_UNUSED_7:
            break;
        case WID_EDITOR_MODE2_UNUSED_8:
            break;
        case WID_EDITOR_MODE2_FILTER_FLOOR:
        case WID_EDITOR_MODE2_FILTER_OBJ:
        case WID_EDITOR_MODE2_FILTER_WALL:
        case WID_EDITOR_MODE2_FILTER_ACTIONS:
        case WID_EDITOR_MODE2_FILTER_ALL:
            wid_editor_set_layer_mode(y);
            break;
        case WID_EDITOR_MODE2_64x64:
            wid_editor_tile_mode_set(false);
            wid_editor_nuke();
            wid_editor_64x64();
            wid_editor_undo_save();
            break;

        case WID_EDITOR_MODE2_96x96:
            wid_editor_tile_mode_set(false);
            wid_editor_nuke();
            wid_editor_96x96();
            wid_editor_undo_save();
            break;

        case WID_EDITOR_MODE2_32x32:
            wid_editor_tile_mode_set(false);
            wid_editor_nuke();
            wid_editor_32x32();
            wid_editor_undo_save();
            break;

        case WID_EDITOR_MODE2_CENTER:
            wid_editor_center();
            wid_editor_undo_save();
            break;
        }
    }

    if (y == WID_EDITOR_MENU_CELLS_DOWN - 1) {
        if (x < WID_EDITOR_MODE_MAX) {
            switch (x) {
            case WID_EDITOR_MODE_PAINT:
            case WID_EDITOR_MODE_DRAW:
            case WID_EDITOR_MODE_LINE:
            case WID_EDITOR_MODE_DEL:
            case WID_EDITOR_MODE_FILL:
            case WID_EDITOR_MODE_CUT:
            case WID_EDITOR_MODE_COPY:
            case WID_EDITOR_MODE_PASTE:
            case WID_EDITOR_MODE_YANK:
            case WID_EDITOR_MODE_EDIT:
            case WID_EDITOR_MODE_SQUARE:
            case WID_EDITOR_MODE_CIRCLE:
            case WID_EDITOR_MODE_OCTAGON:
                wid_editor_set_mode(x);
                break;

            case WID_EDITOR_MODE_UNDO:
                wid_editor_tile_mode_set(false);
                wid_editor_undo();
                break;

            case WID_EDITOR_MODE_REDO:
                wid_editor_tile_mode_set(false);
                wid_editor_redo();
                break;

            case WID_EDITOR_MODE_STYLE:
                wid_editor_tile_mode_set(false);
                wid_editor_style();
                break;

            case WID_EDITOR_MODE_RANDOM:
                wid_editor_tile_mode_set(false);
                wid_editor_random();
                break;

            case WID_EDITOR_MODE_TEST:
                wid_editor_tile_mode_set(false);
                wid_editor_test();
                break;

            case WID_EDITOR_MODE_HFLIP:
                wid_editor_tile_mode_set(false);
                wid_editor_hflip();
                break;

            case WID_EDITOR_MODE_VFLIP:
                wid_editor_tile_mode_set(false);
                wid_editor_vflip();
                break;

            case WID_EDITOR_MODE_ROTATE:
                wid_editor_tile_mode_set(false);
                wid_editor_rotate();
                break;

            case WID_EDITOR_MODE_SAVE:
                wid_editor_save_level();
                break;

            case WID_EDITOR_MODE_TOGGLE:
                wid_editor_tile_mode_toggle();
                break;

            case WID_EDITOR_MODE_COLOR:
                wid_editor_color_choose();
                break;
            }
        }
    }
}

static void wid_editor_tile_right_button_pressed (int x, int y)
{
    if ((x == -1) || (y == -1)) {
        return;
    }

    wid_editor_ctx *ctx = wid_editor_window_ctx;
    int mx = -1;
    int my = -1;

    if (time_get_time_ms() - ctx->mode_toggled < 100) {
        return;
    }

    if (!ctx->tile_mode) {
        mx = x + ctx->map_x;
        my = y + ctx->map_y;
    }

    if (ctx->tile_mode) {
        wid_editor_tile_mode_toggle();
        return;
    } else {
        if ((x < WID_EDITOR_MENU_MAP_ACROSS) && 
            (y < WID_EDITOR_MENU_MAP_DOWN)) {
            int z;
            for (z = MAP_DEPTH_MAX - 1; z >= 0; z--) {
                tpp tp = ctx->map.tile[mx][my][z].tp;
                if (tp) {
                    /*
                     * Fake a cut so a 'p' can put this back.
                     */
                    memcpy(&ctx->map_copy, &ctx->map, sizeof(ctx->map));
                    ctx->cut_start_x = mx;
                    ctx->cut_end_x = mx;
                    ctx->cut_start_y = my;
                    ctx->cut_end_y = my;

                    wid_editor_set_new_tp(mx, my, z, 0, 0);

                    map_editor_fixup();

                    wid_editor_undo_save();
                    return;
                }
            }
        }
    }
}

static uint8_t wid_editor_mouse_down (widp w,
                                      int mx, int my,
                                      uint32_t button)
{
    int xy = (typeof(xy)) (uintptr_t) wid_get_client_context2(w);
    int x = (xy & 0xff);
    int y = (xy & 0xff00) >> 8;

    if (wid_choose_title_dialog) {
        return (true);
    }

    if (button == 1) {
        wid_editor_tile_left_button_pressed(x, y);
    }

    if ((button == 2) || (button == 3)) {
        wid_editor_tile_right_button_pressed(x, y);
    }

    return (true);
}

static uint8_t wid_editor_key_down (widp w, const SDL_KEYSYM *key)
{
    wid_editor_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    int xy = (typeof(xy)) (uintptr_t) wid_get_client_context2(w);
    int x = ctx->focus_x;
    int y = ctx->focus_y;

    if (wid_choose_title_dialog) {
        return (true);
    }

    switch (key->mod) {
        case KMOD_LSHIFT:
        case KMOD_RSHIFT:
        switch (key->sym) {
            case SDLK_LEFT:
                wid_editor_map_scroll(-1, 0);
                return (true);

            case SDLK_RIGHT:
                wid_editor_map_scroll(1, 0);
                return (true);

            case SDLK_UP:
                wid_editor_map_scroll(0, -1);
                return (true);

            case SDLK_DOWN:
                wid_editor_map_scroll(0, 1);
                return (true);

            default:
                break;
        }

        default:
            break;
    }

    ctx->old_edit_mode = ctx->edit_mode;

    switch (key->sym) {
        case '`':
            return (false);

        case '\t':
            wid_editor_tile_mode_toggle();
            return (true);

        case ' ':
            wid_editor_tile_left_button_pressed(x, y);
            return (false);

        case SDLK_BACKSPACE:
            wid_editor_tile_mode_set(false);
            wid_editor_tile_right_button_pressed(x, y);
            return (true); 

        case 'l':
            wid_editor_tile_mode_set(false);
            wid_editor_set_mode(WID_EDITOR_MODE_LINE);
            wid_editor_tile_left_button_pressed(x, y);
            return (true);

        case 'r':
            wid_editor_tile_mode_set(false);
            wid_editor_set_mode(WID_EDITOR_MODE_SQUARE);
            wid_editor_tile_left_button_pressed(x, y);
            return (true);

        case 's':
            wid_editor_tile_mode_set(false);
            wid_editor_save_level();
            return (true);

        case 'd':
            wid_editor_tile_mode_set(false);
            wid_editor_set_mode(WID_EDITOR_MODE_DRAW);
            return (true);

        case 'p':
            wid_editor_tile_mode_set(false);
            wid_editor_set_mode(WID_EDITOR_MODE_PAINT);
            return (true);

        case 'y':
            wid_editor_tile_mode_set(false);
            wid_editor_set_mode(WID_EDITOR_MODE_YANK);
            wid_editor_tile_left_button_pressed(x, y);
            return (true);

        case 't':
            wid_editor_tile_mode_set(false);
            wid_editor_test();
            return (true);

        case 'c':
            wid_editor_tile_mode_set(false);
            wid_editor_set_mode(WID_EDITOR_MODE_COPY);
            wid_editor_tile_left_button_pressed(x, y);
            return (true);

        case 'v':
            wid_editor_tile_mode_set(false);
            wid_editor_set_mode(WID_EDITOR_MODE_PASTE);
            wid_editor_tile_left_button_pressed(x, y);
            return (true);

        case 'f':
            wid_editor_tile_mode_set(false);
            wid_editor_set_mode(WID_EDITOR_MODE_FILL);
            wid_editor_tile_left_button_pressed(x, y);
            return (true);

        case 'u':
            wid_editor_tile_mode_set(false);
            wid_editor_undo();
            return (true);

        case 'e':
            wid_editor_tile_mode_set(false);
            wid_editor_redo();
            return (true);

        case 'x':
            wid_editor_tile_mode_set(false);
            wid_editor_set_mode(WID_EDITOR_MODE_DEL);
            wid_editor_tile_left_button_pressed(x, y);
            return (true);

        case 'z':
            wid_editor_tile_mode_set(false);
            wid_editor_nuke();
            wid_editor_undo_save();
            return (true);

        case 'q':
        case SDLK_ESCAPE:

            if (ctx->got_line_start) {
                ctx->got_line_start = false;
                return (true);
            }

            if (ctx->got_square_start) {
                ctx->got_square_start = false;
                return (true);
            }

            if (ctx->got_cut_start) {
                ctx->got_cut_start = false;
                return (true);
            }

            if (ctx->edit_mode > WID_EDITOR_MODE_DRAW) {
                ctx->edit_mode = WID_EDITOR_MODE_DRAW;
                return (true);
            }

            wid_editor_hide();
            return (true);

        case SDLK_RETURN: {
            return (true);

        case SDLK_HOME:
            break;

        case SDLK_END:
            break;

        default:
            return (true);
        }
    }

    wid_editor_update_buttons();
    if (ctx->b) {
        wid_mouse_warp(ctx->b);
    }

    return (true);
}

static uint8_t wid_editor_joy_button (widp w, int mx, int my)
{
    wid_editor_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    if (wid_choose_title_dialog) {
        return (true);
    }

    int xy = (typeof(xy)) (uintptr_t) wid_get_client_context2(w);
    int x = ctx->focus_x;
    int y = ctx->focus_y;

    int ret = false;

    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
        wid_editor_tile_left_button_pressed(x, y);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_B]) {
        ret = true;
        wid_editor_hide();
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_X]) {
        wid_editor_tile_right_button_pressed(x, y);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_Y]) {
        wid_editor_tile_mode_toggle();
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_LEFT]) {
        ctx->edit_mode--;
        if (ctx->edit_mode < 0) {
            wid_editor_set_mode(0);
        }
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_RIGHT]) {
        ctx->edit_mode++;
        if (ctx->edit_mode >= WID_EDITOR_MODE_MAX) {
            wid_editor_set_mode(WID_EDITOR_MODE_MAX - 1);
        }
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_STICK_DOWN]) {
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_STICK_DOWN]) {
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_START]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_XBOX]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_BACK]) {
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_UP]) {
        wid_editor_focus_up();
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_DOWN]) {
        wid_editor_focus_down();
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT]) {
        wid_editor_focus_left();
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT]) {
        wid_editor_focus_right();
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_FIRE]) {
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_FIRE]) {
        ret = true;
    }

    wid_editor_update_buttons();
    if (ctx->b) {
        wid_mouse_warp(ctx->b);
    }

    return (ret);
}

static uint8_t wid_editor_mouse_motion (widp w,
                                        int mx, int my,
                                        int relx, int rely,
                                        int wheelx, int wheely)
{
    if (!relx && !rely && !wheelx && !wheely) {
        return (false);
    }

    if (wid_choose_title_dialog) {
        return (true);
    }

    if (wheelx < 0) {
        wid_editor_map_scroll(1, 0);
    } else if (wheelx > 0) {
        wid_editor_map_scroll(-1, 0);
    }

    if (wheely < 0) {
        wid_editor_map_scroll(0, 1);
    } else if (wheely > 0) {
        wid_editor_map_scroll(0, -1);
    }

    int mouse_x;
    int mouse_y;
    mouse_down = SDL_GetMouseState(&mouse_x, &mouse_y);

    int xy = (typeof(xy)) (uintptr_t) wid_get_client_context2(w);
    int x = (xy & 0xff);
    int y = (xy & 0xff00) >> 8;

    if (mouse_down & SDL_BUTTON_LEFT) {
        wid_editor_tile_left_button_pressed(x, y);
        return (true);
    }

    /*
     * MACOS seems bugged in SDL with this
     */
    if (mouse_down & SDL_BUTTON_RIGHT) {
        wid_editor_tile_right_button_pressed(x, y);
        return (true);
    }

    return (true);
}

static void wid_editor_mouse_over (widp w,
                                   int relx, int rely,
                                   int wheelx, int wheely)
{
    wid_editor_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    if (wid_choose_title_dialog) {
        return;
    }

    if (!relx && !rely && !wheelx && !wheely) {
        return;
    }

    /*
     * If we recreate the map with a fixed focus we will be told about
     * a mouse over event immediately which may not be over the focus item
     * and will cause us to move. Annoying.
     */
    if (time_get_time_ms() - ctx->created < 100) {
        return;
    }

    int focus = (typeof(focus)) (uintptr_t) wid_get_client_context2(w);
    int focus_x = (focus & 0xff);
    int focus_y = (focus & 0xff00) >> 8;

    wid_editor_set_focus(ctx, focus_x, focus_y);
}

static void wid_editor_destroy (widp w)
{
    wid_editor_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    wid_set_client_context(w, 0);

    int x, y, z;

    for (x = 0; x < WID_EDITOR_MENU_CELLS_ACROSS; x++) {
        for (y = 0; y < WID_EDITOR_MENU_CELLS_DOWN; y++) {

            tpp tp = ctx->tile[x][y].tile_tp;
            if (!tp) {
                continue;
            }

            ctx->tile[x][y].tile_tp = 0;

            for (z = 0; z < MAP_DEPTH_MAX; z++) {
                wid_editor_set_new_tp(x, y, z, 0, 0);
            }
        }
    }

    level_destroy(&ctx->level, false /* keep players */);

    saved_level_pos = ctx->level_pos;
    saved_focus_x = ctx->focus_x;
    saved_focus_y = ctx->focus_y;
    saved_map_x = ctx->map_x;
    saved_map_y = ctx->map_y;

    myfree(ctx);
    wid_editor_window = 0;
    wid_editor_window_ctx = 0;
}

static void wid_editor_bg_create (void)
{
    widp wid;

    if (wid_editor_background) {
        return;
    }

    {
        wid = wid_editor_background = wid_new_window("bg");

        float f = (1024.0 / 680.0);

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, f };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title5");

        wid_lower(wid);

        color c;
        c = WHITE;
        wid_set_mode(wid, WID_MODE_NORMAL);
        wid_set_color(wid, WID_COLOR_TL, c);
        wid_set_color(wid, WID_COLOR_BR, c);
        wid_set_color(wid, WID_COLOR_BG, c);

        wid_update(wid);
    }
}

static uint8_t wid_editor_load_tile (const tree_node *node, void *arg)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    tpp tp;

    tp = (typeof(tp)) 
            (((char*) node) - STRUCT_OFFSET(struct tp_, tree2));

    if (tp_is_hidden_from_editor(tp)) {
        return (true);
    }

    int tile_pool = WID_TILE_MODE_ITEMS;

    if (tp_is_wall(tp) || tp_is_rock(tp) || tp_is_crystal(tp)) {
        tile_pool = WID_TILE_MODE_WALLS;
    } else if (tp_is_door(tp)) {
        tile_pool = WID_TILE_MODE_DOORS;
    } else if (tp_is_dungeon_floor(tp) || 
               tp_is_dirt(tp) || 
               tp_is_corridor(tp) || 
               tp_is_lava(tp) || 
               tp_is_acid(tp)) {
        tile_pool = WID_TILE_MODE_FLOORS;
    } else if (tp_is_monst(tp)) {
        tile_pool = WID_TILE_MODE_MONST;
    } else if (tp_is_player(tp)) {
        tile_pool = WID_TILE_MODE_PLAYER;
    } else if (tp_is_potion(tp)) {
        tile_pool = WID_TILE_MODE_POTION;
    } else if (tp_is_key(tp)) {
        tile_pool = WID_TILE_MODE_KEYS;
    } else if (tp_is_trap(tp)) {
        tile_pool = WID_TILE_MODE_TRAP;
    } else if (tp_is_weapon(tp)) {
        tile_pool = WID_TILE_MODE_WEAPONS;
    } else if (tp_is_mob_spawner(tp)) {
        tile_pool = WID_TILE_MODE_MOB_SPAWNERS;
    } else if (tp_is_treasure(tp)) {
        tile_pool = WID_TILE_MODE_TREASURE;
    } else if (tp_is_food(tp)) {
        tile_pool = WID_TILE_MODE_FOOD;
    } else if (tp_is_action(tp)) {
        tile_pool = WID_TILE_MODE_ACTIONS;
    } else if (tp_is_world_floor(tp)) {
        tile_pool = WID_TILE_MODE_WORLD;
    } else if (tp_is_exit(tp) || 
               tp_is_teleport(tp)) {
        tile_pool = WID_TILE_MODE_EXITS;
    }

    int count = ctx->tile_count[tile_pool];
    if (count >= THING_MAX_ID) {
        ERR("too many things");
    }

    ctx->tile_pools[tile_pool][count].tile_tp = tp;
    ctx->tile_count[tile_pool]++;

    /*
     * Start out with something.
     */
    if (!wid_editor_chosen_tile[tile_pool]) {
        wid_editor_chosen_tile[tile_pool] = tp;
    }

    return (true);
}

static void wid_editor_load_tiles (void)
{
    tree_walk(thing_templates_create_order,
              wid_editor_load_tile, 0 /* arg */);
}

static void wid_editor_load_map (level_pos_t level_pos)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    levelp l = level_load(level_pos, 
                          ctx->w,
                          true, /* is_editor */
                          false, /* is_map_editor */
                          false /* on_server */);
    if (!l) {
        LOG("Failed to load level %d.%d", level_pos.y, level_pos.x);

        l = level_new(ctx->w, 
                      level_pos, 
                      true, /* is_editor */
                      false, /* is_map_editor */
                      false /* on_server */);
        if (!l) {
            ERR("failed to create level");
        }
    }

    ctx->level = l;
    ctx->loading_x = level_pos.x;
    ctx->loading_y = level_pos.y;
}

static void wid_editor_tick (widp w)
{
    /*
     * Preserve layers from modification that are not being editted.
     */
    wid_editor_overwrite_inactive_layers();

    if (!wid_is_hidden(wid_console_window)) {
        return;
    }

    if (wid_editor_save_popup || 
        wid_editor_map_dialog || 
        wid_choose_color_dialog || 
        wid_choose_text_dialog || 
        wid_choose_title_dialog) {
        return;
    }

    wid_editor_ctx *ctx = wid_editor_window_ctx;
    if (!ctx) {
        return;
    }

    memset(ctx->map_highlight, 0, sizeof(ctx->map_highlight));

    int mx, my;

    wid_editor_mx_my(&mx, &my);

    if (ctx->got_line_start) {
        wid_editor_draw_highlight_line(ctx->line_start_x, ctx->line_start_y, 
                                       mx, my);
    }

    if (ctx->got_square_start) {
        wid_editor_draw_highlight_square(ctx->square_start_x, 
                                         ctx->square_start_y,
                                         mx, my);
    }

    if (ctx->got_cut_start) {
        wid_editor_draw_highlight_cut(ctx->cut_start_x, 
                                      ctx->cut_start_y,
                                      mx, my);
    }

    int moved = 0;

    if (sdl_joy_axes) {
        /*
         * Right stick
         */

        int changed = 0;

        if (sdl_joy_axes[3] > sdl_joy_deadzone) {
            changed = 1;
        }

        if (sdl_joy_axes[3] < -sdl_joy_deadzone) {
            changed = 1;
        }

        if (sdl_joy_axes[4] > sdl_joy_deadzone) {
            changed = 1;
        }

        if (sdl_joy_axes[4] < -sdl_joy_deadzone) {
            changed = 1;
        }

        if (changed) {
            static int ts;

            if (time_have_x_thousandths_passed_since(
                                DELAY_THOUSANDTHS_PLAYER_POLL * 6, ts)) {

                ts = time_get_time_ms();

                if (sdl_joy_axes[3] > sdl_joy_deadzone) {
                    wid_editor_focus_right();
                    moved = 1;
                }

                if (sdl_joy_axes[3] < -sdl_joy_deadzone) {
                    wid_editor_focus_left();
                    moved = 1;
                }

                if (sdl_joy_axes[4] > sdl_joy_deadzone) {
                    wid_editor_focus_down();
                    moved = 1;
                }

                if (sdl_joy_axes[4] < -sdl_joy_deadzone) {
                    wid_editor_focus_up();
                    moved = 1;
                }

                if (!ctx->tile_mode &&
                    (ctx->focus_x < WID_EDITOR_MENU_MAP_ACROSS) && 
                    (ctx->focus_y < WID_EDITOR_MENU_MAP_DOWN)) {

                    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
                        wid_editor_tile_left_button_pressed(ctx->focus_x, 
                                                            ctx->focus_y);
                    }
                }
            }
        }

        /*
         * Left stick
         */
        changed = 0;

        if (sdl_joy_axes[0] > sdl_joy_deadzone) {
            changed = 1;
        }

        if (sdl_joy_axes[0] < -sdl_joy_deadzone) {
            changed = 1;
        }

        if (sdl_joy_axes[1] > sdl_joy_deadzone) {
            changed = 1;
        }

        if (sdl_joy_axes[1] < -sdl_joy_deadzone) {
            changed = 1;
        }

        if (changed) {
            static int ts;

            if (time_have_x_thousandths_passed_since(
                                DELAY_THOUSANDTHS_PLAYER_POLL, ts)) {

                ts = time_get_time_ms();

                if (sdl_joy_axes[0] > sdl_joy_deadzone) {
                    moved = 1;
                    if (ctx->tile_mode) {
                        wid_editor_focus_right();
                    } else {
                        wid_editor_map_scroll(1, 0);
                    }
                }

                if (sdl_joy_axes[0] < -sdl_joy_deadzone) {
                    moved = 1;
                    if (ctx->tile_mode) {
                        wid_editor_focus_left();
                    } else {
                        wid_editor_map_scroll(-1, 0);
                    }
                }

                if (sdl_joy_axes[1] > sdl_joy_deadzone) {
                    moved = 1;
                    if (ctx->tile_mode) {
                        wid_editor_focus_down();
                    } else {
                        wid_editor_map_scroll(0, 1);
                    }
                }

                if (sdl_joy_axes[1] < -sdl_joy_deadzone) {
                    moved = 1;
                    if (ctx->tile_mode) {
                        wid_editor_focus_up();
                    } else {
                        wid_editor_map_scroll(0, -1);
                    }
                }
            }
        }
    }

    if (!sdl_shift_held) {
#if SDL_MAJOR_VERSION == 1 && SDL_MINOR_VERSION == 2 /* { */
        uint8_t *state = SDL_GetKeyState(0);
        uint8_t right = state[SDLK_RIGHT] ? 1 : 0;
        uint8_t left  = state[SDLK_LEFT] ? 1 : 0;
        uint8_t up    = state[SDLK_UP] ? 1 : 0;
        uint8_t down  = state[SDLK_DOWN] ? 1 : 0;
        uint8_t space = state[SDLK_SPACE] ? 1 : 0;
#else /* } { */
        const uint8_t *state = SDL_GetKeyboardState(0);
        uint8_t right = state[SDL_SCANCODE_RIGHT] ? 1 : 0;
        uint8_t left  = state[SDL_SCANCODE_LEFT] ? 1 : 0;
        uint8_t up    = state[SDL_SCANCODE_UP] ? 1 : 0;
        uint8_t down  = state[SDL_SCANCODE_DOWN] ? 1 : 0;
        uint8_t space = state[SDL_SCANCODE_SPACE] ? 1 : 0;
#endif /* } */

        if (up || right || down || left || space) {
            static int ts;

            if (time_have_x_thousandths_passed_since(
                                DELAY_THOUSANDTHS_PLAYER_POLL * 6, ts)) {

                ts = time_get_time_ms();

                if (right) {
                    wid_editor_focus_right();
                    moved = 1;
                }

                if (left) {
                    wid_editor_focus_left();
                    moved = 1;
                }

                if (down) {
                    wid_editor_focus_down();
                    moved = 1;
                }

                if (up) {
                    wid_editor_focus_up();
                    moved = 1;
                }

                if (!ctx->tile_mode &&
                    (
                        (ctx->edit_mode == WID_EDITOR_MODE_DRAW)    ||
                        (ctx->edit_mode == WID_EDITOR_MODE_PAINT)
                    ) &&
                    (ctx->focus_x < WID_EDITOR_MENU_MAP_ACROSS) && 
                    (ctx->focus_y < WID_EDITOR_MENU_MAP_DOWN)) {

                    if (space) {
                        wid_editor_tile_left_button_pressed(ctx->focus_x, 
                                                            ctx->focus_y);
                    }
                }
            }
        }
    }

    wid_editor_update_buttons();

    if (moved) {
        if (ctx->b) {
            wid_mouse_warp(ctx->b);
        }
    }

    wid_editor_update_buttons();
}

static void wid_editor_save (const char *dir_and_file, int is_test_level)
{
    wid_editor_ctx *ed = wid_editor_window_ctx;

    LOG("Saving: %s", dir_and_file);

    /*
     * Write the file.
     */
    marshal_p ctx;
    ctx = marshal(dir_and_file);

    if (!ed->level) {
        ERR("no level to save");
    }

    marshal_level(ctx, ed->level);

    PUT_BRA(ctx);

    PUT_NAMED_UINT32(ctx, "width", MAP_WIDTH);
    PUT_NAMED_UINT32(ctx, "height", MAP_HEIGHT);

    int x, y, z;

    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            for (y = 0; y < MAP_HEIGHT; y++) {

                tpp tp = ed->map.tile[x][y][z].tp;
                if (!tp) {
                    continue;
                }

                PUT_BRA(ctx);

                PUT_NAMED_UINT32(ctx, "x", x);
                PUT_NAMED_UINT32(ctx, "y", y);
                PUT_NAMED_STRING(ctx, "t", tp_name(tp));

                wid_editor_map_tile *tile = &ed->map.tile[x][y][z];
                thing_template_data *data = &tile->data;

                if (data->exit_set) {
                    PUT_NAME(ctx, "exit");
                    PUT_BRA(ctx);
                    PUT_NAMED_INT8(ctx, "x", data->exit.x);
                    PUT_NAMED_INT8(ctx, "y", data->exit.y);
                    PUT_KET(ctx);
                }

                if (data->text[0]) {
                    PUT_NAME(ctx, "text");
                    PUT_BRA(ctx);
                    PUT_NAMED_STRING(ctx, "text", data->text);
                    PUT_KET(ctx);
                }

                if (data->col_name &&
                    strcmp(data->col_name, "white")) {
                    PUT_NAME(ctx, "color");
                    PUT_BRA(ctx);
                    PUT_NAMED_STRING(ctx, "color", data->col_name);
                    PUT_KET(ctx);
                }

                PUT_KET(ctx);
            }
        }
    }

    PUT_KET(ctx);
    PUT_KET(ctx); // level

    if (marshal_fini(ctx) < 0) {
        /*
         * Fail
         */
        char *popup_str = dynprintf("Failed to save %s: %s", dir_and_file,
                                    strerror(errno));

        MSG_BOX("%s", popup_str);
        myfree(popup_str);
    } else {
        if (!is_test_level) {
            /*
             * Success
             */
            char *popup_str = dynprintf("Saved %s", dir_and_file);
            widp popup = wid_tooltip(popup_str, 0.5f, 0.5f, med_font);
            wid_destroy_in(popup, ONESEC);
            myfree(popup_str);
        }

        LOG("Saved: %s", dir_and_file);
    }
}

static void wid_editor_go_back (void)
{
    wid_destroy(&wid_editor_background);
    wid_destroy(&wid_editor_window);

    wid_map("Choose epic level", 0, 0);
}

static void wid_editor_save_close_dialog (widp w)
{
    widp top = wid_get_top_parent(w);
    wid_destroy(&top);
    wid_editor_save_popup = 0;
}

static void wid_editor_save_level (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    level_pos_t level_pos = ctx->level_pos;
    char *tmp = dynprintf("%s%d.%d", LEVELS_PATH, level_pos.y, level_pos.x);
    CON("Save editor level %s", tmp);
    wid_editor_save(tmp, false /* is_test_level */);
    myfree(tmp);

    ctx->save_needed = false;
}

static void wid_editor_save_callback_yes (widp w)
{
    wid_editor_save_level();

    wid_editor_save_close_dialog(w);

    wid_editor_go_back();
}

static void wid_editor_save_callback_no (widp w)
{
    wid_editor_save_close_dialog(w);

    wid_editor_go_back();
}

static void wid_editor_save_callback_cancel (widp w)
{
    wid_editor_save_close_dialog(w);
}

static void wid_editor_save_ask (void)
{
    if (wid_editor_save_popup) {
        return;
    }

    wid_editor_save_popup = 
        wid_menu(0,
                vvlarge_font,
                large_font,
                0, // on_update
                0.5, /* x */
                0.5, /* y */
                1, /* columns */
                1, /* focus */
                4, /* items */

                (int) 0, "save level?", (void*) 0,

                (int) 'y', "Yes", wid_editor_save_callback_yes,

                (int) 'n', "No",  wid_editor_save_callback_no,

                (int) 'c', "Cancel",  wid_editor_save_callback_cancel);
}

static void wid_editor_hide (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    if (ctx->save_needed) {
        wid_editor_save_ask();
    } else {
        wid_editor_go_back();
    }
}

void wid_editor (level_pos_t level_pos)
{
    /*
     * Create a context to hold button info so we can update it when the focus 
     * changes
     */
    wid_editor_ctx *ctx = myzalloc(sizeof(*ctx), "wid editor");
    wid_editor_window_ctx = ctx;

    ctx->focus_x = -1;
    ctx->focus_y = -1;
    ctx->level_pos = level_pos;

    ctx->col_name = "white";
    ctx->col = color_find(ctx->col_name);

    widp window;
    ctx->w = wid_editor_window = window = wid_new_window("wid editor");
    wid_set_client_context(window, ctx);
    ctx->is_new = true;
    ctx->layer_mode = WID_EDITOR_MODE2_FILTER_ALL;

    /*
     * Main window
     */
    {
        fpoint tl = { 0.0, 0.0};
        fpoint br = { 1.0, 1.0};

        wid_set_tl_br_pct(window, tl, br);

        color c = BLACK;
        c.a = 0;
        wid_set_color(window, WID_COLOR_BG, c);

        wid_set_on_tick(window, wid_editor_tick);
        wid_set_on_destroy(window, wid_editor_destroy);
        wid_set_client_context(window, ctx);
    }

    {
        /*
         * Create the buttons
         */
        int x, y;

        for (x = 0; x < WID_EDITOR_MENU_CELLS_ACROSS; x++) {
        for (y = 0; y < WID_EDITOR_MENU_CELLS_DOWN; y++) {
            widp b = wid_new_square_button(wid_editor_window, "wid tile");
            ctx->tile[x][y].button = b;
            ctx->tile[x][y].x = x;
            ctx->tile[x][y].y = y;

            wid_set_on_key_down(b, wid_editor_key_down);
            wid_set_on_joy_down(b, wid_editor_joy_button);
            wid_set_on_mouse_over_begin(b, wid_editor_mouse_over);
            wid_set_on_mouse_motion(b, wid_editor_mouse_motion);
            wid_set_on_mouse_down(b, wid_editor_mouse_down);
            wid_set_on_display(b, wid_editor_button_display);

            wid_set_client_context(b, ctx);
            int focus = (y << 8) | x;
            wid_set_client_context2(b, (void*) (uintptr_t) focus);

            if (x == WID_EDITOR_MENU_CELLS_ACROSS - 1) {
                switch (y) {
                case WID_EDITOR_MODE2_TITLE:
                    wid_set_text(b, "Title");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Set level title",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE2_FILTER_FLOOR:
                    wid_set_text(b, "Floor");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Floor layer", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE2_FILTER_OBJ:
                    wid_set_text(b, "Obj");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Obj layer", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE2_FILTER_WALL:
                    wid_set_text(b, "Wall");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Wall layer", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE2_FILTER_ACTIONS:
                    wid_set_text(b, "Actions");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Actions layer", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE2_FILTER_ALL:
                    wid_set_text(b, "All");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "All layers", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE2_96x96:
                    wid_set_text(b, "96x96");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Create empty level 96x96 size",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE2_64x64:
                    wid_set_text(b, "64x64");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Create empty level 64x64 size",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE2_32x32:
                    wid_set_text(b, "32x32");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Create empty level 32x32 size",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE2_CENTER:
                    wid_set_text(b, "CENTER");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Center level",
                                        vsmall_font);
                    }
                    break;
                }
            }

            if (y == WID_EDITOR_MENU_CELLS_DOWN - 1) {
                switch (x) {
                case WID_EDITOR_MODE_DRAW:
                    wid_set_text(b, "Draw");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "D - draw tiles", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_PAINT:
                    wid_set_text(b, "Paint");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "P - recolor tiles", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_LINE:
                    wid_set_text(b, "Line");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "L - draw lines of tiles", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_FILL:
                    wid_set_text(b, "Fill");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "f - flood fill", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_DEL:
                    wid_set_text(b, "Del");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "x - remove tiles", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_UNDO:
                    wid_set_text(b, "Undo");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "u - shortcut", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_REDO:
                    wid_set_text(b, "Redo");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "e - shortcut", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_SAVE:
                    wid_set_text(b, "Save");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "s - shortcut", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_TOGGLE:
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "TAB - switch between tile and map mode", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_COLOR:
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Choose color to paint tiles", vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_YANK:
                    wid_set_text(b, "Yank");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "y - picks up a tile and its color",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_EDIT:
                    wid_set_text(b, "Edit");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "edit thing attributes",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_SQUARE:
                    wid_set_text(b, "Rect");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "r - draw a rectangle",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_CIRCLE:
                    wid_set_text(b, "Circ");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "draw a circle",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_OCTAGON:
                    wid_set_text(b, "Oct");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "draw an octagon",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_CUT:
                    wid_set_text(b, "Cut");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Remove rectangular section",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_COPY:
                    wid_set_text(b, "Copy");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "c - copy an area into the buffer",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_PASTE:
                    wid_set_text(b, "Paste");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "v - paste a previously cut area",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_TEST:
                    wid_set_text(b, "Test");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "t - shortcut, Test out level",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_RANDOM:
                    wid_set_text(b, "Random");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Create random level",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_STYLE:
                    wid_set_text(b, "Style");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Retheme walls and floors randomly",
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_VFLIP:
                    wid_set_text(b, "Vflip");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Vertical flip.", 
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_HFLIP:
                    wid_set_text(b, "Hflip");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Horzizontal flip.", 
                                        vsmall_font);
                    }
                    break;
                case WID_EDITOR_MODE_ROTATE:
                    wid_set_text(b, "Rot");
                    if (!sdl_joy_axes) {
                        wid_set_tooltip(b, "Rotate level.", 
                                        vsmall_font);
                    }
                    break;
                }
            }
        }
        }
    }

    ctx->focus_x = -1;
    ctx->focus_y = -1;
    ctx->map_x = -1;
    ctx->map_y = -1;

    /*
     * Load all tiles
     */
    wid_editor_load_tiles();

    /*
     * Load the level
     */
    wid_editor_load_map(level_pos);

    map_editor_fixup();

    /*
     * If no position was loaded from the level, use a default unless this is 
     * an edit of an old level.
     */
    if ((level_pos.x == 66) && (level_pos.y == 66)) {
        /*
         * Returning from testing a level? Welcome back...
         */
        ctx->level_pos = saved_level_pos;
        ctx->focus_x = saved_focus_x;
        ctx->focus_y = saved_focus_y;
        ctx->map_x = saved_map_x;
        ctx->map_y = saved_map_y;
    } else if (!memcmp(&saved_level_pos, &ctx->level_pos, sizeof(level_pos_t))) {
        /*
         * Reloading the same level?
         */
        if (saved_focus_x != -1) {
            ctx->focus_x = saved_focus_x;
            ctx->focus_y = saved_focus_y;
            ctx->map_x = saved_map_x;
            ctx->map_y = saved_map_y;
        }
    }

    if (ctx->focus_x == -1) {
        ctx->focus_x = WID_EDITOR_MENU_CELLS_ACROSS / 2;
    }

    if (ctx->focus_y == -1) {
        ctx->focus_y = WID_EDITOR_MENU_CELLS_DOWN / 2;
    }

    if (ctx->map_x == -1) {
        ctx->map_x = MAP_WIDTH / 2;
    }

    if (ctx->map_y == -1) {
        ctx->map_y = MAP_HEIGHT / 2;
    }

    ctx->undo_at = -1;
    wid_editor_undo_save();

    /*
     * Repair the context so it is not pointing at the last level loaded.
     */
    wid_set_client_context(window, ctx);

    wid_editor_update_buttons();
    wid_update(window);
    wid_editor_update_buttons();
    wid_update(window);
    wid_editor_bg_create();

    ctx->created = time_get_time_ms();
    wid_editor_tile_mode_set(false);
    ctx->tile_pool = WID_TILE_MODE_WALLS;

    wid_editor_set_mode(WID_EDITOR_MODE_DRAW);

    if (WID_TILE_MODE_LAST > WID_TILE_POOL_MAX) {
        ERR("bug, you need to resize WID_TILE_POOL_MAX");
    }
}
