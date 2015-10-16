/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the README file for license info for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "color.h"
#include "string_ext.h"
#include "ttf.h"
#include "wid_menu.h"
#include "time_util.h"
#include "timer.h"
#include "math_util.h"

int wid_menu_visible;

static void wid_menu_destroy(widp w);
static void wid_menu_callback(widp w, 
                              int focus, 
                              on_mouse_down_t event_handler,
                              int key);
static void wid_menu_prev_focus(wid_menu_ctx *ctx);
static void wid_menu_next_focus(wid_menu_ctx *ctx);

static void wid_menu_update (widp w)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    int items = ctx->items;

    double total_height_needed = 
                    ctx->focus_height + 
                    ctx->normal_height * (items - 1);

    /*
     * We make the selected item larger than the rest so we need to work out 
     * by what percent that the larger item should get and by how much to 
     * shring the smaller items so it all fits into 100%
     */
    double focus_wid_height = ctx->focus_height / total_height_needed;
    double normal_wid_height = (ctx->normal_height * ((double)items - 1)) / 
                    total_height_needed;
    normal_wid_height /= (double)(items - 1);

    double y = 0;

    int i;
    for (i = 0; i < items; i++) {

        double cx = (1.0 - ctx->total_col_width) / 2.0;
        int c;
        double saved_y = y;

        for (c = 0; c < ctx->cols; c++) {
            y = saved_y;

            widp b = ctx->buttons[i][c];
            verify(b);

            widp bar = ctx->bar[i][c];
            if (bar) {
                verify(bar);
            }

            double bar_width = ctx->bar_width[i][c];

            wid_set_text_centerx(b, 1);
            wid_set_text_centery(b, 1);

            fpoint tl = { 0.0, 0.0};
            fpoint br = { 1.0, 1.0};

            tl.x = cx;
            br.x = cx + ctx->col_width[c];
            cx = br.x;

            fontp font;
            color c;

            tl.y = y;

            if (i == ctx->focus) {
                font = ctx->focus_font;
                y += focus_wid_height;
                c = GREEN;
            } else {
                font = ctx->normal_font;
                y += normal_wid_height;

                if (!ctx->event_handler[i]) {
                    c = YELLOW;
                } else {
                    c = GRAY70;
                }
            }

            br.y = y;

            wid_set_tl_br_pct(b, tl, br);

            color transparent = BLACK;
            transparent.a = 0;

            /*
             * Make sure the other widgets look plain in all modes.
             */
            int mode;
            for (mode = WID_MODE_NORMAL; mode < WID_MODE_LAST; mode++) {
                wid_set_no_shape(b);
                wid_set_mode(b, mode);
                wid_set_color(b, WID_COLOR_TEXT, c);
                wid_set_color(b, WID_COLOR_BG, transparent);
                wid_set_color(b, WID_COLOR_TL, transparent);
                wid_set_color(b, WID_COLOR_BR, transparent);
                wid_set_font(b, font);
                wid_set_text_outline(b, true);
            }

            wid_set_mode(b, WID_MODE_NORMAL);
            wid_update(b);

            if (bar) {
                fpoint tl;
                fpoint br;

                tl.x = 0.1;
                tl.y = 0;
                br.x = bar_width;
                br.y = 0.9;

                wid_set_bevel(bar, 2);
                wid_set_tl_br_pct(bar, tl, br);
                wid_set_color(bar, WID_COLOR_BG, c);

                wid_set_color(bar, WID_COLOR_TL, WHITE);
                wid_set_color(bar, WID_COLOR_BR, BLACK);
            }
        }
    }

    wid_update(w);

    if (ctx->on_update) {
        (ctx->on_update)(w, ctx->focus);
    }
}

static uint8_t wid_menu_mouse_event (widp w,
                                     int item,
                                     int col,
                                     int32_t x, int32_t y,
                                     uint32_t button)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    /*
     * Don't process events too soon. Else the menu might not even have 
     * appeared.
     */
    if (time_get_time_ms() - ctx->created < 100) {
        return (false);
    }

    widp b = ctx->buttons[item][col];
    verify(b);

    on_mouse_down_t event_handler = ctx->event_handler[item];
    if (!event_handler) {
        return (false);
    }

    (event_handler)(b, mouse_x, mouse_y, SDL_BUTTON_LEFT);

    return (true);
}

static uint8_t wid_menu_button_mouse_event (widp w,
                                            int32_t x, int32_t y,
                                            uint32_t button)
{
    verify(w);

    int item = (typeof(item)) (uintptr_t) wid_get_client_context2(w);
    int col = (typeof(col)) (uintptr_t) wid_get_client_context3(w);

    return (wid_menu_mouse_event(w, item, col, x, y, button));
}

static uint8_t wid_menu_parent_mouse_event (widp w,
                                            int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    int item = ctx->focus;

    return (wid_menu_mouse_event(w, item, 0, x, y, button));
}

static uint8_t wid_menu_joy_down_event (widp w,
                                        int32_t x, int32_t y)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    /*
     * Don't process events too soon. Else the menu might not even have 
     * appeared.
     */
    if (time_get_time_ms() - ctx->created < 100) {
        return (false);
    }

    int item = ctx->focus;
    int col = 0;

    widp b = ctx->buttons[item][col];
    verify(b);

    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
        on_mouse_down_t event_handler = ctx->event_handler[item];
        (event_handler)(b, mouse_x, mouse_y, SDL_BUTTON_LEFT);
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_B]) {
        ctx->focus = ctx->items - 1;
        widp b = ctx->buttons[item][col];
        verify(b);
        on_mouse_down_t event_handler = ctx->event_handler[item];
        wid_menu_callback(b, ctx->focus, event_handler, SDL_BUTTON_LEFT);
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_X]) {
        wid_menu_prev_focus(ctx);
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_Y]) {
        wid_menu_next_focus(ctx);
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_LEFT]) {
        on_mouse_down_t event_handler = ctx->event_handler[item];
        (event_handler)(b, mouse_x, mouse_y, SDL_BUTTON_LEFT);
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_RIGHT]) {
        on_mouse_down_t event_handler = ctx->event_handler[item];
        (event_handler)(b, mouse_x, mouse_y, SDL_BUTTON_RIGHT);
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_STICK_DOWN]) {
        on_mouse_down_t event_handler = ctx->event_handler[item];
        (event_handler)(b, mouse_x, mouse_y, SDL_BUTTON_LEFT);
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_STICK_DOWN]) {
        on_mouse_down_t event_handler = ctx->event_handler[item];
        (event_handler)(b, mouse_x, mouse_y, SDL_BUTTON_LEFT);
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_START]) {
        on_mouse_down_t event_handler = ctx->event_handler[item];
        (event_handler)(b, mouse_x, mouse_y, SDL_BUTTON_LEFT);
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_XBOX]) {
        on_mouse_down_t event_handler = ctx->event_handler[item];
        (event_handler)(b, mouse_x, mouse_y, SDL_BUTTON_LEFT);
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_BACK]) {
        ctx->focus = ctx->items - 1;
        widp b = ctx->buttons[item][col];
        verify(b);
        on_mouse_down_t event_handler = ctx->event_handler[item];
        wid_menu_callback(b, ctx->focus, event_handler, SDL_BUTTON_LEFT);
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_UP]) {
        wid_menu_prev_focus(ctx);
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_DOWN]) {
        wid_menu_next_focus(ctx);
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_FIRE]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_FIRE]) {
    }

    return (true);
}

static uint8_t wid_menu_button_joy_down_event (widp w,
                                            int32_t x, int32_t y)
{
    return (wid_menu_joy_down_event(w, x, y));
}

static uint8_t wid_menu_parent_joy_down_event (widp w,
                                            int32_t x, int32_t y)
{
    return (wid_menu_joy_down_event(w, x, y));
}

static void wid_menu_next_focus (wid_menu_ctx *ctx)
{
    ctx->focus++;
    if (ctx->focus > ctx->items - 1) {
        ctx->focus = 0;
    }

    if (!ctx->event_handler[ctx->focus]) {
        wid_menu_next_focus(ctx);
    }

    wid_menu_update(ctx->w);
}

static void wid_menu_prev_focus (wid_menu_ctx *ctx)
{
    ctx->focus--;
    if (ctx->focus < 0) {
        ctx->focus = ctx->items - 1;
    }

    if (!ctx->event_handler[ctx->focus]) {
        wid_menu_prev_focus(ctx);
    }

    wid_menu_update(ctx->w);
}

static void wid_menu_last_focus (wid_menu_ctx *ctx)
{
    ctx->focus = ctx->items - 1;

    wid_menu_update(ctx->w);
}

static void wid_menu_first_focus (wid_menu_ctx *ctx)
{
    ctx->focus = 0;

    if (!ctx->event_handler[ctx->focus]) {
        wid_menu_next_focus(ctx);
    }

    wid_menu_update(ctx->w);
}

static void wid_menu_set_focus (wid_menu_ctx *ctx, int focus)
{
    ctx->focus = focus;

    wid_menu_update(ctx->w);
}

typedef struct {
    on_mouse_down_t event_handler;
    widp w;
    int key;
} wid_menu_callback_ctx;

static void wid_menu_callback_do (void *context)
{
    wid_menu_callback_ctx *tmp = (typeof(tmp)) context;

    verify(tmp);

    (tmp->event_handler)(tmp->w, mouse_x, mouse_y, tmp->key);

    myfree(tmp);
}

static void wid_menu_callback (widp w, 
                               int focus, 
                               on_mouse_down_t event_handler,
                               int key)
{
    if (!event_handler) {
        return;
    }

    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);
    wid_menu_set_focus(ctx, focus);

    wid_menu_callback_ctx *tmp = myzalloc(sizeof(*tmp), "menu callback");
    tmp->w = w;
    tmp->event_handler = event_handler;
    tmp->key = key;

    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_menu_callback_do,
            (action_timer_destroy_callback)0,
            tmp, /* context */
            "menu callback",
            1, 0 /* jitter */);
}


static uint8_t wid_menu_key_event (widp w, const SDL_KEYSYM *key)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    /*
     * Don't process events too soon. Else the menu might not even have 
     * appeared.
     */
    if (time_get_time_ms() - ctx->created < 100) {
        return (false);
    }

    switch (key->sym) {
        case SDLK_TAB:
            wid_menu_next_focus(ctx);
            break;

        case SDLK_BACKSPACE:
        case SDLK_ESCAPE:
            ctx->focus = ctx->items - 1;

            /*
             * Fall through
             */

        case ' ':
        case SDLK_RETURN: {
                widp b = ctx->buttons[ctx->focus][0];
                verify(b);

                on_mouse_down_t event_handler = 
                                ctx->event_handler[ctx->focus];

                wid_menu_callback(b, ctx->focus, event_handler, 
                                  SDL_BUTTON_LEFT);
                return (true);
            }

        case SDLK_LEFT: {
                widp b = ctx->buttons[ctx->focus][0];
                verify(b);

                on_mouse_down_t event_handler = 
                                ctx->event_handler[ctx->focus];
                
                wid_menu_callback(b, ctx->focus, event_handler, 
                                  SDL_BUTTON_LEFT);
                return (true);
            }

        case SDLK_RIGHT: {
                widp b = ctx->buttons[ctx->focus][0];
                verify(b);

                on_mouse_down_t event_handler = 
                                ctx->event_handler[ctx->focus];

                wid_menu_callback(b, ctx->focus, event_handler, 
                                  SDL_BUTTON_RIGHT);
                return (true);
            }

        case SDLK_UP:
            wid_menu_prev_focus(ctx);
            break;

        case SDLK_DOWN:
            wid_menu_next_focus(ctx);
            break;

        case SDLK_HOME:
            wid_menu_first_focus(ctx);
            break;

        case SDLK_END:
            wid_menu_last_focus(ctx);
            break;

        default: {
            int i;

            /*
             * Shortcut key pressed?
             */
            for (i = 0; i < ctx->items; i++) {
                if (key->sym == ctx->shortcut[i]) {
                    on_mouse_down_t event_handler = 
                                    ctx->event_handler[i];

                    widp b = ctx->buttons[i][0];
                    verify(b);

                    wid_menu_callback(b, i, event_handler, (int)SDLK_RIGHT);

                    return (true);
                }
            }
            return (false);
        }
    }

    return (true);
}

static uint8_t wid_menu_button_key_event (widp w, const SDL_KEYSYM *key)
{
    return (wid_menu_key_event(w, key));
}

static uint8_t wid_menu_parent_key_event (widp w, const SDL_KEYSYM *key)
{
    return (wid_menu_key_event(w, key));
}

static void wid_menu_mouse_over (widp w,
                                 int32_t relx, int32_t rely,
                                 int32_t wheelx, int32_t wheely)

{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    int item = (typeof(item)) (uintptr_t) wid_get_client_context2(w);
    int col = (typeof(col)) (uintptr_t) wid_get_client_context3(w);

    if (!relx && !rely && !wheelx && !wheely) {
        return;
    }

    /*
     * Don't process events too soon. Else the menu might not even have 
     * appeared.
     */
    if (time_get_time_ms() - ctx->created < 100) {
        return;
    }

    verify(ctx);

    /*
     * If we recreate the menu with a fixed focus we will be told about
     * a mouse over event immediately which may not be over the focus item
     * and will cause us to move. Annoying.
     */
    if (time_get_time_ms() - ctx->created < 100) {
        return;
    }

    ctx->focus = item;

    if (!ctx->event_handler[ctx->focus]) {
        wid_menu_next_focus(ctx);
    }

    wid_menu_update(ctx->w);
}

static void wid_menu_destroy (widp w)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    wid_set_client_context(w, 0);
    myfree(ctx);

    wid_menu_visible = false;
}

static void wid_menu_tick (widp w)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    int items = ctx->items;

    static int val;
    static int delta = 1;
    static int step = 2;

    if (val > 250) {
        val = 250;
        delta = -1;
    }

    if (val < 100) {
        val = 100;
        delta = 1;
    }

    static uint32_t ts;

    if (time_have_x_hundredths_passed_since(1, ts)) {
        ts = time_get_time_ms();

        val += delta * step;
    }

    int i;
    for (i = 0; i < items; i++) {

        int c;
        for (c = 0; c < ctx->cols; c++) {

            widp b = ctx->buttons[i][c];
            verify(b);

            wid_set_text_centerx(b, 1);
            wid_set_text_centery(b, 1);

            color c;

            uint8_t tick = rand() % 255;
            c.a = tick;
            if (i != ctx->focus) {
                continue;
            }

            c = GREEN;
            c.g = val;

            /*
             * Make sure the other widgets look plain in all modes.
             */
            int old_mode = wid_get_mode(b);

            int mode;
            for (mode = WID_MODE_NORMAL; mode < WID_MODE_LAST; mode++) {
                wid_set_no_shape(b);
                wid_set_mode(b, mode);
                wid_set_color(b, WID_COLOR_TEXT, c);
                wid_set_text_outline(b, true);
            }

            wid_set_mode(b, old_mode);
        }
    }

#ifdef ENABLE_FLICKER
    {
    static uint32_t ts;

    if (!time_have_x_tenths_passed_since(11, ts)) {
        return;
    }

    ts = time_get_time_ms();

    /*
     * Make the background flicker.
     */
    static double r;
    static double g;
    static double b;

    static double nr;
    static double ng;
    static double nb;

    if (!(myrand() % 100)) {
        static int cnt = 0;
	static color cols[1000] = {
        };

        if (!cnt) {
            cols[cnt++] = FIREBRICK;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED1;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED2;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED3;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED2;
	    cols[cnt++] = RED1;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED2;
	    cols[cnt++] = RED4;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED1;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED1;
	    cols[cnt++] = RED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED2;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED3;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED2;
	    cols[cnt++] = RED2;
	    cols[cnt++] = RED1;
	    cols[cnt++] = RED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED2;
	    cols[cnt++] = RED3;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED3;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED4;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGERED3;
	    cols[cnt++] = RED2;
	    cols[cnt++] = RED1;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED2;
	    cols[cnt++] = RED3;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED2;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED2;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED2;
	    cols[cnt++] = ORANGERED3;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGERED3;
	    cols[cnt++] = RED4;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED1;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED2;
	    cols[cnt++] = RED2;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED1;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED2;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED1;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED2;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED2;
	    cols[cnt++] = RED2;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED1;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED1;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED2;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED2;
	    cols[cnt++] = RED3;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED4;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED4;
	    cols[cnt++] = ORANGERED3;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED4;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGE4;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED1;
	    cols[cnt++] = RED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED2;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED4;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED2;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED1;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED4;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED2;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED2;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED2;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = ORANGERED4;
	    cols[cnt++] = RED2;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED1;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED2;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED1;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED2;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED3;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED3;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED4;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED4;
	    cols[cnt++] = ORANGERED;
	    cols[cnt++] = RED;
	    cols[cnt++] = RED4;
	    cols[cnt++] = RED;
	    cols[cnt++] = ORANGE;
	    cols[cnt++] = RED4;
	}


        color c = cols[myrand() % cnt];

        nr = c.r;
        ng = c.g;
        nb = c.b;
    }

    color c;

    r = (r * 10.0 + nr) / 11.0;
    g = (g * 10.0 + ng) / 11.0;
    b = (b * 10.0 + nb) / 11.0;

    /*
     * And then I decided to dump all the red stuff above and just make it 
     * fade in and out. Oh well.
     */
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = rand() % 50;

    {
        widp w = ctx->w;
        wid_set_color(w, WID_COLOR_BG, c);
        wid_set_color(w, WID_COLOR_TL, c);
        wid_set_color(w, WID_COLOR_BR, c);
    }
    }
#endif
}

static void wid_menu_destroy_begin (widp w)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    int i;
    for (i = 0; i < ctx->items; i++) {

        int c;
        for (c = 0; c < ctx->cols; c++) {
            widp b = ctx->buttons[i][c];
            verify(b);

            double x = gauss(0.0, 2.0);
            wid_move_delta_pct_in(b, x, 0.0, 200);
        }
    }
}

widp wid_menu (widp parent,
               fontp focus_font,
               fontp normal_font,
               on_update_t on_update,
               double x,
               double y,
               int cols,
               int focus,
               int items, ...)
{
    if (items >= WID_MENU_MAX_ITEMS) {
        ERR("too many menu items");
    }

    wid_menu_visible = true;

    /*
     * Create a context to hold button info so we can update it when the focus 
     * changes
     */
    wid_menu_ctx *ctx = myzalloc(sizeof(*ctx), "wid menu");

    ctx->on_update = on_update;

    /*
     * Work out how large each button bar should be.
     */
    double unused;
    ttf_text_size(&focus_font, "X", &unused, 
                  &ctx->focus_height, 0, 1.0f, 1.0f,
                  false /* fixed width */);

    ttf_text_size(&normal_font, "X", &unused, 
                  &ctx->normal_height, 0, 1.0f, 1.0f,
                  false /* fixed width */);

    /*
     * Padding
     */
    double padding = 0.95;
    ctx->focus_height *= padding;
    ctx->normal_height *= padding;

    widp wrapper;
    if (parent) {
        wrapper = wid_new_container(parent, "wid menu");
    } else {
        wrapper = wid_new_window("wid menu");
    }

    {
        fpoint tl = { 0.0, 0.0};
        fpoint br = { 1.0, 1.0};

        wid_set_tl_br_pct(wrapper, tl, br);

#if 0
        color c = BLACK;
        c.a = 100;
        wid_set_color(wrapper, WID_COLOR_BG, c);
        wid_set_color(wrapper, WID_COLOR_TL, c);
        wid_set_color(wrapper, WID_COLOR_BR, c);
#endif

        wid_set_on_destroy_begin(wrapper, wid_menu_destroy_begin);
        wid_set_on_destroy(wrapper, wid_menu_destroy);
        wid_set_client_context(wrapper, ctx);
        wid_set_on_key_down(wrapper, wid_menu_parent_key_event);
        wid_set_on_mouse_down(wrapper, wid_menu_parent_mouse_event);
        wid_set_on_joy_down(wrapper, wid_menu_parent_joy_down_event);
        ctx->w = wrapper;
    }

    /*
     * Create the button container
     */
    widp w = wid_new_container(wrapper, "wid menu");
    wid_set_on_tick(w, wid_menu_tick);

    {
        fpoint tl = { 0.0, 0.0};
        fpoint br = { 0.0, 0.0};

        double total_height_needed = 
                        ctx->focus_height + 
                        ctx->normal_height * (items - 1);

        br.x = global_config.video_gl_width;

        /*
         * Need a bit of fudge so the text outline does not creep out of the 
         * widget scissors.
         */
        br.y = total_height_needed * 1.2;

        wid_set_tl_br(w, tl, br);
        wid_move_to_pct_centered(w, 0.5, 0.5);
    }

    wid_set_client_context(w, ctx);
    ctx->items = items;
    ctx->cols = cols;
    ctx->focus = focus;
    ctx->focus_font = focus_font;
    ctx->normal_font = normal_font;

    va_list ap;
    int i;
    va_start(ap, items);

    /*
     * If multiple columns then read the widths of each.
     */
    if (cols > 1) {
        int c;
        for (c = 0; c < cols; c++) {
            double width = va_arg(ap, double);
            ctx->col_width[c] = width;
            ctx->total_col_width += width;
        }
    } else {
        ctx->col_width[0] = 1.0;
        ctx->total_col_width = 1.0;
    }

    /*
     * Create the buttons
     */
    for (i = 0; i < ctx->items; i++) {
        const char shortcut = va_arg(ap, int);

        int c;
        for (c = 0; c < cols; c++) {
            const char *text = va_arg(ap, const char*);

            widp b = wid_new_container(w, "wid menu button");
            ctx->buttons[i][c] = b;
            ctx->shortcut[i] = shortcut;

            wid_set_on_key_down(b, wid_menu_button_key_event);
            wid_set_on_mouse_down(b, wid_menu_button_mouse_event);
            wid_set_on_joy_down(b, wid_menu_button_joy_down_event);
            wid_set_on_mouse_over_begin(b, wid_menu_mouse_over);

            wid_set_client_context(b, ctx);
            wid_set_client_context2(b, (void*) (uintptr_t) i);
            wid_set_client_context3(b, (void*) (uintptr_t) c);

            if (text) {
                if (!strncmp(text, "bar:", 3)) {
                    widp bar = ctx->bar[i][c] = wid_new_square_button(b, "bar");

                    int val;
                    int max;
                    sscanf(text, "bar:%d,%d", &val, &max);

                    double p = ((double)val) / ((double)max);

                    ctx->bar_width[i][c] = p;

                    wid_set_on_key_down(bar, wid_menu_button_key_event);
                    wid_set_on_mouse_down(bar, wid_menu_button_mouse_event);
                    wid_set_on_joy_down(bar, wid_menu_button_joy_down_event);
                    wid_set_on_mouse_over_begin(bar, wid_menu_mouse_over);

                    wid_set_client_context(bar, ctx);
                    wid_set_client_context2(bar, (void*) (uintptr_t) i);
                    wid_set_client_context3(bar, (void*) (uintptr_t) c);

                } else {
                    wid_set_text(b, text);
                }
            }
        }

        on_mouse_down_t fn = va_arg(ap, on_mouse_down_t);

        for (c = 0; c < cols; c++) {
            ctx->event_handler[i] = fn;
        }
    }

    ctx->created = time_get_time_ms();

    va_end(ap);

    wid_move_to_pct_centered(wrapper, 0.5, 0.5);
    wid_move_to_pct_centered(w, x, y);

    wid_update(wrapper);
    wid_set_do_not_lower(wrapper, 1);
    wid_raise(wrapper);

    wid_menu_update(wrapper);
    wid_update(wrapper);
    wid_raise(wrapper);

    wid_set_focus(wrapper);
    wid_focus_lock(wrapper);

    return (wrapper);
}
