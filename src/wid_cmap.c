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
#include "wid_cmap.h"
#include "time_util.h"
#include "timer.h"
#include "color.h"
#include "math_util.h"
#include "thing_tile.h"
#include "wid_editor.h"
#include "wid_intro.h"
#include "tile.h"
#include "string_util.h"

static widp wid_cmap_window;
static wid_cmap_ctx *wid_cmap_window_ctx;
static widp wid_cmap_background;
static void wid_cmap_destroy(widp w);
static void wid_cmap_set_focus(wid_cmap_ctx *ctx, int focusx, int focusy);
static int saved_focus_x = 0;
static int saved_focus_y = 0;

static void wid_cmap_update_buttons (void)
{
    wid_cmap_ctx *ctx = wid_cmap_window_ctx;

    double width = 1.0 / (double)(COLORS_DOWN);
    double height = 1.0 / (double)(COLORS_ACROSS);

    int x, y;
    static int count;
    static int max_count = 1000;

    if (!ctx) {
        return;
    }

    verify(ctx);
    ctx->b = 0;

    for (x = 0; x < COLORS_DOWN; x++) {
    for (y = 0; y < COLORS_ACROSS; y++) {

        widp b = ctx->buttons[y][x];
        verify(b);

        fpoint tl;
        fpoint br;
        fontp font;
        color c;

        tl.x = (double) (x) * width;
        tl.y = (double) (y) * height;

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
            c = WHITE;

            ctx->b = b;
        } else {
            tl.x += zoom;
            tl.y += zoom;
            br.x -= zoom * 2.0;
            br.y -= zoom * 2.0;

            c = WHITE;
        }

        wid_set_tl_br_pct(b, tl, br);
        wid_set_color(b, WID_COLOR_TEXT, c);
        wid_set_font(b, font);

        wid_cmap_color *map = &ctx->colors[y][x];

        c = map->color;
        wid_set_color(b, WID_COLOR_BG, c);

        if ((x == ctx->focusx) && (y == ctx->focusy)) {
            wid_set_color(b, WID_COLOR_TL, GREEN);
            wid_set_color(b, WID_COLOR_BR, GREEN);
            wid_set_bevel(b, 1);
        } else {
            wid_set_color(b, WID_COLOR_TL, WHITE);
            wid_set_color(b, WID_COLOR_BR, BLACK);
            wid_set_bevel(b, 0);
        }
    }
    }

    count++;
    if (count >= max_count) {
        count = 0;
    }

    if (wid_cmap_window) {
        wid_update(wid_cmap_window);
    }

    /*
     * Zoom buttons in
     */
    if (ctx->is_new) {
        ctx->is_new = false;

        for (x = 0; x < COLORS_DOWN; x++) {
            for (y = 0; y < COLORS_ACROSS; y++) {

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

static uint8_t wid_cmap_mouse_event (widp w, int focusx, int focusy)
{
    wid_cmap_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    ctx->focusx = focusx;
    ctx->focusy = focusy;

    (ctx->selected)(ctx->w);

    return (true);
}

static uint8_t wid_cmap_button_mouse_event (widp w,
                                                int32_t x, int32_t y,
                                                uint32_t button)
{
    int focus = (typeof(focus)) (uintptr_t) wid_get_client_context2(w);
    int focusx = (focus & 0xff);
    int focusy = (focus & 0xff00) >> 8;

    return (wid_cmap_mouse_event(w, focusx, focusy));
}

static void wid_cmap_focus_right (wid_cmap_ctx *ctx)
{
    ctx->focusx++;
    if (ctx->focusx > COLORS_DOWN - 1) {
        ctx->focusx = 0;
    }

    wid_cmap_update_buttons();
}

static void wid_cmap_focus_left (wid_cmap_ctx *ctx)
{
    ctx->focusx--;
    if (ctx->focusx < 0) {
        ctx->focusx = COLORS_DOWN - 1;
    }

    wid_cmap_update_buttons();
}

static void wid_cmap_focus_down (wid_cmap_ctx *ctx)
{
    ctx->focusy++;
    if (ctx->focusy > COLORS_ACROSS - 1) {
        ctx->focusy = 0;
    }

    wid_cmap_update_buttons();
}

static void wid_cmap_focus_up (wid_cmap_ctx *ctx)
{
    ctx->focusy--;
    if (ctx->focusy < 0) {
        ctx->focusy = COLORS_ACROSS - 1;
    }

    wid_cmap_update_buttons();
}

static void wid_cmap_last_focus (wid_cmap_ctx *ctx)
{
    ctx->focusx = COLORS_DOWN - 1;
    ctx->focusy = COLORS_ACROSS - 1;

    wid_cmap_update_buttons();
}

static void wid_cmap_first_focus (wid_cmap_ctx *ctx)
{
    ctx->focusx = 0;
    ctx->focusy = 0;

    wid_cmap_update_buttons();
}

static void wid_cmap_set_focus (wid_cmap_ctx *ctx, int focusx, int focusy)
{
    ctx->focusx = focusx;
    ctx->focusy = focusy;

    wid_cmap_update_buttons();
}

static uint8_t wid_cmap_parent_key_down (widp w, const SDL_KEYSYM *key)
{
    wid_cmap_ctx *ctx = wid_get_client_context(w);
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
            wid_cmap_focus_left(ctx);
            break;

        case SDLK_RIGHT:
            wid_cmap_focus_right(ctx);
            break;

        case SDLK_UP:
            wid_cmap_focus_up(ctx);
            break;

        case SDLK_DOWN:
            wid_cmap_focus_down(ctx);
            break;

        case SDLK_HOME:
            wid_cmap_first_focus(ctx);
            break;

        case SDLK_END:
            wid_cmap_last_focus(ctx);
            break;

        default:
            return (true);
    }

    return (true);
}

static uint8_t wid_cmap_parent_joy_button (widp w, int32_t x, int32_t y)
{
    wid_cmap_ctx *ctx = wid_get_client_context(w);
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
        wid_cmap_focus_up(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_DOWN]) {
        wid_cmap_focus_down(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT]) {
        wid_cmap_focus_left(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT]) {
        wid_cmap_focus_right(ctx);
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

    wid_cmap_update_buttons();
    if (ctx->b) {
        wid_mouse_warp(ctx->b);
    }

    return (ret);
}

static uint8_t wid_cmap_button_key_event (widp w, const SDL_KEYSYM *key)
{
    wid_cmap_ctx *ctx = wid_get_client_context(w);
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

static uint8_t wid_cmap_button_joy_down_event (widp w, 
                                                     int32_t x, int32_t y)
{
    wid_cmap_ctx *ctx = wid_get_client_context(w);
    verify(ctx);
    int ret = false;

    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
        ret = wid_cmap_mouse_event(w, ctx->focusx, ctx->focusy);
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
        wid_cmap_mouse_event(w, ctx->focusx, ctx->focusy);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_STICK_DOWN]) {
        wid_cmap_mouse_event(w, ctx->focusx, ctx->focusy);
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
        wid_cmap_mouse_event(w, ctx->focusx, ctx->focusy);
        ret = true;
    }

    wid_cmap_update_buttons();
    if (ctx->b) {
        wid_mouse_warp(ctx->b);
    }

    return (ret);
}

static void wid_cmap_mouse_over (widp w,
                                int32_t relx, int32_t rely,
                                int32_t wheelx, int32_t wheely)
{
    wid_cmap_ctx *ctx = wid_get_client_context(w);
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

    wid_cmap_set_focus(ctx, focusx, focusy);
}

static void wid_cmap_destroy (widp w)
{
    wid_cmap_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    wid_set_client_context(w, 0);

    if (wid_cmap_background) {
        wid_destroy(&wid_cmap_background);
    }

    saved_focus_x = ctx->focusx;
    saved_focus_y = ctx->focusy;

    myfree(ctx);
    wid_cmap_window = 0;
    wid_cmap_window_ctx = 0;
}

static void wid_cmap_destroy_begin (widp w)
{
    wid_cmap_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    /*
     * Zoom buttons in
     */
    int x, y;

    for (x = 0; x < COLORS_DOWN; x++) {
        for (y = 0; y < COLORS_ACROSS; y++) {

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

void wid_cmap_visible (void)
{
    wid_visible(wid_cmap_window, 0);
}

static void wid_cmap_cell_selected (widp w)
{
    wid_cmap_ctx *ctx = wid_cmap_window_ctx;
    if (!wid_cmap_window_ctx) {
        return;
    }

    if (ctx->focusx == -1) {
        return;
    }

    if (ctx->focusy == -1) {
        return;
    }

    const char *n = ctx->colors[ctx->focusy][ctx->focusx].name;

    if (ctx->on_selected) {
        (*ctx->on_selected)(n);
        wid_destroy(&wid_cmap_window);
        return;
    }

    wid_hide(wid_cmap_window, 0);
}

static void wid_cmap_cell_cancelled (widp w)
{
    wid_cmap_ctx *ctx = wid_cmap_window_ctx;

    if (ctx->on_cancelled) {
        (*ctx->on_cancelled)();
        wid_destroy(&wid_cmap_window);
        return;
    }

    wid_destroy(&wid_cmap_window);
    wid_intro_visible();
}

static void wid_cmap_bg_create (void)
{
    widp wid;

    if (wid_cmap_background) {
        return;
    }

    {
        wid = wid_cmap_background = wid_new_window("bg");

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

static void wid_cmap_load_colors (wid_cmap_ctx *ctx)
{
    tree_color_val *c;
    int x, y;

    x = 0;
    y = 0;

    { TREE_WALK(colors, c) {

        if (!c->include_in_palette) {
            continue;
        }

        const char *name = c->tree.key;
        ctx->colors[y][x].color = c->c;
        ctx->colors[y][x].name = name;

        widp b = ctx->buttons[y][x];
        if (!b) {
            ERR("no cmap button");
        }

        wid_set_tooltip(b, name, med_font);
        wid_set_font(b, vsmall_font);

        x++;
        if (x >= COLORS_ACROSS) {
            x = 0;
            y++;
        }
    } }
}

widp wid_cmap (const char *title, 
               on_cmap_selected_t on_selected, 
               on_cmap_cancelled_t on_cancelled)
{
    wid_cmap_event_t selected = wid_cmap_cell_selected;
    wid_cmap_event_t cancelled = wid_cmap_cell_cancelled;

    /*
     * Create a context to hold button info so we can update it when the focus 
     * changes
     */
    wid_cmap_ctx *ctx = myzalloc(sizeof(*ctx), "wid map");
    wid_cmap_window_ctx = ctx;

    ctx->focusx = -1;
    ctx->focusy = -1;
    ctx->cancelled = cancelled;
    ctx->selected = selected;
    ctx->on_selected = on_selected;
    ctx->on_cancelled = on_cancelled;

    widp window;
    ctx->w = wid_cmap_window = window = wid_new_window("wid map");
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

        wid_set_on_destroy_begin(window, wid_cmap_destroy_begin);
        wid_set_on_key_down(window, wid_cmap_parent_key_down);
        wid_set_on_joy_down(window, wid_cmap_parent_joy_button);
        wid_set_on_destroy(window, wid_cmap_destroy);
    }

    /*
     * Create the title
     */
    {
        widp w = wid_new_square_button(window, "wid map title");
        wid_set_no_shape(w);

        fpoint tl = { 0.0, 0.0};
        fpoint br = { 1.0, 0.1};

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

        for (x = 0; x < COLORS_DOWN; x++) {
        for (y = 0; y < COLORS_ACROSS; y++) {
            widp b = wid_new_square_button(button_container,
                                           "wid map button");
            ctx->buttons[y][x] = b;
            ctx->colors[y][x].x = x;
            ctx->colors[y][x].y = y;

            wid_set_on_mouse_over_begin(b, wid_cmap_mouse_over);
            wid_set_on_key_down(b, wid_cmap_button_key_event);
            wid_set_on_joy_down(b, wid_cmap_button_joy_down_event);
            wid_set_on_mouse_down(b, wid_cmap_button_mouse_event);
            wid_set_bevel(b, 0);

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
        ctx->focusx = COLORS_ACROSS / 2;
    } else {
        ctx->focusx = saved_focus_x;
    }

    if (saved_focus_y == -1) {
        ctx->focusy = COLORS_DOWN / 2;
    } else {
        ctx->focusy = saved_focus_y;
    }

    /*
     * Load all colors
     */
    wid_cmap_load_colors(ctx);

    /*
     * Repair the context so it is not pointing at the last color loaded.
     */
    wid_set_client_context(window, ctx);

    wid_cmap_update_buttons();
    wid_update(window);
    wid_raise(window);
    wid_cmap_update_buttons();
    wid_update(window);
    wid_raise(window);
    wid_cmap_bg_create();

    ctx->created = time_get_time_ms();

    return (window);
}
