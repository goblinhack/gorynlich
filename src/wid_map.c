/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "dir.h"
#include "color.h"
#include "string_ext.h"
#include "ttf.h"
#include "thing_template.h"
#include "wid_map.h"
#include "time_util.h"
#include "timer.h"
#include "level.h"
#include "math_util.h"
#include "thing_tile.h"
#include "wid_editor.h"
#include "wid_intro.h"
#include "tile.h"
#include "string_util.h"
#include "wid_tooltip.h"
#include "ramdisk.h"

static widp wid_map_window;
static wid_map_ctx *wid_map_window_ctx;
static widp wid_map_background;
static void wid_map_destroy(widp w);
static void wid_map_preview_small(widp w, fpoint tl, fpoint br);
static void wid_map_set_focus(wid_map_ctx *ctx, int focusx, int focusy);
static int saved_focus_x = 1;
static int saved_focus_y = 1;

static void wid_map_update_buttons (void)
{
    wid_map_ctx *ctx = wid_map_window_ctx;

    double width = 1.0 / (double)(LEVELS_DOWN);
    double height = 1.0 / (double)(LEVELS_ACROSS);

    int x, y;
    static int count;
    static int max_count = 1000;

    if (!ctx) {
        return;
    }

    verify(ctx);
    ctx->b = 0;

    for (x = 1; x < LEVELS_DOWN; x++) {
    for (y = 1; y < LEVELS_ACROSS; y++) {

        widp b = ctx->buttons[y][x];
        verify(b);

        fpoint tl;
        fpoint br;
        fontp font;
        color c;

        tl.x = (double) (x-1) * width;
        tl.y = (double) (y-1) * height;
        tl.x += width / 2.0;
        tl.y += height / 2.0;

        br.x = tl.x;
        br.y = tl.y;
        br.x += width;
        br.y += height;

        font = small_font;

        double zoom = 0.001;
        if ((x == ctx->focusx) && (y == ctx->focusy)) {
            tl.x -= zoom;
            tl.y -= zoom;
            br.x += zoom * 2.0;
            br.y += zoom * 2.0;
            c = GREEN;

            ctx->b = b;
        } else {
            tl.x += zoom;
            tl.y += zoom;
            br.x -= zoom * 2.0;
            br.y -= zoom * 2.0;

            c = GRAY70;
        }

        wid_set_tl_br_pct(b, tl, br);
        wid_set_color(b, WID_COLOR_TEXT, c);
        wid_set_font(b, font);

        wid_map_level *map = &ctx->levels[y][x];

        if (map && map->level) {
            color c;
            c = WHITE;
            c.a = 0;

            if (map->exit_count && map->player_count) {
                /*
                 * Ok
                 */
                wid_set_color(b, WID_COLOR_TL, WHITE);
                wid_set_color(b, WID_COLOR_BR, BLACK);
                wid_set_bevel(b, 0);

            } else if (!map->player_count) {
                /*
                 * No player start
                 */
                wid_set_color(b, WID_COLOR_TL, RED);
                wid_set_color(b, WID_COLOR_BR, RED);
                wid_set_bevel(b, 3);
            } else {
                /*
                 * No exit
                 */
                wid_set_color(b, WID_COLOR_TL, GOLD);
                wid_set_color(b, WID_COLOR_BR, GOLD);
                wid_set_bevel(b, 0);
            }

            wid_set_color(b, WID_COLOR_BG, c);
            wid_set_on_display(b, wid_map_preview_small);

        } else if ((x == ctx->focusx) && (y == ctx->focusy)) {
            color c = GRAY;
            c.a = 100;
            wid_set_color(b, WID_COLOR_BG, c);

            wid_set_color(b, WID_COLOR_TL, GREEN);
            wid_set_color(b, WID_COLOR_BR, GREEN);
            wid_set_bevel(b, 5);

        } else {
            color c = GRAY;
            c.a = 100;
            wid_set_color(b, WID_COLOR_BG, c);

            wid_set_color(b, WID_COLOR_TL, WHITE);
            wid_set_color(b, WID_COLOR_BR, BLACK);
            wid_set_bevel(b, 1);
        }

        if ((x == ctx->focusx) && (y == ctx->focusy)) {
            wid_set_color(b, WID_COLOR_TL, GREEN);
            wid_set_color(b, WID_COLOR_BR, GREEN);
            wid_set_bevel(b, 5);
        }
    }
    }

    count++;
    if (count >= max_count) {
        count = 0;
    }

    if (wid_map_window) {
        wid_update(wid_map_window);
    }

    /*
     * Zoom buttons in
     */
    if (ctx->is_new) {
        ctx->is_new = false;

        for (x = 1; x < LEVELS_DOWN; x++) {
            for (y = 1; y < LEVELS_ACROSS; y++) {

                widp b = ctx->buttons[y][x];

                fpoint tl;
                fpoint br;

                wid_get_tl_br(b, &tl, &br);

                double x = gauss(0.0, 2.0);
                double y = gauss(0.0, 2.0);

                wid_move_to_pct_centered(b, x, y);
                wid_move_to_centered_in(b, tl.x, tl.y, 500);
            }
        }
    }
}

static uint8_t wid_map_mouse_event (widp w, int focusx, int focusy)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    ctx->focusx = focusx;
    ctx->focusy = focusy;

    (ctx->selected)(ctx->w);

    return (true);
}

static uint8_t wid_map_button_mouse_event (widp w,
                                                int32_t x, int32_t y,
                                                uint32_t button)
{
    int focus = (typeof(focus)) (uintptr_t) wid_get_client_context2(w);
    int focusx = (focus & 0xff);
    int focusy = (focus & 0xff00) >> 8;

    return (wid_map_mouse_event(w, focusx, focusy));
}

static void wid_map_focus_right (wid_map_ctx *ctx)
{
    ctx->focusx++;
    if (ctx->focusx > LEVELS_DOWN - 1) {
        ctx->focusx = 0;
    }

    levelp l = ctx->levels[ctx->focusy][ctx->focusx].level;
    if (l) {
        char *tmp = dynprintf("%d.%d %s", ctx->focusy, ctx->focusx, level_get_title(l));
        wid_tooltip_transient(tmp, 500);
        myfree(tmp);
    }

    wid_map_update_buttons();
}

static void wid_map_focus_left (wid_map_ctx *ctx)
{
    ctx->focusx--;
    if (ctx->focusx < 0) {
        ctx->focusx = LEVELS_DOWN - 1;
    }

    levelp l = ctx->levels[ctx->focusy][ctx->focusx].level;
    if (l) {
        char *tmp = dynprintf("%d.%d %s", ctx->focusy, ctx->focusx, level_get_title(l));
        wid_tooltip_transient(tmp, 500);
        myfree(tmp);
    }

    wid_map_update_buttons();
}

static void wid_map_focus_down (wid_map_ctx *ctx)
{
    ctx->focusy++;
    if (ctx->focusy > LEVELS_ACROSS - 1) {
        ctx->focusy = 0;
    }

    levelp l = ctx->levels[ctx->focusy][ctx->focusx].level;
    if (l) {
        char *tmp = dynprintf("%d.%d %s", ctx->focusy, ctx->focusx, level_get_title(l));
        wid_tooltip_transient(tmp, 500);
        myfree(tmp);
    }

    wid_map_update_buttons();
}

static void wid_map_focus_up (wid_map_ctx *ctx)
{
    ctx->focusy--;
    if (ctx->focusy < 0) {
        ctx->focusy = LEVELS_ACROSS - 1;
    }

    levelp l = ctx->levels[ctx->focusy][ctx->focusx].level;
    if (l) {
        char *tmp = dynprintf("%d.%d %s", ctx->focusy, ctx->focusx, level_get_title(l));
        wid_tooltip_transient(tmp, 500);
        myfree(tmp);
    }

    wid_map_update_buttons();
}

static void wid_map_last_focus (wid_map_ctx *ctx)
{
    ctx->focusx = LEVELS_DOWN - 1;
    ctx->focusy = LEVELS_ACROSS - 1;

    wid_map_update_buttons();
}

static void wid_map_first_focus (wid_map_ctx *ctx)
{
    ctx->focusx = 0;
    ctx->focusy = 0;

    wid_map_update_buttons();
}

static void wid_map_set_focus (wid_map_ctx *ctx, int focusx, int focusy)
{
    ctx->focusx = focusx;
    ctx->focusy = focusy;

    wid_map_update_buttons();
}

static uint8_t wid_map_parent_key_down (widp w, const SDL_KEYSYM *key)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    switch (key->sym) {
        case '`':
            return (false);

        case SDLK_ESCAPE:
            (ctx->cancelled)(ctx->w);
            return (true);

        case ' ': 
        case SDLK_RETURN: 
            (ctx->selected)(ctx->w);
            return (true);

        case SDLK_LEFT:
            wid_map_focus_left(ctx);
            break;

        case SDLK_RIGHT:
            wid_map_focus_right(ctx);
            break;

        case SDLK_UP:
            wid_map_focus_up(ctx);
            break;

        case SDLK_DOWN:
            wid_map_focus_down(ctx);
            break;

        case SDLK_HOME:
            wid_map_first_focus(ctx);
            break;

        case SDLK_END:
            wid_map_last_focus(ctx);
            break;

        default:
            return (true);
    }

    return (true);
}

static uint8_t wid_map_parent_joy_button (widp w, int32_t x, int32_t y)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);
    int ret = false;

    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_B]) {
        (ctx->cancelled)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_X]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_Y]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_LEFT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_RIGHT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_STICK_DOWN]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_STICK_DOWN]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_START]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_XBOX]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_BACK]) {
        (ctx->cancelled)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_UP]) {
        wid_map_focus_up(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_DOWN]) {
        wid_map_focus_down(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT]) {
        wid_map_focus_left(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT]) {
        wid_map_focus_right(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_FIRE]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_FIRE]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }

    wid_map_update_buttons();
    if (ctx->b) {
        wid_mouse_warp(ctx->b);
    }

    return (ret);
}

static uint8_t wid_map_button_key_event (widp w, const SDL_KEYSYM *key)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    switch (key->sym) {
        case '`':
            return (false);

        case SDLK_ESCAPE:
            (ctx->cancelled)(ctx->w);
            return (true);

        case ' ':
        case SDLK_RETURN:
            (ctx->selected)(ctx->w);
            return (true);

        case SDLK_DOWN:
        case SDLK_HOME:
        case SDLK_END:
        case SDLK_BACKSPACE:
        case SDLK_DELETE:
        case SDLK_LEFT:
        case SDLK_RIGHT:
        case SDLK_UP:
            break;

        default:
            return (true);
    }

    return (false);
}

static uint8_t wid_map_button_joy_down_event (widp w, 
                                                     int32_t x, int32_t y)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);
    int ret = false;

    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
        ret = wid_map_mouse_event(w, ctx->focusx, ctx->focusy);
    }

    if (sdl_joy_buttons[SDL_JOY_BUTTON_B]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_X]) {
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_Y]) {
        (ctx->selected)(ctx->w);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_LEFT]) {
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_RIGHT]) {
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_STICK_DOWN]) {
        wid_map_mouse_event(w, ctx->focusx, ctx->focusy);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_STICK_DOWN]) {
        wid_map_mouse_event(w, ctx->focusx, ctx->focusy);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_START]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_XBOX]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_BACK]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_UP]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_DOWN]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_FIRE]) {
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_FIRE]) {
        wid_map_mouse_event(w, ctx->focusx, ctx->focusy);
        ret = true;
    }

    wid_map_update_buttons();
    if (ctx->b) {
        wid_mouse_warp(ctx->b);
    }

    return (ret);
}

static void wid_map_mouse_over (widp w,
                                int32_t relx, int32_t rely,
                                int32_t wheelx, int32_t wheely)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    if (!relx && !rely && !wheelx && !wheely) {
        return;
    }

    /*
     * If we recreate the map with a fixed focus we will be told about
     * a mouse over event immediately which may not be over the focus item
     * and will cause us to move. Annoying.
     */
    if (time_get_time_ms() - ctx->created < 500) {
        return;
    }

    int focus = (typeof(focus)) (uintptr_t) wid_get_client_context2(w);
    int focusx = (focus & 0xff);
    int focusy = (focus & 0xff00) >> 8;

    wid_map_set_focus(ctx, focusx, focusy);
}

static void wid_map_destroy (widp w)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    wid_set_client_context(w, 0);

    if (wid_map_background) {
        wid_destroy(&wid_map_background);
    }

    int x, y;

    for (x = 1; x < LEVELS_DOWN; x++) {
        for (y = 1; y < LEVELS_ACROSS; y++) {

            levelp l = ctx->levels[y][x].level;
            if (!l) {
                continue;
            }

            ctx->levels[y][x].level = 0;
            level_destroy(&l, false /* keep players */);
        }
    }

    saved_focus_x = ctx->focusx;
    saved_focus_y = ctx->focusy;

    myfree(ctx);
    wid_map_window = 0;
    wid_map_window_ctx = 0;
}

static void wid_map_destroy_begin (widp w)
{
    wid_map_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    /*
     * Zoom buttons in
     */
    int x, y;

    for (x = 1; x < LEVELS_DOWN; x++) {
        for (y = 1; y < LEVELS_ACROSS; y++) {

            widp b = ctx->buttons[y][x];
            fpoint tl;
            fpoint br;

            wid_get_tl_br(b, &tl, &br);
            double x = gauss(0.0, 2.0);
            double y = gauss(0.0, 2.0);

            wid_move_to_pct_centered_in(b, x, y, 500);
        }
    }
}

void wid_map_cell_load (void)
{
    wid_map_ctx *ctx = wid_map_window_ctx;
    verify(ctx);
    verify(ctx->w);

    /*
     * If we recreate the map with a fixed focus we will be told about
     * a mouse over event immediately which may not be over the focus item
     * and will cause us to move. Annoying.
     */
    if (time_get_time_ms() - ctx->created < 500) {
        return;
    }

    level_pos_t level_pos;

    level_pos.x = ctx->focusx;
    level_pos.y = ctx->focusy;

    if (level_pos.x == -1) {
        return;
    }

    if (level_pos.y == -1) {
        return;
    }

    wid_editor(level_pos);

    wid_destroy(&wid_map_window);
}

void wid_map_visible (void)
{
    wid_visible(wid_map_window, 0);
}

static void wid_map_cell_selected (widp w)
{
    wid_map_ctx *ctx = wid_map_window_ctx;
    if (!wid_map_window_ctx) {
        return;
    }

    level_pos_t level_pos;

    level_pos.x = ctx->focusx;
    level_pos.y = ctx->focusy;

    if (ctx->on_selected) {
        (*ctx->on_selected)(level_pos);
        wid_destroy(&wid_map_window);
        return;
    }

    if (level_pos.x == -1) {
        return;
    }

    if (level_pos.y == -1) {
        return;
    }

    LOG("Client: Edit selected level %d.%d", level_pos.y, level_pos.x);

    levelp l = ctx->levels[ctx->focusy][ctx->focusx].level;

    char *tmp = dynprintf("%s, %d.%d",
                          l ? level_get_title(l) : "Unnamed level",
                          ctx->focusy,
                          ctx->focusx);

    wid_hide(wid_map_window, 0);

    wid_map_cell_load();

    myfree(tmp);
}

static void wid_map_cell_cancelled (widp w)
{
    wid_map_ctx *ctx = wid_map_window_ctx;
    if (!wid_map_window_ctx) {
        return;
    }

    if (ctx->on_cancelled) {
        (*ctx->on_cancelled)();
        wid_destroy(&wid_map_window);
        return;
    }

    wid_destroy(&wid_map_window);
    wid_intro_visible();
}

static void wid_map_bg_create (void)
{
    widp wid;

    if (wid_map_background) {
        return;
    }

    {
        wid = wid_map_background = wid_new_window("bg");

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

static void wid_map_find_exits (int x, int y)
{
    wid_map_ctx *ctx = wid_map_window_ctx;
    if (!wid_map_window_ctx) {
        return;
    }

    wid_map_level *map = &ctx->levels[y][x];
    if (!map) {
        return;
    }

    int mx, my, mz;

    for (mx = 0; mx < MAP_WIDTH; mx++)
    for (my = 0; my < MAP_HEIGHT; my++) 
    for (mz = 0; mz < MAP_DEPTH_MAX; mz++) {
        wid_map_tile *tile = &map->tiles[mx][my][mz];
        if (!tile) {
            continue;
        }

        tpp tp = tile->tp;
        if (!tp) {
            continue;
        }

        tpp_data data = &tile->data;
        if (tp_is_exit(tp) && data->exit_set) {

            int ex = data->exit.x;
            int ey = data->exit.y;

            int e;
            for (e = 0; e < WID_MAP_EXITS_MAX; e++) {
                if (!map->exit_level_x[e] && !map->exit_level_y[e]) {
                    map->exit_level_x[e] = ex;
                    map->exit_level_y[e] = ey;
                    map->exit_at_mx[e] = mx;
                    map->exit_at_my[e] = my;
                    map->exit_count++;
                    break;
                }
            }
        }
    }
}

static void wid_map_find_player_start (int x, int y)
{
    wid_map_ctx *ctx = wid_map_window_ctx;
    if (!wid_map_window_ctx) {
        return;
    }

    wid_map_level *map = &ctx->levels[y][x];
    if (!map) {
        return;
    }

    int mx, my, mz;

    for (my = 0; my < MAP_HEIGHT; my++) {
        for (mx = 0; mx < MAP_WIDTH; mx++) {
            mz = MAP_DEPTH_OBJ;

            wid_map_tile *tile = &map->tiles[mx][my][mz];
            if (!tile) {
                continue;
            }

            tpp tp = tile->tp;
            if (!tp) {
                continue;
            }

            if (tp_is_player(tp)) {
                map->px = mx;
                map->py = my;
                map->player_count++;
            }
        }
    }
}

/*
 * Draw lines to all ezits from level tiles.
 */
static void wid_map_draw_exits (void)
{
    /*
     * Pathetic glowing lines effect.
     */
    static uint8_t a;
    static int da = 1;

    a += da;
    if (a == 255) {
        da = -1;
    }
    if (a == 0) {
        da = 1;
    }
    
    wid_map_ctx *ctx = wid_map_window_ctx;
    if (!wid_map_window_ctx) {
        return;
    }

    int x, y;

    for (x = 1; x < LEVELS_DOWN; x++) 
    for (y = 1; y < LEVELS_ACROSS; y++) {

        wid_map_level *map = &ctx->levels[y][x];
        if (!map) {
            continue;
        }

        widp b = ctx->buttons[y][x];
        int32_t tlx, tly, brx, bry;
        wid_get_abs_coords(b, &tlx, &tly, &brx, &bry);

        int e;
        for (e = 0; e < map->exit_count; e++) {
            int sx = map->exit_at_mx[e];
            int sy = map->exit_at_my[e];
            int ex = map->exit_level_x[e];
            int ey = map->exit_level_y[e];

            widp b2 = ctx->buttons[ey][ex];
            if (!b2) {
                continue;
            }

            wid_map_level *map2 = &ctx->levels[ey][ex];
            if (!map2) {
                continue;
            }

            int px = map2->px;
            int py = map2->py;

            int32_t tlx2, tly2, brx2, bry2;
            wid_get_abs_coords(b2, &tlx2, &tly2, &brx2, &bry2);

            double left = 
                tlx + ((((double)(brx - tlx)) / ((double)MAP_WIDTH)) * sx);
            double top = 
                tly + (((double)(bry - tly)) / ((double)MAP_HEIGHT)) * sy;

            double right = 
                tlx2 + ((((double)(brx2 - tlx2)) / ((double)MAP_WIDTH)) * px);
            double bottom = 
                tly2 + (((double)(bry2 - tly2)) / ((double)MAP_HEIGHT)) * py;

            color c = RED;
            c.a = a;
            glcolor(c);

            gl_blitline(left, top, right, bottom);
        }
    }
}

/*
 * Show a large level preview
 */
#if 0
static void wid_map_preview_do (int thumbnail)
{
    wid_map_ctx *ctx = wid_map_window_ctx;
    if (!wid_map_window_ctx) {
        return;
    }

    verify(ctx);

    /*
     * Draw exits on top of the rendered level tiles.
     */
    wid_map_draw_exits();

    if (ctx->focusx == -1) {
        return;
    }

    if (ctx->focusy == -1) {
        return;
    }

    wid_map_level *map = &ctx->levels[ctx->focusy][ctx->focusx];
    if (!map) {
        return;
    }

    int x, y, z;

    blit_init();

    for (x = 0; x < MAP_WIDTH; x++) 
    for (y = 0; y < MAP_HEIGHT; y++) 
    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        tilep tile = map->tiles[x][y][z].tile;
        if (!tile) {
            tpp tp = map->tiles[x][y][z].tp;
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

            tile = tile_find(tilename);
            if (!tile) {
                ERR("cannot find tilep for tile %s", tilename);
            }

            map->tiles[x][y][z].tile = tile;
        }

        fpoint tl;
        fpoint br;

        /*
         * Scale
         */
        double dx = 0.005;
        double dy = 0.005;

        if (thumbnail) {
            dx /= 2.0;
            dy /= 2.0;
        }

        if (ctx->focusx > LEVELS_ACROSS / 2) {
            tl.x = ((double)x+1) * dx;
            br.x = ((double)x+2.5) * dx;
            tl.y = ((double)y+1) * dy;
            br.y = ((double)y+2.5) * dy;
        }

        if (ctx->focusy > LEVELS_DOWN / 2) {
            tl.x = ((double)x-2.5) * dx;
            br.x = ((double)x-1) * dx;
            tl.y = ((double)y-2.5) * dy;
            br.y = ((double)y-1) * dy;
        }

        tl.x *= (double) global_config.video_gl_width;
        tl.y *= (double) global_config.video_gl_height;
        br.x *= (double) global_config.video_gl_width;
        br.y *= (double) global_config.video_gl_height;

        widp b = ctx->buttons[ctx->focusy][ctx->focusx];

        if (!thumbnail) {
            int32_t tlx, tly, brx, bry, mx, my;

            wid_get_abs_coords(b, &tlx, &tly, &brx, &bry);

            mx = (tlx + brx) / 2.0;
            my = (tly + bry) / 2.0;

            tl.x += mx;
            tl.y += my;
            br.x += mx;
            br.y += my;
        } else {
            double offset = 16;
            tl.x += (double) global_config.video_gl_width / offset;
            tl.y += (double) global_config.video_gl_height / offset;
            br.x += (double) global_config.video_gl_width / offset;
            br.y += (double) global_config.video_gl_height / offset;
        }

        glcolor(WHITE);
        tile_blit_fat(tile, 0, tl, br);
    }

    blit_flush();
}
#endif

static void wid_map_preview_do (int thumbnail)
{
    wid_map_ctx *ctx = wid_map_window_ctx;
    if (!wid_map_window_ctx) {
        return;
    }

    verify(ctx);

    wid_map_draw_exits();

    if (ctx->focusx == -1) {
        return;
    }

    if (ctx->focusy == -1) {
        return;
    }

    wid_map_level *map = &ctx->levels[ctx->focusy][ctx->focusx];
    if (!map) {
        return;
    }

    int x, y, z;

    blit_init();

    for (x = 0; x < MAP_WIDTH; x++) 
    for (y = 0; y < MAP_HEIGHT; y++) 
    for (z = 0; z < MAP_DEPTH_MAX; z++) {

        tilep tile = map->tiles[x][y][z].tile;
        if (!tile) {
            tpp tp = map->tiles[x][y][z].tp;
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

            tile = tile_find(tilename);
            if (!tile) {
                ERR("cannot find tilep for tile %s", tilename);
            }

            map->tiles[x][y][z].tile = tile;
        }

        fpoint tl;
        fpoint br;

        double dx = 0.004;
        double dy = 0.004;

        if (thumbnail) {
            dx /= 2.0;
            dy /= 2.0;
        }

        tl.x = ((double)x+2) * dx;
        br.x = ((double)x+3.5) * dx;
        tl.y = ((double)y+2) * dy;
        br.y = ((double)y+3.5) * dy;


        if (ctx->focusx > LEVELS_ACROSS / 2) {
            tl.x -= ((double)160) * dx;
            br.x -= ((double)160) * dx;
        } else {
            tl.x += ((double)20) * dx;
            br.x += ((double)20) * dx;
        }

        if (ctx->focusy > LEVELS_DOWN / 2) {
            tl.y -= ((double)160) * dy;
            br.y -= ((double)160) * dy;
        } else {
            tl.y += ((double)20) * dy;
            br.y += ((double)20) * dy;
        }

        tl.x *= (double) global_config.video_gl_width;
        tl.y *= (double) global_config.video_gl_height;
        br.x *= (double) global_config.video_gl_width;
        br.y *= (double) global_config.video_gl_height;

        widp b = ctx->buttons[ctx->focusy][ctx->focusx];

        if (!thumbnail) {
            int32_t tlx, tly, brx, bry, mx, my;

            wid_get_abs_coords(b, &tlx, &tly, &brx, &bry);

            mx = (tlx + brx) / 2.0;
            my = (tly + bry) / 2.0;

            tl.x += mx;
            tl.y += my;
            br.x += mx;
            br.y += my;
        } else {
            double offset = 16;
            tl.x += (double) global_config.video_gl_width / offset;
            tl.y += (double) global_config.video_gl_height / offset;
            br.x += (double) global_config.video_gl_width / offset;
            br.y += (double) global_config.video_gl_height / offset;
        }

        glcolor(WHITE);
        tile_blit_fat(tile, 0, tl, br);
    }

    blit_flush();
}

static void wid_map_preview_small (widp b, fpoint tl, fpoint br)
{
    wid_map_ctx *ctx = wid_map_window_ctx;
    if (!wid_map_window_ctx) {
        return;
    }

    verify(ctx);

    int xy = (typeof(xy)) (uintptr_t) wid_get_client_context2(b);
    int lx = (xy & 0xff);
    int ly = (xy & 0xff00) >> 8;

    wid_map_level *map = &ctx->levels[ly][lx];
    if (!map) {
        return;
    }

    double dx = (br.x - tl.x) / (double)MAP_WIDTH;
    double dy = (br.y - tl.y) / (double)MAP_HEIGHT;

    int x, y, z;
    int step = 4;

    glcolor(WHITE);

    blit_init();

    for (x = 0; x < MAP_WIDTH; x+=step) 
    for (y = 0; y < MAP_HEIGHT; y+=step) 
    for (z = 0; z < MAP_DEPTH_MAX; z++) {
        tilep tile = map->tiles[x][y][z].tile;
        if (!tile) {
            tpp tp = map->tiles[x][y][z].tp;
            if (!tp) {
                continue;
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

            tile = tile_find(tilename);
            if (!tile) {
                ERR("cannot find tilep for tile %s", tilename);
            }

            map->tiles[x][y][z].tile = tile;
        }

        fpoint tl2;
        fpoint br2;

        tl2.x = tl.x + (dx * (double)x);
        tl2.y = tl.y + (dy * (double)y);

        br2.x = tl.x + (dx * ((double)x) + ((double)step)+3.0);
        br2.y = tl.y + (dy * ((double)y) + ((double)step)+3.0);

        if (br2.x > br.x) {
            br2.x = br.x;
        }

        if (br2.y > br.y) {
            br2.y = br.y;
        }

        tile_blit_fat(tile, 0, tl2, br2);
    }

    blit_flush();
}

void wid_map_preview (widp w)
{
    wid_map_preview_do(false);
}

void wid_map_preview_thumbnail (widp w)
{
    wid_map_preview_do(true);
}

/*
 * Replace or place a tile.
 */
widp wid_editor_level_map_thing_replace_template (widp w,
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
    wid_map_ctx *ctx = wid_map_window_ctx;
    verify(ctx);
    verify(ctx->w);

    int ix = (int)x;
    int iy = (int)y;

    wid_map_level *map = &ctx->levels[ctx->loading_y][ctx->loading_x];
    if (!map) {
        ERR("no map to write to at position (%f,%f) -> (%d,%d) in level %u.%u, "
            "map bounds (%d,%d) -> (%d,%d)", 
            x, y, ix, iy, ctx->loading_x, ctx->loading_y,
            0, 0, MAP_DEPTH_MAX, MAP_HEIGHT);
    }

    if ((ix >= MAP_WIDTH) || (iy >= MAP_HEIGHT) || (ix < 0) || (iy < 0)) {
        ERR("overflow in reading position (%f,%f) -> (%d,%d) in level %u.%u, "
            "map bounds (%d,%d) -> (%d,%d)", 
            x, y, ix, iy, ctx->loading_x, ctx->loading_y,
            0, 0, MAP_DEPTH_MAX, MAP_HEIGHT);
        return (0);
    }

    int z = tp_get_z_depth(tp);
    map->tiles[ix][iy][z].tp = tp;

    if (data) {
        map->tiles[ix][iy][z].data = *data;
    }

    return (0);
}

static int wid_map_loaded_levels[LEVELS_ACROSS][LEVELS_DOWN];

static void wid_map_load_level (wid_map_ctx *ctx, const char *name, int x, int y)
{
    level_pos_t level_pos;

    level_pos.x = x;
    level_pos.y = y;

    ctx->loading_x = x;
    ctx->loading_y = y;

    /*
     * Ignore out of bounds levels like the test level.
     */
    if ((ctx->loading_x >= LEVELS_ACROSS) || 
        (ctx->loading_y >= LEVELS_DOWN)) {
        return;
    }

    if ((ctx->loading_x < 0) || 
        (ctx->loading_y < 0)) {
        return;
    }

    wid_map_loaded_levels[y][x] = 1;

    levelp l = level_load(level_pos, 
                          ctx->w,
                          false, /* is_editor */
                          true, /* is_map_editor */
                          false /* on_server */);
    if (!l) {
        ERR("Failed to load level %d.%d", y, x);
        return;
    }

    ctx->levels[y][x].level = l;

    widp b = ctx->buttons[y][x];

    char *tmp = dynprintf("%d.%d %s", y, x, level_get_title(l));
    wid_set_tooltip(b, tmp, med_font);
    myfree(tmp);

    wid_set_font(b, vsmall_font);

    wid_map_find_player_start(x, y);
    wid_map_find_exits(x, y);
}

static void wid_map_load_levels (wid_map_ctx *ctx)
{
    memset(wid_map_loaded_levels, 0, sizeof(wid_map_loaded_levels));

    tree_file_node *n;
    tree_root *d;

    d = dirlist(LEVELS_PATH,
                0 /* context->include_suffix */,
                0 /* context->exclude_suffix */,
                0 /* context->include_dirs */,
                false /* include_ramdisk */);

    { TREE_WALK(d, n) {

        const char *name = n->tree.key;

        /*
         * Ignore dot files
         */
        if (name[0] == '.') {
            continue;
        }

        int x, y;

        if (sscanf(name, "%d.%d", &y, &x) != 2) {
            ERR("bad format in level name [%s], expecting a,b format", name);
            continue;
        }

        wid_map_load_level(ctx, name, x, y);
    } }

    dirlist_free(&d);

    ramdisk_t *ramfile = ramdisk_get_data();

    while (ramfile->filename) {
        int x, y;

        if (sscanf(ramfile->filename, "data/levels/%d.%d", &y, &x) != 2) {
            ramfile++;
            continue;
        }

        /*
         * Ignore out of bounds levels like the test level.
         */
        if ((ctx->loading_x >= LEVELS_ACROSS) || 
            (ctx->loading_y >= LEVELS_DOWN)) {
            ramfile++;
            continue;
        }

        if ((ctx->loading_x < 0) || 
            (ctx->loading_y < 0)) {
            ramfile++;
            continue;
        }

        if (wid_map_loaded_levels[y][x]) {
            ramfile++;
            continue;
        }

        wid_map_load_level(ctx, ramfile->filename, x, y);
        ramfile++;
    }
}

widp wid_map (const char *title, 
              on_selected_t on_selected, 
              on_cancelled_t on_cancelled)
{
    wid_map_event_t selected = wid_map_cell_selected;
    wid_map_event_t cancelled = wid_map_cell_cancelled;

    /*
     * Create a context to hold button info so we can update it when the focus 
     * changes
     */
    wid_map_ctx *ctx = myzalloc(sizeof(*ctx), "wid map");
    wid_map_window_ctx = ctx;

    ctx->focusx = -1;
    ctx->focusy = -1;
    ctx->cancelled = cancelled;
    ctx->selected = selected;
    ctx->on_selected = on_selected;
    ctx->on_cancelled = on_cancelled;

    widp window;
    ctx->w = wid_map_window = window = wid_new_window("wid map");
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

        wid_set_on_destroy_begin(window, wid_map_destroy_begin);
        wid_set_on_key_down(window, wid_map_parent_key_down);
        wid_set_on_joy_down(window, wid_map_parent_joy_button);
        wid_set_on_destroy(window, wid_map_destroy);
    }

    /*
     * Create the title
     */
    {
        widp w = wid_new_square_button(window, "wid map title");
        wid_set_no_shape(w);

        fpoint tl = { 0.0, 0.0};
        fpoint br = { 1.0, 0.15};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_text(w, title);
        wid_set_text_outline(w, true);
        wid_set_font(w, vvlarge_font);

        wid_set_color(w, WID_COLOR_TEXT, YELLOW);
    }

    /*
     * Create the button container
     */
    {
        widp button_container = wid_new_square_button(window, 
                                                      "wid map buttons");
        wid_set_no_shape(button_container);

        fpoint tl = { 0.0, 0.1};
        fpoint br = { 1.0, 1.0};

        wid_set_tl_br_pct(button_container, tl, br);
        wid_set_client_context(button_container, ctx);

        /*
         * Create the buttons
         */
        int x, y;

        for (x = 1; x < LEVELS_DOWN; x++) {
        for (y = 1; y < LEVELS_ACROSS; y++) {
            widp b = wid_new_square_button(button_container,
                                           "wid map button");
            ctx->buttons[y][x] = b;
            ctx->levels[y][x].x = x;
            ctx->levels[y][x].y = y;

            wid_set_on_mouse_over_begin(b, wid_map_mouse_over);
            wid_set_on_key_down(b, wid_map_button_key_event);
            wid_set_on_joy_down(b, wid_map_button_joy_down_event);
            wid_set_on_mouse_down(b, wid_map_button_mouse_event);
            wid_set_bevel(b, 1);

            wid_set_color(b, WID_COLOR_BG, GRAY20);
            wid_set_color(b, WID_COLOR_TL, GRAY60);
            wid_set_color(b, WID_COLOR_BR, GRAY10);
            wid_set_color(b, WID_COLOR_TEXT, GREEN);

            wid_set_mode(b, WID_MODE_OVER);

            wid_set_color(b, WID_COLOR_BG, GRAY20);
            wid_set_color(b, WID_COLOR_TL, GRAY70);
            wid_set_color(b, WID_COLOR_BR, GRAY10);
            wid_set_color(b, WID_COLOR_TEXT, GREEN);

            wid_set_mode(b, WID_MODE_NORMAL);

            wid_set_client_context(b, ctx);
            int focus = (y << 8) | x;
            wid_set_client_context2(b, (void*) (uintptr_t) focus);
        }
        }
    }

    if (saved_focus_x == -1) {
        ctx->focusx = LEVELS_ACROSS / 2;
    } else {
        ctx->focusx = saved_focus_x;
    }

    if (saved_focus_y == -1) {
        ctx->focusy = LEVELS_DOWN / 2;
    } else {
        ctx->focusy = saved_focus_y;
    }

    /*
     * Load all levels
     */
    wid_map_load_levels(ctx);

    /*
     * Repair the context so it is not pointing at the last level loaded.
     */
    wid_set_client_context(window, ctx);

    wid_map_update_buttons();
    wid_update(window);
    wid_raise(window);
    wid_map_update_buttons();
    wid_update(window);
    wid_raise(window);
    wid_map_bg_create();

    wid_set_on_display_top_level(window, wid_map_preview);

    ctx->created = time_get_time_ms();

    return (window);
}
