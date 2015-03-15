/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the README file for license.
 */

#include <SDL.h>

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
#include "wid_intro.h"
#include "tile.h"
#include "string_util.h"

static void wid_editor_destroy(widp w);
static void wid_editor_tile_right_button_pressed(widp w, int x, int y);
static void wid_editor_set_focus(wid_editor_ctx *ctx, int focusx, int focusy);
static void wid_editor_map_scroll(int dx, int dy);

static widp wid_editor_window;
static widp wid_editor_background;
static wid_editor_ctx *wid_editor_window_ctx;

static int saved_focus_x = -1;
static int saved_focus_y = -1;
static tpp wid_editor_chosen_tile;

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

    ctx->map_tile[ix][iy][z].tp = tp;

    return (0);
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
        color c = CYAN;
        c.a = 50;
        wid_set_color(b, WID_COLOR_BG, c);
        wid_set_color(b, WID_COLOR_TL, WHITE);
        wid_set_color(b, WID_COLOR_BR, GRAY);
        wid_set_color(b, WID_COLOR_TEXT, GRAY);
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

        int over_tile = 0;
        int over_map_tile = 0;

        if (!ctx->tile_mode) {
            int z;
            int mx = x + ctx->map_x;
            int my = y + ctx->map_y;

            if ((x < WID_EDITOR_MENU_MAP_ACROSS) && 
                (y < WID_EDITOR_MENU_MAP_DOWN)) {
                for (z = 0; z < MAP_DEPTH; z++) {
                    if (ctx->map_tile[mx][my][z].tp) {
                        over_map_tile = 1;
                        break;
                    }
                }
            }
        } else {
            if (ctx->tile[x][y].tile_tp) {
                over_tile = 1;
            }
        }

        double zoom = 0.002;
        if ((x == ctx->focusx) && (y == ctx->focusy)) {
            if (!over_map_tile) {
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

        if ((x == ctx->focusx) && (y == ctx->focusy)) {
            color c = RED;
            c.a = 200;
            wid_set_color(b, WID_COLOR_BG, c);
        } else if (over_tile) {
            color c = BLACK;
            c.a = 200;
            wid_set_color(b, WID_COLOR_BG, c);
        } else if (over_map_tile) {
            color c = WHITE;
            c.a = 0;
            wid_set_color(b, WID_COLOR_BG, c);
        } else {
            color c = GRAY;
            c.a = 50;
            wid_set_color(b, WID_COLOR_BG, c);
        }

        {
            wid_set_bevel(b, 1);
            color c = WHITE;
            c.a = 20;
            wid_set_color(b, WID_COLOR_TL, c);
            wid_set_color(b, WID_COLOR_BR, c);
        }
    }
    }

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

        glcolor(WHITE);
        tile_blit_fat(tile, 0, tl, br);
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

    width *= 1.25;
    height *= 1.25;

    br.x = tl.x + width;
    tl.y = br.y - height;

    blit_init();

    int z;
    for (z = 0; z < MAP_DEPTH; z++) {
        tpp tp = ctx->map_tile[x][y][z].tp;
        if (!tp) {
            continue;
        }

        thing_tilep thing_tile;
        tree_rootp tiles;

        tiles = tp_get_tiles(tp);
        if (!tiles) {
            return;
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

    ctx->map_tile[x][y][z].tp = wid_editor_chosen_tile;
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
}

static void wid_editor_tile_fill (int x, int y)
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
        if (ctx->map_tile[x][y][z].tp) {
            return;
        }
    }

    z = tp_get_z_depth(wid_editor_chosen_tile);
    ctx->map_tile[x][y][z].tp = wid_editor_chosen_tile;

    wid_editor_tile_fill(x + 1, y);
    wid_editor_tile_fill(x - 1, y);
    wid_editor_tile_fill(x, y + 1);
    wid_editor_tile_fill(x, y - 1);
}

static void wid_editor_tile_left_button_pressed (widp w, int x, int y)
{
    wid_editor_ctx *ctx = wid_get_client_context(w);
    int xy = (typeof(xy)) (uintptr_t) wid_get_client_context2(w);
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
    } else if (wid_editor_chosen_tile) {
        if ((x < WID_EDITOR_MENU_MAP_ACROSS) && 
            (y < WID_EDITOR_MENU_MAP_DOWN)) {

            int z = tp_get_z_depth(wid_editor_chosen_tile);

            switch (ctx->edit_mode) {
            case WID_EDITOR_MODE_DRAW:
                ctx->map_tile[mx][my][z].tp = wid_editor_chosen_tile;
                break;
            case WID_EDITOR_MODE_LINE:
                if (!ctx->got_line_start) {
                    ctx->got_line_start = true;
                    ctx->line_start_x = mx;
                    ctx->line_start_y = my;
                } else {
                    wid_editor_draw_line (ctx->line_start_x, 
                                          ctx->line_start_y, mx, my);
                    ctx->got_line_start = false;
                }
                break;
            case WID_EDITOR_MODE_FILL:
                wid_editor_tile_fill(mx, my);
                break;
            case WID_EDITOR_MODE_DEL:
                wid_editor_tile_right_button_pressed(w, x, y);
                break;
            }
        }
    }
}

static void wid_editor_tile_right_button_pressed (widp w, int x, int y)
{
    wid_editor_ctx *ctx = wid_get_client_context(w);
    int xy = (typeof(xy)) (uintptr_t) wid_get_client_context2(w);
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
                tpp tp = ctx->map_tile[mx][my][z].tp;
                if (tp) {
                    ctx->map_tile[mx][my][z].tp = 0;
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
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    int xy = (typeof(xy)) (uintptr_t) wid_get_client_context2(w);
    int x = (xy & 0xff);
    int y = (xy & 0xff00) >> 8;

    if (y == WID_EDITOR_MENU_CELLS_DOWN - 1) {
        if (x < WID_EDITOR_MODE_MAX) {
            ctx->edit_mode = x;
            return (true);
        }
    }

    if (button == 1) {
        wid_editor_tile_left_button_pressed(w, x, y);
    }

    if ((button == 2) || (button == 3)) {
        wid_editor_tile_right_button_pressed(w, x, y);
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

    switch (key->sym) {
        case '`':
            return (false);

        case '\t':
            wid_editor_tile_mode_toggle();
            return (true);

        case ' ':
            wid_editor_tile_left_button_pressed(w, x, y);
            return (false);

        case SDLK_BACKSPACE:
            wid_editor_tile_right_button_pressed(w, x, y);
            return (true); 

        case 'l':
            ctx->edit_mode = WID_EDITOR_MODE_LINE; 
            return (true);

        case 'd':
            ctx->edit_mode = WID_EDITOR_MODE_DRAW; 
            return (true);

        case 'f':
            if (ctx->tile_mode) {
                ctx->edit_mode = WID_EDITOR_MODE_FILL; 
            } else {
                int mx = x + ctx->map_x;
                int my = y + ctx->map_y;
                wid_editor_tile_fill(mx, my);
            }
            return (true);

        case 'x':
            ctx->edit_mode = WID_EDITOR_MODE_DEL; 
            return (true);

        case SDLK_ESCAPE:
            return (true);

        case SDLK_RETURN: {
            return (true);

        case SDLK_LEFT:
            wid_editor_focus_left();
            break;

        case SDLK_RIGHT:
            wid_editor_focus_right();
            break;

        case SDLK_UP:
            wid_editor_focus_up();
            break;

        case SDLK_DOWN:
            wid_editor_focus_down();
            break;

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
        wid_editor_tile_left_button_pressed(w, x, y);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_B]) {
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_X]) {
        wid_editor_tile_right_button_pressed(w, x, y);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_Y]) {
        wid_editor_tile_mode_toggle();
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_LEFT]) {
        ctx->edit_mode--;
        if (ctx->edit_mode < 0) {
            ctx->edit_mode = 0;
        }
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_RIGHT]) {
        ctx->edit_mode++;
        if (ctx->edit_mode > WID_EDITOR_MODE_MAX) {
            ctx->edit_mode = WID_EDITOR_MODE_MAX;
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
        wid_editor_tile_left_button_pressed(w, x, y);
        return (true);
    }

    /*
     * MACOS seems bugged in SDL with this
     */
    if (mouse_down & SDL_BUTTON_RIGHT) {
        wid_editor_tile_right_button_pressed(w, x, y);
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
                ctx->map_tile[x][y][z].tp = 0;
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

static uint8_t wid_editor_load_tile (const tree_node *node, void *arg)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    verify(ctx);
    verify(ctx->w);

    static int x;
    static int y;
    tpp tp;

    tp = (typeof(tp)) 
            (((char*) node) - STRUCT_OFFSET(struct tp_, tree2));

    if (tp_is_hidden_from_editor(tp)) {
        return (true);
    }

    ctx->tile[x][y].tile_tp = tp;

    x++;

    if (x >= WID_EDITOR_MENU_TILES_ACROSS) {
        x = 0;
        y++;
    }

    if (y >= WID_EDITOR_MENU_TILES_DOWN) {
        y++;
        DIE("too many tiles; implement a scrollbar neil");
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
        ERR("Failed to load level %d.%d", level_pos.y, level_pos.x);
        return;
    }

    ctx->level = l;
    ctx->loading_x = level_pos.x;
    ctx->loading_y = level_pos.y;
}

static void wid_editor_tick (widp w)
{
    wid_editor_ctx *ctx = wid_editor_window_ctx;
    if (!ctx) {
        return;
    }

    int moved = 0;
    static uint ts1;

    if (time_have_x_thousandths_passed_since(
                        DELAY_THOUSANDTHS_PLAYER_POLL * 6, ts1)) {

        ts1 = time_get_time_ms();

        if (sdl_joy_axes) {

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
        }
    }

    static uint ts2;

    if (time_have_x_thousandths_passed_since(
                        DELAY_THOUSANDTHS_PLAYER_POLL, ts2)) {

        ts2 = time_get_time_ms();

        if (sdl_joy_axes) {

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

    if (moved) {
        if (ctx->b) {
            wid_mouse_warp(ctx->b);
        }
    }

    wid_editor_update_buttons();
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
        c.a = 100;
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
                break;
            case WID_EDITOR_MODE_LINE:
                wid_set_text(b, "Line");
                break;
            case WID_EDITOR_MODE_FILL:
                wid_set_text(b, "Fill");
                break;
            case WID_EDITOR_MODE_DEL:
                wid_set_text(b, "Del");
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

    /*
     * Repair the context so it is not pointing at the last level loaded.
     */
    wid_set_client_context(window, ctx);

    wid_editor_update_buttons();
    wid_update(window);
    wid_raise(window);
    wid_editor_update_buttons();
    wid_update(window);
    wid_raise(window);
    wid_editor_bg_create();

    ctx->created = time_get_time_ms();
    ctx->tile_mode = 1;
}
