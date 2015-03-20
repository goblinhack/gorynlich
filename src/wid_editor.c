/*
 * Copyright (C) 2011 Neil McGill
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
#include "wid_editor.h"
#include "time_util.h"
#include "timer.h"
#include "level.h"
#include "math_util.h"
#include "thing_template.h"
#include "thing_tile.h"
#include "wid_editor.h"
#include "wid_tooltip.h"
#include "wid_intro.h"
#include "tile.h"
#include "marshal.h"
#include "wid_map.h"
#include "wid_menu.h"
#include "string_util.h"
#include "bits.h"

static void wid_editor_hide(void);
static void wid_editor_tile_right_button_pressed(int x, int y);
static void wid_editor_set_focus(wid_editor_ctx *ctx, int focusx, int focusy);
static void wid_editor_map_scroll(int dx, int dy);
static void wid_editor_undo_save(void);
static void wid_editor_save_level(void);
static widp wid_editor_save_popup; // edit wid_editor_tick if you add more

static widp wid_editor_window;
static widp wid_editor_background;
static wid_editor_ctx *wid_editor_window_ctx;

static int saved_focus_x = -1;
static int saved_focus_y = -1;
static tpp wid_editor_chosen_tile;

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

/*
 * Replace or place a tile.
 */
widp wid_editor_replace_template (widp w,
                                  double x,
                                  double y,
                                  thingp t,
                                  tpp tp,
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
        (z > MAP_DEPTH)) {
        ERR("overflow in reading position "
            "(%f,%f,%d) -> (%d,%d) in level %u.%u, "
            "map bounds (%d,%d) -> (%d,%d)", 
            x, y, z, ix, iy, ctx->loading_x, ctx->loading_y,
            0, 0, MAP_DEPTH, MAP_HEIGHT);
        return (0);
    }

    ctx->map.tile[ix][iy][z].tp = tp;

    return (0);
}

static tpp map_find_wall_at (int x, int y)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    tpp tp = ctx->map.tile[x][y][MAP_DEPTH_WALL].tp;
    if (tp && tp_is_wall(tp)) {
        return (tp);
    }

    return (0);
}

static tpp map_find_pipe_at (int x, int y)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    tpp tp = ctx->map.tile[x][y][MAP_DEPTH_WALL].tp;
    if (tp && tp_is_pipe(tp)) {
        return (tp);
    }

    return (0);
}

static tpp map_find_door_at (int x, int y)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    tpp tp = ctx->map.tile[x][y][MAP_DEPTH_WALL].tp;
    if (tp && tp_is_door(tp)) {
        return (tp);
    }

    return (0);
}

static void map_fixup (void)
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

            if (map_find_wall_at(x, y)) {
                ok = true;
            } else if (map_find_pipe_at(x, y)) {
                ok = true;
            } else if (map_find_door_at(x, y)) {
                ok = true;
            }

            if (!ok) {
                continue;
            }

            memset(nbrs, 0, sizeof(nbrs));

            for (dx = -1; dx <= 1; dx++) {
                for (dy = -1; dy <= 1; dy++) {

                    if (map_find_wall_at(x, y)) {
                        nbrs[dx + 1][dy + 1] = map_find_wall_at(x + dx, y + dy);
                    }

                    if (map_find_pipe_at(x, y)) {
                        nbrs[dx + 1][dy + 1] = map_find_pipe_at(x + dx, y + dy);
                    }

                    if (map_find_door_at(x, y)) {
                        nbrs[dx + 1][dy + 1] = map_find_door_at(x + dx, y + dy);
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

#define BLOCK(a,b,c,d,e,f,g,h,i, _index_)                               \
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

            BLOCK(1,1,1,1,1,1,1,1,1,IS_JOIN_BLOCK)
            BLOCK(0,0,0,0,1,0,0,0,0,IS_JOIN_NODE)
            BLOCK(0,0,0,0,1,1,0,0,0,IS_JOIN_LEFT)
            BLOCK(0,0,0,0,1,0,0,1,0,IS_JOIN_TOP)
            BLOCK(0,0,0,1,1,0,0,0,0,IS_JOIN_RIGHT)
            BLOCK(0,1,0,0,1,0,0,0,0,IS_JOIN_BOT)
            BLOCK(0,0,0,1,1,1,0,0,0,IS_JOIN_HORIZ)
            BLOCK(0,1,0,0,1,0,0,1,0,IS_JOIN_VERT)
            BLOCK(0,0,0,0,1,1,0,1,1,IS_JOIN_TL2)
            BLOCK(0,1,1,0,1,1,0,0,0,IS_JOIN_BL2)
            BLOCK(1,1,0,1,1,0,0,0,0,IS_JOIN_BR2)
            BLOCK(0,0,0,1,1,0,1,1,0,IS_JOIN_TR2)
            BLOCK(0,0,0,0,1,1,0,1,0,IS_JOIN_TL)
            BLOCK(0,1,0,0,1,1,0,0,0,IS_JOIN_BL)
            BLOCK(0,1,0,1,1,0,0,0,0,IS_JOIN_BR)
            BLOCK(0,0,0,1,1,0,0,1,0,IS_JOIN_TR)
            BLOCK(1,1,0,1,1,0,1,1,0,IS_JOIN_T90_3)
            BLOCK(1,1,1,1,1,1,0,0,0,IS_JOIN_T180_3)
            BLOCK(0,1,1,0,1,1,0,1,1,IS_JOIN_T270_3)
            BLOCK(0,0,0,1,1,1,1,1,1,IS_JOIN_T_3)
            BLOCK(0,1,0,0,1,1,0,1,0,IS_JOIN_T270)
            BLOCK(0,1,0,1,1,1,0,0,0,IS_JOIN_T180)
            BLOCK(0,1,0,1,1,0,0,1,0,IS_JOIN_T90)
            BLOCK(0,0,0,1,1,1,0,1,0,IS_JOIN_T)
            BLOCK(0,1,1,0,1,1,0,1,0,IS_JOIN_T270_2)
            BLOCK(1,1,0,1,1,1,0,0,0,IS_JOIN_T180_2)
            BLOCK(0,1,0,1,1,0,1,1,0,IS_JOIN_T90_2)
            BLOCK(0,0,0,1,1,1,0,1,1,IS_JOIN_T_2)
            BLOCK(0,1,0,0,1,1,0,1,1,IS_JOIN_T270_1)
            BLOCK(0,1,1,1,1,1,0,0,0,IS_JOIN_T180_1)
            BLOCK(1,1,0,1,1,0,0,1,0,IS_JOIN_T90_1)
            BLOCK(0,0,0,1,1,1,1,1,0,IS_JOIN_T_1)
            BLOCK(0,1,0,1,1,1,0,1,0,IS_JOIN_X)
            BLOCK(0,1,0,1,1,1,0,1,1,IS_JOIN_X1)
            BLOCK(0,1,1,1,1,1,0,1,0,IS_JOIN_X1_270)
            BLOCK(1,1,0,1,1,1,0,1,0,IS_JOIN_X1_180)
            BLOCK(0,1,0,1,1,1,1,1,0,IS_JOIN_X1_90)
            BLOCK(0,1,0,1,1,1,1,1,1,IS_JOIN_X2)
            BLOCK(0,1,1,1,1,1,0,1,1,IS_JOIN_X2_270)
            BLOCK(1,1,1,1,1,1,0,1,0,IS_JOIN_X2_180)
            BLOCK(1,1,0,1,1,1,1,1,0,IS_JOIN_X2_90)
            BLOCK(0,1,1,1,1,1,1,1,0,IS_JOIN_X3)
            BLOCK(1,1,0,1,1,1,0,1,1,IS_JOIN_X3_180)
            BLOCK(0,1,1,1,1,1,1,1,1,IS_JOIN_X4)
            BLOCK(1,1,1,1,1,1,0,1,1,IS_JOIN_X4_270)
            BLOCK(1,1,1,1,1,1,1,1,0,IS_JOIN_X4_180)
            BLOCK(1,1,0,1,1,1,1,1,1,IS_JOIN_X4_90)

            /*
             * Single node doors need to join onto walls.
             */
            if (index == IS_JOIN_NODE) {
                if (map_find_door_at(x, y)) {
                    if ( map_find_wall_at(x - 1, y) &&
                         map_find_wall_at(x + 1, y) &&
                        !map_find_wall_at(x, y - 1) &&
                        !map_find_wall_at(x, y + 1)) {
                        index = IS_JOIN_HORIZ2;
                    }
                }
            }

            if (index == IS_JOIN_NODE) {
                if (map_find_door_at(x, y)) {
                    if (!map_find_wall_at(x - 1, y) &&
                        !map_find_wall_at(x + 1, y) &&
                         map_find_wall_at(x, y - 1) &&
                         map_find_wall_at(x, y + 1)) {
                        index = IS_JOIN_VERT2;
                    }
                }
            }

            if (index == -1) {
                DIE("%u%u%u %u%u%u %u%u%u not handled",
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
                        DIE("no joinable tile for %s", tp_name(t));
                    }
                }
            }

            if (!tile) {
                DIE("no tile for %s", tp_name(t));
            }

            const char *tilename = thing_tile_name(thing_tile);

            if (!tilename) {
                DIE("no tilename for %s", tp_name(e));
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
        case WID_EDITOR_MODE_LINE:
        case WID_EDITOR_MODE_FILL:
        case WID_EDITOR_MODE_DEL:
        case WID_EDITOR_MODE_UNDO:
        case WID_EDITOR_MODE_REDO:
        case WID_EDITOR_MODE_NUKE:
            break;

        case WID_EDITOR_MODE_TOGGLE:
            if (wid_editor_chosen_tile) {
                wid_set_thing_template(b, wid_editor_chosen_tile);
                wid_set_color(b, WID_COLOR_TL, RED);
                wid_set_color(b, WID_COLOR_BR, RED);
            }
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

        font = small_font;

        int is_a_tile = 0;
        int is_a_map_tile = 0;

        int mx = x + ctx->map_x;
        int my = y + ctx->map_y;

        if (!ctx->tile_mode) {
            int z;

            if ((x < WID_EDITOR_MENU_MAP_ACROSS) && 
                (y < WID_EDITOR_MENU_MAP_DOWN)) {
                for (z = 0; z < MAP_DEPTH; z++) {
                    if (ctx->map.tile[mx][my][z].tp) {
                        is_a_map_tile = 1;
                        break;
                    }
                }
            }
        } else {
            if (ctx->tile[x][y].tile_tp) {
                is_a_tile = 1;
            }
        }

        double zoom = 0.002;
        if ((x == ctx->focusx) && (y == ctx->focusy)) {
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
        } else if ((x == ctx->focusx) && (y == ctx->focusy)) {
            color c = RED;
            c.a = 100;
            wid_set_color(b, WID_COLOR_BG, c);
        } else if (is_a_tile) {
            tpp tp = ctx->tile[x][y].tile_tp;

            if (tp == wid_editor_chosen_tile) {
                wid_set_color(b, WID_COLOR_TL, RED);
                wid_set_color(b, WID_COLOR_BR, RED);
            }

            color c = BLACK;
            c.a = 0;
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

    wid_editor_update_edit_mode_buttons();
    wid_update(wid_editor_window);
}

static void wid_editor_button_display (widp w, fpoint tl, fpoint br)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    int xy = (typeof(xy)) (uintptr_t) wid_get_client_context2(w);
    int x = (xy & 0xff);
    int y = (xy & 0xff00) >> 8;

    if (ctx->tile_mode) {
        tpp tp = ctx->tile[x][y].tile_tp;
        if (!tp) {
            return;
        }

        thing_tilep thing_tile;
        tree_rootp tiles;

        tiles = tp_get_tiles(tp);
        if (!tiles) {
            return;
        }

        thing_tile = thing_tile_first(tiles);
        if (!thing_tile) {
            return;
        }

        const char *tilename = thing_tile_name(thing_tile);
        if (!tilename) {
            ERR("cannot find tile %s", tilename);
            return;
        }

        tilep tile = tile_find(tilename);
        if (!tile) {
            ERR("cannot find tilep for tile %s", tilename);
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
        DIE("overflow on map coords (%d, %d)", x, y);
    }

    double width = br.x - tl.x;
    double height = br.y - tl.y;

    width *= 1.5;
    height *= 1.5;

    br.x = tl.x + width;
    tl.y = br.y - height;

    blit_init();

    int z;
    for (z = 0; z < MAP_DEPTH; z++) {
        tpp tp = ctx->map.tile[x][y][z].tp;
        if (!tp) {
            continue;
        }

        if (z == MAP_DEPTH_WALL) {
            tilep tile = ctx->map.map_tile[x][y];
            if (tile) {
                glcolor(WHITE);
                tile_blit_fat(tile, 0, tl, br);
                continue;
            }
        }

        thing_tilep thing_tile;
        tree_rootp tiles;

        tiles = tp_get_tiles(tp);
        if (!tiles) {
            continue;
        }

        thing_tile = thing_tile_first(tiles);
        if (!thing_tile) {
            continue;
        }

        const char *tilename = thing_tile_name(thing_tile);
        if (!tilename) {
            ERR("cannot find tile %s", tilename);
            continue;
        }

        tilep tile = tile_find(tilename);
        if (!tile) {
            ERR("cannot find tilep for tile %s", tilename);
        }

        glcolor(WHITE);
        tile_blit_fat(tile, 0, tl, br);
    }

    blit_flush();
}

static void wid_editor_focus_right (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    if (!ctx) {
        return;
    }

    ctx->focusx++;
    if (ctx->focusx > WID_EDITOR_MENU_CELLS_ACROSS - 1) {
        ctx->focusx = WID_EDITOR_MENU_CELLS_ACROSS - 1;
        wid_editor_map_scroll(1, 0);
    }
}

static void wid_editor_focus_left (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    if (!ctx) {
        return;
    }

    ctx->focusx--;
    if (ctx->focusx < 0) {
        ctx->focusx = 0;
        wid_editor_map_scroll(-1, 0);
    }
}

static void wid_editor_focus_down (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    if (!ctx) {
        return;
    }

    ctx->focusy++;
    if (ctx->focusy > WID_EDITOR_MENU_CELLS_DOWN - 1) {
        ctx->focusy = WID_EDITOR_MENU_CELLS_DOWN - 1;
        wid_editor_map_scroll(0, 1);
    }
}

static void wid_editor_focus_up (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    if (!ctx) {
        return;
    }

    ctx->focusy--;
    if (ctx->focusy < 0) {
        ctx->focusy = 0;
        wid_editor_map_scroll(0, -1);
    }
}

static void wid_editor_set_focus (wid_editor_ctx *ctx, int focusx, int focusy)
{
    ctx->focusx = focusx;
    ctx->focusy = focusy;

    wid_editor_update_buttons();
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

static void wid_editor_map_thing_replace (int x, int y)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    if ((x < 0) || (y < 0) ||
        (x >= MAP_WIDTH) ||
        (y >= MAP_HEIGHT)) {
        DIE("bad map coord %d,%d", x, y);
    }

    if (!wid_editor_chosen_tile) {
        return;
    }

    int z = tp_get_z_depth(wid_editor_chosen_tile);

    ctx->map.tile[x][y][z].tp = wid_editor_chosen_tile;
}

static tpp wid_editor_map_thing_get (int x, int y)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    if ((x < 0) || (y < 0) ||
        (x >= MAP_WIDTH) ||
        (y >= MAP_HEIGHT)) {
        return (0);
    }

    int z;
    for (z = MAP_DEPTH - 1; z > 0; z--) {
        tpp tp = ctx->map.tile[x][y][z].tp;
        if (tp) {
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
        DIE("bad map coord %d,%d", x, y);
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
        wid_editor_map_thing_replace((int)x, (int)y);
    } else if (flag == 1) {
        wid_editor_map_thing_replace((int)y, (int)x);
    } else if (flag == 2) {
        wid_editor_map_thing_replace((int)y, (int)-x);
    } else if (flag == 3) {
        wid_editor_map_thing_replace((int)x, (int)-y);
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
            wid_editor_map_thing_replace((int)x, (int)y);
        } else if (flag == 1) {
            wid_editor_map_thing_replace((int)y, (int)x);
        } else if (flag == 2) {
            wid_editor_map_thing_replace((int)y, (int)-x);
        } else if (flag == 3) {
            wid_editor_map_thing_replace((int)x, (int)-y);
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

    map_fixup();

    wid_editor_undo_save();
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

static void wid_editor_draw_square (int x0, int y0, int x1, int y1)
{
    int x, y;

    for (x = x0; x <= x1; x++) {
        wid_editor_map_thing_replace(x, y0);
        wid_editor_map_thing_replace(x, y1);
    }

    for (y = y0; y <= y1; y++) {
        wid_editor_map_thing_replace(x0, y);
        wid_editor_map_thing_replace(x0, y);
    }

    map_fixup();

    wid_editor_undo_save();
}

static void wid_editor_paste (int mx, int my)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    int x0 = ctx->cut_start_x;
    int x1 = ctx->cut_end_x;

    int y0 = ctx->cut_start_y;
    int y1 = ctx->cut_end_y;

    int x, y, z;

    for (x = x0; x <= x1; x++) {
        for (y = y0; y <= y1; y++) {
            for (z = 0; z < MAP_DEPTH; z++) {
                tpp tp = ctx->map_cut.tile[x][y][z].tp;
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

                ctx->map.tile[px][py][z].tp = tp;
            }
        }
    }

    map_fixup();

    wid_editor_undo_save();
}

static void wid_editor_cut (int mx, int my)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    int x0 = ctx->cut_start_x;
    int x1 = ctx->cut_end_x;

    int y0 = ctx->cut_start_y;
    int y1 = ctx->cut_end_y;

    int x, y, z;

    for (x = x0; x <= x1; x++) {
        for (y = y0; y <= y1; y++) {
            for (z = 0; z < MAP_DEPTH; z++) {

                int px = mx + (x - x0);
                int py = my + (y - y0);

                if ((px < 0) || (py < 0) ||
                    (px >= MAP_WIDTH) ||
                    (py >= MAP_HEIGHT)) {
                    continue;
                }

                ctx->map.tile[px][py][z].tp = 0;
            }
        }
    }

    map_fixup();

    wid_editor_undo_save();
}

static void wid_editor_draw_highlight_square (int x0, int y0, int x1, int y1)
{
    int x, y;

    for (x = x0; x <= x1; x++) {
        wid_editor_map_highlight_replace(x, y0);
        wid_editor_map_highlight_replace(x, y1);
    }

    for (y = y0; y <= y1; y++) {
        wid_editor_map_highlight_replace(x0, y);
        wid_editor_map_highlight_replace(x0, y);
    }
}

static void wid_editor_draw_highlight_cut (int x0, int y0, int x1, int y1)
{
    int x, y;

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
        ctx->undo_at = 0;
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
        ctx->undo_at = WID_EDITOR_UNDO - 1;
    }

    if (!ctx->valid_undo[ctx->undo_at]) {
        ctx->undo_at = old;
        return;
    }

    memcpy(&ctx->map, &ctx->map_undo[ctx->undo_at], sizeof(ctx->map));

    map_fixup();
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

    map_fixup();
}

static void wid_editor_nuke (void)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    memset(&ctx->map, 0, sizeof(ctx->map));

    map_fixup();

    wid_editor_undo_save();
}

static void wid_editor_tile_fill_ (int x, int y)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;

    if (!wid_editor_chosen_tile) {
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
    switch (tp_get_z_depth(wid_editor_chosen_tile)) {
        case MAP_DEPTH_EDITOR: 
            min_z = 0; 
            break;
        case MAP_DEPTH_FLOOR: 
            min_z = MAP_DEPTH_FLOOR; 
            break;
        case MAP_DEPTH_WALL: 
            min_z = MAP_DEPTH_WALL; 
            break;
        case MAP_DEPTH_MONST: 
            min_z = MAP_DEPTH_WALL; 
            break;
        case MAP_DEPTH_PLAYER: 
            min_z = MAP_DEPTH_WALL; 
            break;
        default:
            min_z = 0;
            break;
    }

    for (z = min_z; z < MAP_DEPTH; z++) {
        if (ctx->map.tile[x][y][z].tp) {
            return;
        }
    }

    z = tp_get_z_depth(wid_editor_chosen_tile);
    ctx->map.tile[x][y][z].tp = wid_editor_chosen_tile;

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

    map_fixup();

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
        wid_editor_tile_mode_toggle();

        tpp tp = ctx->tile[x][y].tile_tp;
        if (tp) {
            wid_editor_chosen_tile = tp;
            return;
        }
    } else {
        if (wid_editor_chosen_tile) {
            if ((x < WID_EDITOR_MENU_MAP_ACROSS) && 
                (y < WID_EDITOR_MENU_MAP_DOWN)) {
                switch (ctx->edit_mode) {
                case WID_EDITOR_MODE_DRAW:
                    wid_editor_map_thing_replace(mx, my);

                    map_fixup();

                    wid_editor_undo_save();
                    break;

                case WID_EDITOR_MODE_LINE:
                    if (!ctx->got_line_start) {
                        ctx->got_line_start = true;
                        ctx->line_start_x = mx;
                        ctx->line_start_y = my;
                    } else {
                        wid_editor_draw_line(ctx->line_start_x, 
                                            ctx->line_start_y, mx, my);
                        ctx->got_line_start = false;
                    }
                    break;

                case WID_EDITOR_MODE_SQUARE:
                    if (!ctx->got_square_start) {
                        ctx->got_square_start = true;
                        ctx->square_start_x = mx;
                        ctx->square_start_y = my;
                    } else {
                        wid_editor_draw_square(ctx->square_start_x,
                                               ctx->square_start_y, mx, my);
                        ctx->got_square_start = false;
                    }
                    break;

                case WID_EDITOR_MODE_CUT:
                    if (!ctx->got_cut_start) {
                        ctx->got_cut_start = true;
                        ctx->cut_start_x = mx;
                        ctx->cut_start_y = my;
                    } else {
                        memcpy(&ctx->map_cut, &ctx->map, sizeof(ctx->map));
                        wid_editor_cut(mx, my);
                        ctx->got_cut_start = false;
                        ctx->got_cut_end = true;
                        ctx->cut_end_x = mx;
                        ctx->cut_end_y = my;
                    }
                    break;

                case WID_EDITOR_MODE_COPY:
                    if (!ctx->got_cut_start) {
                        ctx->got_cut_start = true;
                        ctx->cut_start_x = mx;
                        ctx->cut_start_y = my;
                    } else {
                        memcpy(&ctx->map_cut, &ctx->map, sizeof(ctx->map));
                        ctx->got_cut_start = false;
                        ctx->got_cut_end = true;
                        ctx->cut_end_x = mx;
                        ctx->cut_end_y = my;
                    }
                    break;

                case WID_EDITOR_MODE_PASTE:
                    wid_editor_paste(mx, my);
                    break;

                case WID_EDITOR_MODE_YANK:
                    {
                        tpp tp = wid_editor_map_thing_get(mx, my);
                        if (tp) {
                            wid_editor_chosen_tile = tp;

                            /*
                             * Fake a cut so a 'p' can put this back.
                             */
                            memcpy(&ctx->map_cut, &ctx->map, sizeof(ctx->map));
                            ctx->cut_start_x = mx;
                            ctx->cut_end_x = mx;
                            ctx->cut_start_y = my;
                            ctx->cut_end_y = my;
                        }
                    }
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

    if (y == WID_EDITOR_MENU_CELLS_DOWN - 1) {
        if (x < WID_EDITOR_MODE_MAX) {
            switch (x) {
            case WID_EDITOR_MODE_DRAW:
            case WID_EDITOR_MODE_LINE:
            case WID_EDITOR_MODE_DEL:
            case WID_EDITOR_MODE_FILL:
            case WID_EDITOR_MODE_CUT:
            case WID_EDITOR_MODE_PASTE:
            case WID_EDITOR_MODE_YANK:
            case WID_EDITOR_MODE_SQUARE:
                wid_editor_set_mode(x);
                break;

            case WID_EDITOR_MODE_UNDO:
                ctx->tile_mode = false;
                wid_editor_undo();
                break;

            case WID_EDITOR_MODE_REDO:
                ctx->tile_mode = false;
                wid_editor_redo();
                break;

            case WID_EDITOR_MODE_NUKE:
                ctx->tile_mode = false;
                wid_editor_nuke();
                break;

            case WID_EDITOR_MODE_SAVE:
                wid_editor_save_level();
                break;

            case WID_EDITOR_MODE_TOGGLE:
                wid_editor_tile_mode_toggle();
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
            for (z = MAP_DEPTH - 1; z > 0; z--) {
                tpp tp = ctx->map.tile[mx][my][z].tp;
                if (tp) {
                    /*
                     * Fake a cut so a 'p' can put this back.
                     */
                    memcpy(&ctx->map_cut, &ctx->map, sizeof(ctx->map));
                    ctx->cut_start_x = mx;
                    ctx->cut_end_x = mx;
                    ctx->cut_start_y = my;
                    ctx->cut_end_y = my;

                    ctx->map.tile[mx][my][z].tp = 0;

                    map_fixup();

                    wid_editor_undo_save();
                    return;
                }
            }
        }
    }
}

static uint8_t wid_editor_mouse_down (widp w,
                                      int mx, int my,
                                      uint button)
{
    int xy = (typeof(xy)) (uintptr_t) wid_get_client_context2(w);
    int x = (xy & 0xff);
    int y = (xy & 0xff00) >> 8;

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
    int x = ctx->focusx;
    int y = ctx->focusy;

    int mx = -1;
    int my = -1;

    if (!ctx->tile_mode) {
        mx = x + ctx->map_x;
        my = y + ctx->map_y;
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
        }
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
            ctx->tile_mode = false;
            wid_editor_tile_left_button_pressed(mx, my);
            return (false);

        case SDLK_BACKSPACE:
            ctx->tile_mode = false;
            wid_editor_tile_right_button_pressed(mx, my);
            return (true); 

        case 'l':
            ctx->tile_mode = false;
            wid_editor_set_mode(WID_EDITOR_MODE_LINE);
            wid_editor_tile_left_button_pressed(mx, my);
            return (true);

        case 'r':
            ctx->tile_mode = false;
            wid_editor_set_mode(WID_EDITOR_MODE_SQUARE);
            wid_editor_tile_left_button_pressed(mx, my);
            return (true);

        case 's':
            ctx->tile_mode = false;
            wid_editor_save_level();
            return (true);

        case 'd':
            ctx->tile_mode = false;
            wid_editor_set_mode(WID_EDITOR_MODE_DRAW);
            return (true);

        case 'y':
            ctx->tile_mode = false;
            wid_editor_set_mode(WID_EDITOR_MODE_YANK);
            wid_editor_tile_left_button_pressed(mx, my);
            return (true);

        case 't':
            ctx->tile_mode = false;
            wid_editor_set_mode(WID_EDITOR_MODE_CUT);
            wid_editor_tile_left_button_pressed(mx, my);
            return (true);

        case 'c':
            ctx->tile_mode = false;
            wid_editor_set_mode(WID_EDITOR_MODE_COPY);
            wid_editor_tile_left_button_pressed(mx, my);
            return (true);

        case 'p':
            ctx->tile_mode = false;
            wid_editor_set_mode(WID_EDITOR_MODE_PASTE);
            wid_editor_tile_left_button_pressed(mx, my);
            return (true);

        case 'f':
            ctx->tile_mode = false;
            wid_editor_set_mode(WID_EDITOR_MODE_FILL);
            wid_editor_tile_left_button_pressed(mx, my);
            return (true);

        case 'u':
            ctx->tile_mode = false;
            wid_editor_undo();
            return (true);

        case 'e':
            ctx->tile_mode = false;
            wid_editor_redo();
            return (true);

        case 'x':
            ctx->tile_mode = false;
            wid_editor_set_mode(WID_EDITOR_MODE_DEL);
            wid_editor_tile_left_button_pressed(mx, my);
            return (true);

        case 'z':
            ctx->tile_mode = false;
            wid_editor_nuke();
            return (true);

        case SDLK_ESCAPE:
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

    int xy = (typeof(xy)) (uintptr_t) wid_get_client_context2(w);
    int x = ctx->focusx;
    int y = ctx->focusy;

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
    wid_editor_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    if (!relx && !rely && !wheelx && !wheely) {
        return (false);
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
    int focusx = (focus & 0xff);
    int focusy = (focus & 0xff00) >> 8;

    wid_editor_set_focus(ctx, focusx, focusy);
}

static void wid_editor_destroy (widp w)
{
    wid_editor_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    wid_set_client_context(w, 0);

    if (wid_editor_background) {
        wid_destroy(&wid_editor_background);
    }

    int x, y, z;

    for (x = 0; x < WID_EDITOR_MENU_CELLS_ACROSS; x++) {
        for (y = 0; y < WID_EDITOR_MENU_CELLS_DOWN; y++) {

            tpp tp = ctx->tile[x][y].tile_tp;
            if (!tp) {
                continue;
            }

            ctx->tile[x][y].tile_tp = 0;

            for (z = 0; z < MAP_DEPTH; z++) {
                ctx->map.tile[x][y][z].tp = 0;
            }
        }
    }

    level_destroy(&ctx->level, false /* keep players */);

    saved_focus_x = ctx->focusx;
    saved_focus_y = ctx->focusy;

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

static int tile_x;
static int tile_y;

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

    ctx->tile[tile_x][tile_y].tile_tp = tp;

    /*
     * Start out with something.
     */
    if (!wid_editor_chosen_tile) {
        wid_editor_chosen_tile = tp;
    }

    tile_x++;

    if (tile_x >= WID_EDITOR_MENU_TILES_ACROSS) {
        tile_x = 0;
        tile_y++;
    }

    if (tile_y >= WID_EDITOR_MENU_TILES_DOWN) {
        tile_y++;
        DIE("too many tiles; implement a scrollbar neil");
    }

    return (true);
}

static void wid_editor_load_tiles (void)
{
    tile_x = 0;
    tile_y = 1;

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
        ERR("Failed to load level %d.%d", level_pos.y, level_pos.x);
        return;
    }

    ctx->level = l;
    ctx->loading_x = level_pos.x;
    ctx->loading_y = level_pos.y;
}

static void wid_editor_tick (widp w)
{
    if (wid_editor_save_popup) {
        return;
    }

    wid_editor_ctx *ctx = wid_editor_window_ctx;
    if (!ctx) {
        return;
    }

    memset(ctx->map_highlight, 0, sizeof(ctx->map_highlight));

    if (ctx->got_line_start) {
        int mx = ctx->focusx + ctx->map_x;
        int my = ctx->focusy + ctx->map_y;

        wid_editor_draw_highlight_line(ctx->line_start_x, ctx->line_start_y, 
                                       mx, my);
    }

    if (ctx->got_square_start) {
        int mx = ctx->focusx + ctx->map_x;
        int my = ctx->focusy + ctx->map_y;

        wid_editor_draw_highlight_square(ctx->square_start_x, 
                                         ctx->square_start_y,
                                         mx, my);
    }

    if (ctx->got_cut_start) {
        int mx = ctx->focusx + ctx->map_x;
        int my = ctx->focusy + ctx->map_y;

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
            static uint ts;

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
                    (ctx->focusx < WID_EDITOR_MENU_MAP_ACROSS) && 
                    (ctx->focusy < WID_EDITOR_MENU_MAP_DOWN)) {

                    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
                        wid_editor_tile_left_button_pressed(ctx->focusx, 
                                                            ctx->focusy);
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
            static uint ts;

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
            static uint ts;

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
                    (ctx->edit_mode == WID_EDITOR_MODE_DRAW) &&
                    (ctx->focusx < WID_EDITOR_MENU_MAP_ACROSS) && 
                    (ctx->focusy < WID_EDITOR_MENU_MAP_DOWN)) {

                    if (space) {
                        wid_editor_tile_left_button_pressed(ctx->focusx, 
                                                            ctx->focusy);
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

static void wid_editor_save (const char *dir_and_file)
{
    wid_editor_ctx *ed = wid_editor_window_ctx;

    LOG("Saving: %s", dir_and_file);

    /*
     * Write the file.
     */
    marshal_p ctx;
    ctx = marshal(dir_and_file);

    marshal_level(ctx, ed->level);

    PUT_BRA(ctx);

    PUT_NAMED_UINT32(ctx, "width", MAP_WIDTH);
    PUT_NAMED_UINT32(ctx, "height", MAP_HEIGHT);

    int x, y, z;

    for (z = 0; z < MAP_DEPTH; z++) {
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
        /*
         * Success
         */
        char *popup_str = dynprintf("Saved %s", dir_and_file);
        widp popup = wid_tooltip(popup_str, 0.5f, 0.5f, med_font);
        wid_destroy_in(popup, ONESEC);
        myfree(popup_str);

        LOG("Saved: %s", dir_and_file);
    }
}

static void wid_editor_go_back (void)
{
    wid_destroy(&wid_editor_background);
    wid_destroy(&wid_editor_window);
    wid_map();
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
    LOG("Save editor level %s", tmp);
    wid_editor_save(tmp);
    myfree(tmp);
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
    wid_editor_save_popup = 
        wid_menu(0,
                vvlarge_font,
                large_font,
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

    if (ctx->undo_at > 1) {
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

    ctx->focusx = -1;
    ctx->focusy = -1;
    ctx->level_pos = level_pos;

    widp window;
    ctx->w = wid_editor_window = window = wid_new_window("wid editor");
    wid_set_client_context(window, ctx);
    ctx->is_new = true;

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

            if (y == WID_EDITOR_MENU_CELLS_DOWN - 1) {
            switch (x) {
            case WID_EDITOR_MODE_DRAW:
                wid_set_text(b, "Draw");
                if (!sdl_joy_axes) {
                    wid_set_tooltip(b, "D - shortcut", vsmall_font);
                }
                break;
            case WID_EDITOR_MODE_LINE:
                wid_set_text(b, "Line");
                if (!sdl_joy_axes) {
                    wid_set_tooltip(b, "L - shortcut", vsmall_font);
                }
                break;
            case WID_EDITOR_MODE_FILL:
                wid_set_text(b, "Fill");
                if (!sdl_joy_axes) {
                    wid_set_tooltip(b, "f - shortcut", vsmall_font);
                }
                break;
            case WID_EDITOR_MODE_DEL:
                wid_set_text(b, "Del");
                if (!sdl_joy_axes) {
                    wid_set_tooltip(b, "x - shortcut", vsmall_font);
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
                    wid_set_tooltip(b, "TAB - shortcut", vsmall_font);
                }
                break;
            case WID_EDITOR_MODE_YANK:
                wid_set_text(b, "Yank");
                if (!sdl_joy_axes) {
                    wid_set_tooltip(b, "y - shortcut. picks up a tile",
                                    vsmall_font);
                }
                break;
            case WID_EDITOR_MODE_SQUARE:
                wid_set_text(b, "Rect");
                if (!sdl_joy_axes) {
                    wid_set_tooltip(b, "r - shortcut",
                                    vsmall_font);
                }
                break;
            case WID_EDITOR_MODE_CUT:
                wid_set_text(b, "Cut");
                if (!sdl_joy_axes) {
                    wid_set_tooltip(b, "t - shortcut",
                                    vsmall_font);
                }
                break;
            case WID_EDITOR_MODE_COPY:
                wid_set_text(b, "Copy");
                if (!sdl_joy_axes) {
                    wid_set_tooltip(b, "c - shortcut",
                                    vsmall_font);
                }
                break;
            case WID_EDITOR_MODE_PASTE:
                wid_set_text(b, "Paste");
                if (!sdl_joy_axes) {
                    wid_set_tooltip(b, "p - shortcut",
                                    vsmall_font);
                }
                break;
            case WID_EDITOR_MODE_NUKE:
                wid_set_text(b, "Nuke");
                if (!sdl_joy_axes) {
                    wid_set_tooltip(b, "Z - shortcut. Destroy level.", 
                                    vsmall_font);
                }
                break;
            }
            }
        }
        }
    }

    if (saved_focus_x == -1) {
        ctx->focusx = WID_EDITOR_MENU_CELLS_ACROSS / 2;
    } else {
        ctx->focusx = saved_focus_x;
    }

    if (saved_focus_y == -1) {
        ctx->focusy = WID_EDITOR_MENU_CELLS_DOWN / 2;
    } else {
        ctx->focusy = saved_focus_y;
    }

    /*
     * Load all tiles
     */
    wid_editor_load_tiles();

    /*
     * Load the level
     */
    wid_editor_load_map(level_pos);

    map_fixup();

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
    ctx->tile_mode = 1;

    wid_editor_set_mode(WID_EDITOR_MODE_DRAW);
}
