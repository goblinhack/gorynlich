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
#include "wid_numpad.h"
#include "time_util.h"
#include "timer.h"
#include "math_util.h"

/*
 * How keys appear on screen
 */
static const char* keys[WID_NUMPAD_DOWN][WID_NUMPAD_ACROSS] = {
  { "7", "8", "9", "DEL"    },
  { "4", "5", "6", "CLEAR"  },
  { "1", "2", "3", "CANCEL",},
  { " ", "0", ".", "OK",    },
};

/*
 * The real key behind the scenes
 */
static const char key_char[WID_NUMPAD_DOWN][WID_NUMPAD_ACROSS] = {
  { '7', '8', '9', ''  },
  { '4', '5', '6', ''  },
  { '1', '2', '3', '', },
  { ' ', '0', '.', '\n', },
};

int wid_numpad_visible;

static void wid_numpad_destroy(widp w);
static void wid_numpad_set_focus(wid_numpad_ctx *ctx,
                                   int focusx, int focusy);
static uint8_t wid_numpad_text_input_key_event(widp w, 
                                                 const SDL_KEYSYM *key);

static void wid_numpad_update_buttons (widp w)
{
    wid_numpad_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    double width = 1.0 / (double)(WID_NUMPAD_ACROSS + 1);
    double height = 1.0 / (double)(WID_NUMPAD_DOWN + 1);

    int x, y;

    ctx->b = 0;

    for (x = 0; x < WID_NUMPAD_ACROSS; x++) {
    for (y = 0; y < WID_NUMPAD_DOWN; y++) {

        widp b = ctx->buttons[y][x];
        verify(b);

        fpoint tl;
        fpoint br;
        fontp font;
        color c;

        tl.x = (double) x * width;
        tl.y = (double) y * height;
        tl.x += width / 2.0;
        tl.y += height / 2.0;

        br.x = tl.x;
        br.y = tl.y;
        br.x += width;
        br.y += height;

        const char *t = keys[y][x];

        double zoom = 0.005;
        if ((x == ctx->focusx) && (y == ctx->focusy)) {
            tl.x -= zoom;
            tl.y -= zoom;
            br.x += zoom * 2.0;
            br.y += zoom * 2.0;
            c = GREEN;

            if (strlen(t) > 1) {
                font = med_font;
            } else {
                font = vvlarge_font;
            }

            ctx->b = b;

            wid_raise(b);
        } else {
            if (strlen(t) > 1) {
                font = med_font;
            } else {
                font = vlarge_font;
            }
            wid_lower(b);

            c = GRAY70;
        }

        if (!strcasecmp(t, "ok")) {
            font = vvlarge_font;
        }

        wid_set_tl_br_pct(b, tl, br);
        wid_set_color(b, WID_COLOR_TEXT, c);
        wid_set_font(b, font);
    }
    }

    wid_update(w);

    /*
     * Zoom buttons in
     */
    if (ctx->is_new) {
        ctx->is_new = false;

        for (x = 0; x < WID_NUMPAD_ACROSS; x++) {
            for (y = 0; y < WID_NUMPAD_DOWN; y++) {

                const char *t = keys[y][x];
                widp b = ctx->buttons[y][x];

                /*
                 * Start on the ok button.
                 */
                if (!strcasecmp(t, "ok")) {
                    ctx->focusx = x;
                    ctx->focusy = y;
                    wid_mouse_warp(b);
                    continue;
                }

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

static void wid_numpad_event (widp w, int focusx, int focusy,
                                const SDL_KEYSYM *key)
{
    wid_numpad_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    const char *add;
    if ((focusx == -1) && (focusy == -1)) {
        add = 0;
    } else {
        add = keys[focusy][focusx];
    }

    if (key) {
        wid_receive_input(ctx->input, key);
    } else if (!strcasecmp(add, "OK")) {
        (ctx->selected)(ctx->w, wid_get_text(ctx->input));
    } else if (!strcasecmp(add, "CANCEL")) {
        (ctx->cancelled)(ctx->w, wid_get_text(ctx->input));
    } else if (!strcasecmp(add, "CLEAR")) {
        for (;;) {
            SDL_KEYSYM key = {0};
            key.sym = SDLK_BACKSPACE;

            const char *tmp = wid_get_text(ctx->input);
            if (!tmp || !strlen(tmp)) {
                break;
            }
            wid_receive_input(ctx->input, &key);
        }

    } else if (!strcasecmp(add, "DEL")) {
        SDL_KEYSYM key = {0};
        key.sym = SDLK_BACKSPACE;
        wid_receive_input(ctx->input, &key);
    } else if (!strcasecmp(add, "SPACE")) {
        SDL_KEYSYM key = {0};
        key.sym = ' ';
        wid_receive_input(ctx->input, &key);
    } else {
        SDL_KEYSYM key = {0};
        key.sym = add[0];
        wid_receive_input(ctx->input, &key);
    }

    if (key && (focusx == -1) && (focusy == -1)) {
        int x, y;

        for (x = 0; x < WID_NUMPAD_ACROSS; x++) {
            for (y = 0; y < WID_NUMPAD_DOWN; y++) {
                char c = key_char[y][x];
                if (c == key->sym) {
                    focusx = x;
                    focusy = y;
                    break;
                }
            }

            if ((focusx != -1) && (focusy != -1)) {
                break;
            }
        }
    }

    if ((focusx != -1) && (focusy != -1)) {
        wid_numpad_set_focus(ctx, focusx, focusy);
    }
}

static uint8_t wid_numpad_mouse_event (widp w,
                                         int focusx, int focusy)
{
    wid_numpad_event(w, focusx, focusy, 0 /* key */);

    return (true);
}

static uint8_t wid_numpad_button_mouse_event (widp w,
                                                int32_t x, int32_t y,
                                                uint32_t button)
{
    int focus = (typeof(focus)) (uintptr_t) wid_get_client_context2(w);
    int focusx = (focus & 0xff);
    int focusy = (focus & 0xff00) >> 8;

    return (wid_numpad_mouse_event(w, focusx, focusy));
}

static void wid_numpad_focus_right (wid_numpad_ctx *ctx)
{
    ctx->focusx++;
    if (ctx->focusx > WID_NUMPAD_ACROSS - 1) {
        ctx->focusx = 0;
    }

    wid_numpad_update_buttons(ctx->w);
}

static void wid_numpad_focus_left (wid_numpad_ctx *ctx)
{
    ctx->focusx--;
    if (ctx->focusx < 0) {
        ctx->focusx = WID_NUMPAD_ACROSS - 1;
    }

    wid_numpad_update_buttons(ctx->w);
}

static void wid_numpad_focus_down (wid_numpad_ctx *ctx)
{
    ctx->focusy++;
    if (ctx->focusy > WID_NUMPAD_DOWN - 1) {
        ctx->focusy = 0;
    }

    wid_numpad_update_buttons(ctx->w);
}

static void wid_numpad_focus_up (wid_numpad_ctx *ctx)
{
    ctx->focusy--;
    if (ctx->focusy < 0) {
        ctx->focusy = WID_NUMPAD_DOWN - 1;
    }

    wid_numpad_update_buttons(ctx->w);
}

static void wid_numpad_last_focus (wid_numpad_ctx *ctx)
{
    ctx->focusx = WID_NUMPAD_ACROSS - 1;
    ctx->focusy = WID_NUMPAD_DOWN - 1;

    wid_numpad_update_buttons(ctx->w);
}

static void wid_numpad_first_focus (wid_numpad_ctx *ctx)
{
    ctx->focusx = 0;
    ctx->focusy = 0;

    wid_numpad_update_buttons(ctx->w);
}

static void wid_numpad_set_focus (wid_numpad_ctx *ctx, 
                                    int focusx, int focusy)
{
    ctx->focusx = focusx;
    ctx->focusy = focusy;

    wid_numpad_update_buttons(ctx->w);
}

static uint8_t wid_numpad_parent_key_down (widp w, 
                                             const SDL_KEYSYM *key)
{
    wid_numpad_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    switch (key->sym) {
        case '`':
            return (false);

        case SDLK_ESCAPE:
            (ctx->cancelled)(ctx->w, wid_get_text(ctx->input));
            return (true);

        case SDLK_RETURN: {
            (ctx->selected)(ctx->w, wid_get_text(ctx->input));
            return (true);

        case SDLK_LEFT:
            wid_numpad_focus_left(ctx);
            break;

        case SDLK_RIGHT:
            wid_numpad_focus_right(ctx);
            break;

        case SDLK_UP:
            wid_numpad_focus_up(ctx);
            break;

        case SDLK_DOWN:
            wid_numpad_focus_down(ctx);
            break;

        case SDLK_HOME:
            wid_numpad_first_focus(ctx);
            break;

        case SDLK_END:
            wid_numpad_last_focus(ctx);
            break;

        default:
            wid_numpad_event(ctx->w, -1, -1, key);
            return (true);
        }
    }

    return (true);
}

static uint8_t wid_numpad_parent_joy_button (widp w, 
                                               int32_t x,
                                               int32_t y)
{
    wid_numpad_ctx *ctx = wid_get_client_context(w);
    verify(ctx);
    int ret = false;

    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
        (ctx->selected)(ctx->w, wid_get_text(ctx->input));
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_B]) {
        (ctx->cancelled)(ctx->w, wid_get_text(ctx->input));
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_X]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_Y]) {
        (ctx->selected)(ctx->w, wid_get_text(ctx->input));
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_LEFT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_RIGHT]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_STICK_DOWN]) {
        (ctx->selected)(ctx->w, wid_get_text(ctx->input));
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_STICK_DOWN]) {
        (ctx->selected)(ctx->w, wid_get_text(ctx->input));
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_START]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_XBOX]) {
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_BACK]) {
        (ctx->cancelled)(ctx->w, wid_get_text(ctx->input));
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_UP]) {
        wid_numpad_focus_up(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_DOWN]) {
        wid_numpad_focus_down(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT]) {
        wid_numpad_focus_left(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT]) {
        wid_numpad_focus_right(ctx);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_FIRE]) {
        (ctx->selected)(ctx->w, wid_get_text(ctx->input));
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_FIRE]) {
        (ctx->selected)(ctx->w, wid_get_text(ctx->input));
        ret = true;
    }

    wid_numpad_update_buttons(ctx->w);
    if (ctx->b) {
        wid_mouse_warp(ctx->b);
    }

    return (ret);
}

static uint8_t wid_numpad_button_key_event (widp w, const SDL_KEYSYM *key)
{
    wid_numpad_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    switch (key->sym) {
        case '`':
            return (false);

        case SDLK_ESCAPE:
            (ctx->cancelled)(ctx->w, wid_get_text(ctx->input));
            return (true);

        case SDLK_RETURN:
            (ctx->selected)(ctx->w, wid_get_text(ctx->input));
            return (true);

        case SDLK_BACKSPACE:
        case SDLK_DELETE:
        case SDLK_LEFT:
        case SDLK_RIGHT:
        case SDLK_UP:
        case SDLK_DOWN:
        case SDLK_HOME:
        case SDLK_END:
            break;

        default:
            wid_numpad_event(w, -1, -1, key);
            return (true);
    }

    return (false);
}

static uint8_t wid_numpad_button_joy_down_event (widp w, 
                                                     int32_t x, int32_t y)
{
    wid_numpad_ctx *ctx = wid_get_client_context(w);
    verify(ctx);
    int ret = false;

    if (sdl_joy_buttons[SDL_JOY_BUTTON_A]) {
        ret = wid_numpad_mouse_event(w, ctx->focusx, ctx->focusy);
    }

    if (sdl_joy_buttons[SDL_JOY_BUTTON_B]) {
        (ctx->selected)(ctx->w, wid_get_text(ctx->input));
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_X]) {
        SDL_KEYSYM key = {0};
        key.sym = SDLK_BACKSPACE;
        wid_numpad_text_input_key_event(ctx->input, &key);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_Y]) {
        (ctx->selected)(ctx->w, wid_get_text(ctx->input));
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_LEFT]) {
        SDL_KEYSYM key = {0};
        key.sym = SDLK_LEFT;
        wid_numpad_text_input_key_event(ctx->input, &key);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_TOP_RIGHT]) {
        SDL_KEYSYM key = {0};
        key.sym = SDLK_RIGHT;
        wid_numpad_text_input_key_event(ctx->input, &key);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_LEFT_STICK_DOWN]) {
        wid_numpad_mouse_event(w, ctx->focusx, ctx->focusy);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_STICK_DOWN]) {
        wid_numpad_mouse_event(w, ctx->focusx, ctx->focusy);
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
        SDL_KEYSYM key = {0};
        key.sym = SDLK_BACKSPACE;
        wid_receive_input(ctx->input, &key);
        ret = true;
    }
    if (sdl_joy_buttons[SDL_JOY_BUTTON_RIGHT_FIRE]) {
        wid_numpad_mouse_event(w, ctx->focusx, ctx->focusy);
        ret = true;
    }

    wid_numpad_update_buttons(ctx->w);
    if (ctx->b) {
        wid_mouse_warp(ctx->b);
    }

    return (ret);
}

static uint8_t wid_numpad_text_input_key_event (widp w, const SDL_KEYSYM *key)
{
    wid_numpad_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    switch (key->sym) {
        case SDLK_ESCAPE:
            (ctx->cancelled)(ctx->w, wid_get_text(ctx->input));
            return (true);

        case SDLK_RETURN:
            (ctx->selected)(ctx->w, wid_get_text(ctx->input));
            return (true);

        case SDLK_BACKSPACE:
        case SDLK_DELETE:
        case SDLK_LEFT:
        case SDLK_RIGHT:
        case SDLK_UP:
        case SDLK_DOWN:
        case SDLK_HOME:
        case SDLK_END:
            wid_receive_input(ctx->input, key);
            break;

        default:
            wid_numpad_event(w, -1, -1, key);
            return (true);
    }

    return (false);
}

static void wid_numpad_mouse_over (widp w,
                                   int32_t relx, int32_t rely,
                                   int32_t wheelx, int32_t wheely)
{
    wid_numpad_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    if (!relx && !rely && !wheelx && !wheely) {
        return;
    }

    /*
     * If we recreate the numpad with a fixed focus we will be told about
     * a mouse over event immediately which may not be over the focus item
     * and will cause us to move. Annoying.
     */
    if (time_get_time_ms() - ctx->created < 100) {
        return;
    }

    int focus = (typeof(focus)) (uintptr_t) wid_get_client_context2(w);
    int focusx = (focus & 0xff);
    int focusy = (focus & 0xff00) >> 8;

    wid_numpad_set_focus(ctx, focusx, focusy);
}

static void wid_numpad_destroy (widp w)
{
    wid_numpad_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    wid_set_client_context(w, 0);
    myfree(ctx);

    wid_numpad_visible = false;
}

static void wid_numpad_tick (widp w)
{
    wid_numpad_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    static int val;
    static int delta = 1;
    static int step = 2;

    val += delta * step;

    if (val > 255) {
        val = 255;
        delta = -1;
    }

    if (val < 200) {
        val = 200;
        delta = 1;
    }

    int x, y;

    for (x = 0; x < WID_NUMPAD_ACROSS; x++) {
    for (y = 0; y < WID_NUMPAD_DOWN; y++) {

        if ((x != ctx->focusx) || (y != ctx->focusy)) {
            continue;
        }

        widp b = ctx->buttons[y][x];
        verify(b);

        color c;
        c = GREEN;
        c.g = val;

        /*
         * Make sure the other widgets look plain in all modes.
         */
        int mode;
        for (mode = WID_MODE_NORMAL; mode < WID_MODE_LAST; mode++) {
            wid_set_mode(b, mode);
            wid_set_color(b, WID_COLOR_TEXT, c);
            wid_set_text_outline(b, true);
        }

        wid_set_mode(w, WID_MODE_NORMAL);
    }
    }
}

static void wid_numpad_destroy_begin (widp w)
{
    wid_numpad_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    /*
     * Zoom buttons in
     */
    int x, y;

    for (x = 0; x < WID_NUMPAD_ACROSS; x++) {
        for (y = 0; y < WID_NUMPAD_DOWN; y++) {

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

widp wid_numpad (const char *text,
                   const char *title,
                   wid_numpad_event_t selected,
                   wid_numpad_event_t cancelled)
{
    wid_numpad_visible = true;

    /*
     * Create a context to hold button info so we can update it when the focus 
     * changes
     */
    wid_numpad_ctx *ctx = myzalloc(sizeof(*ctx), "wid numpad");
    ctx->focusx = -1;
    ctx->focusy = -1;
    ctx->cancelled = cancelled;
    ctx->selected = selected;

    widp window = wid_new_window("wid numpad");
    ctx->w = window;
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

        wid_set_on_destroy_begin(window, wid_numpad_destroy_begin);
        wid_set_on_key_down(window, wid_numpad_parent_key_down);
        wid_set_on_joy_down(window, wid_numpad_parent_joy_button);
        wid_set_on_destroy(window, wid_numpad_destroy);
        wid_set_client_context(window, ctx);
    }

    /*
     * Create the title
     */
    {
        widp w = wid_new_square_button(window, "wid numpad title");
        wid_set_no_shape(w);

        fpoint tl = { 0.0, 0.1};
        fpoint br = { 1.0, 0.2};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_text(w, title);
        wid_set_text_outline(w, true);
        wid_set_font(w, vvlarge_font);

        wid_set_color(w, WID_COLOR_TEXT, YELLOW);
    }

    /*
     * Create the text input container
     */
    {
        widp w = wid_new_square_button(window, "wid numpad input");

        ctx->input = w;

        fpoint tl = { 0.1, 0.2};
        fpoint br = { 0.9, 0.3};

        wid_set_tl_br_pct(w, tl, br);
        wid_set_text(w, text);
        wid_set_text_outline(w, true);
        wid_set_show_cursor(w, true);
        wid_set_on_key_down(w, wid_numpad_text_input_key_event);
        wid_set_font(w, vlarge_font);
        wid_set_client_context(w, ctx);

        wid_set_color(w, WID_COLOR_BG, GRAY20);
        wid_set_color(w, WID_COLOR_TL, GRAY60);
        wid_set_color(w, WID_COLOR_BR, GRAY10);
        wid_set_color(w, WID_COLOR_TEXT, GREEN);

        wid_set_mode(w, WID_MODE_OVER);

        wid_set_color(w, WID_COLOR_BG, GRAY20);
        wid_set_color(w, WID_COLOR_TL, GRAY70);
        wid_set_color(w, WID_COLOR_BR, GRAY10);
        wid_set_color(w, WID_COLOR_TEXT, GREEN);

        wid_set_mode(w, WID_MODE_NORMAL);
    }

    /*
     * Create the button container
     */
    {
        widp button_container = wid_new_square_button(window, 
                                                      "wid numpad buttons");
        wid_set_no_shape(button_container);
        wid_set_on_tick(button_container, wid_numpad_tick);

        fpoint tl = { 0.0, 0.35};
        fpoint br = { 1.0, 0.85};

        wid_set_tl_br_pct(button_container, tl, br);
        wid_set_client_context(button_container, ctx);

        /*
         * Create the buttons
         */
        int x, y;

        for (x = 0; x < WID_NUMPAD_ACROSS; x++) {
        for (y = 0; y < WID_NUMPAD_DOWN; y++) {
            widp b = wid_new_rounded_small_button(button_container, 
                                                  "wid numpad button");
            ctx->buttons[y][x] = b;

            wid_set_text(b, keys[y][x]);

            wid_set_text_outline(b, true);
            wid_set_on_mouse_over_begin(b, wid_numpad_mouse_over);
            wid_set_on_key_down(b, wid_numpad_button_key_event);
            wid_set_on_joy_down(b, wid_numpad_button_joy_down_event);
            wid_set_on_mouse_down(b, wid_numpad_button_mouse_event);

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

    wid_numpad_update_buttons(window);
    wid_set_do_not_lower(window, 1);
    wid_update(window);
    wid_raise(window);
    wid_numpad_update_buttons(window);
    wid_set_do_not_lower(window, 1);
    wid_update(window);
    wid_raise(window);

    ctx->created = time_get_time_ms();

    return (window);
}
