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
#include "wid_menu.h"

#define WID_MENU_MAX_ITEMS 20

typedef struct {
    widp w;
    int focus;
    int items;
    fontp focus_font;
    fontp normal_font;
    double focus_height;
    double normal_height;
    widp buttons[WID_MENU_MAX_ITEMS];
    on_mouse_down_t event_handler[WID_MENU_MAX_ITEMS];
} wid_menu_ctx;

static void wid_menu_destroy(widp w);

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

        widp b = ctx->buttons[i];
        verify(b);

        wid_set_text_centerx(b, 1);
        wid_set_text_centery(b, 1);

        fpoint tl = { 0.0, 0.0};
        fpoint br = { 1.0, 1.0};

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
                c = WHITE;
            } else {
                c = GRAY;
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
    }

    wid_update(w);
}

static uint8_t wid_menu_mouse_event (widp w,
                                     int focus,
                                     int32_t x, int32_t y,
                                     uint32_t button)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    widp b = ctx->buttons[ctx->focus];
    verify(b);

    on_mouse_down_t event_handler = ctx->event_handler[ctx->focus];

    (event_handler)(b, mouse_x, mouse_y, SDL_BUTTON_LEFT);

    return (true);
}

static uint8_t wid_menu_button_mouse_event (widp w,
                                            int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    int focus = (typeof(focus)) (uintptr_t) wid_get_client_context2(w);

    return (wid_menu_mouse_event(w, focus, x, y, button));
}

static uint8_t wid_menu_parent_mouse_event (widp w,
                                            int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    int focus = ctx->focus;

    return (wid_menu_mouse_event(w, focus, x, y, button));
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

static uint8_t wid_menu_key_event (widp w, int focus, const SDL_KEYSYM *key)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

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
                widp b = ctx->buttons[ctx->focus];
                verify(b);

                on_mouse_down_t event_handler = 
                                ctx->event_handler[ctx->focus];

                if (event_handler) {
                    (event_handler)(b, mouse_x, mouse_y, SDL_BUTTON_LEFT);
                }
                return (true);
            }

        case SDLK_LEFT:
            break;

        case SDLK_RIGHT:
            break;

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

        default:
            return (false);
    }

    return (true);
}

static uint8_t wid_menu_button_key_event (widp w, const SDL_KEYSYM *key)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    int focus = (typeof(focus)) (uintptr_t) wid_get_client_context2(w);

    switch (key->sym) {
        case SDLK_TAB:
        case SDLK_BACKSPACE:
        case SDLK_ESCAPE:
        case ' ':
        case SDLK_RETURN:
        case SDLK_LEFT:
        case SDLK_RIGHT:
        case SDLK_UP:
        case SDLK_DOWN:
        case SDLK_HOME:
        case SDLK_END:
            return (wid_menu_key_event(w, focus, key));

        default:
            return (false);
    }
}

static uint8_t wid_menu_parent_key_event (widp w, const SDL_KEYSYM *key)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    int focus = ctx->focus;

    switch (key->sym) {
        case SDLK_TAB:
        case SDLK_BACKSPACE:
        case SDLK_ESCAPE:
        case ' ':
        case SDLK_RETURN:
        case SDLK_LEFT:
        case SDLK_RIGHT:
        case SDLK_UP:
        case SDLK_DOWN:
        case SDLK_HOME:
        case SDLK_END:
            return (wid_menu_key_event(w, focus, key));

        default:
            return (false);
    }
}

static void wid_menu_mouse_over (widp w)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    int focus = (typeof(focus)) (uintptr_t) wid_get_client_context2(w);

    verify(ctx);

    ctx->focus = focus;

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
}

widp wid_menu (widp parent,
               fontp focus_font,
               fontp normal_font,
               double padding,
               int focus,
               int items, ...)
{
    if (items >= WID_MENU_MAX_ITEMS) {
        DIE("too many menu items");
    }

    /*
     * Create a context to hold button info so we can update it when the focus 
     * changes
     */
    wid_menu_ctx *ctx = myzalloc(sizeof(*ctx), "wid menu");

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
        color c = RED;
        c.a = 100;
            wid_set_color(wrapper, WID_COLOR_BG, c);
            wid_set_color(wrapper, WID_COLOR_TL, c);
            wid_set_color(wrapper, WID_COLOR_BR, c);

//        wid_set_no_shape(wrapper);
        wid_set_on_destroy(wrapper, wid_menu_destroy);
        wid_set_client_context(wrapper, ctx);
        wid_set_on_key_down(wrapper, wid_menu_parent_key_event);
        wid_set_on_mouse_down(wrapper, wid_menu_parent_mouse_event);
        ctx->w = wrapper;
    }

    /*
     * Create the button container
     */
    widp w = wid_new_container(wrapper, "wid menu");

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
        br.y = total_height_needed * 1.1;

        wid_set_tl_br(w, tl, br);
        wid_move_to_pct_centered(w, 0.5, 0.5);
    }

    wid_set_client_context(w, ctx);
    ctx->items = items;
    ctx->focus = focus;
    ctx->focus_font = focus_font;
    ctx->normal_font = normal_font;

    va_list ap;
    int i;
    va_start(ap, items);

    /*
     * Create the buttons
     */
    for (i = 0; i < ctx->items; i++) {
        const char *text = va_arg(ap, const char*);
        on_mouse_down_t fn = va_arg(ap, on_mouse_down_t);

        widp b = wid_new_container(w, "wid menu button");
        ctx->buttons[i] = b;
        ctx->event_handler[i] = fn;

        wid_set_text(b, text);

        wid_set_on_mouse_down(b, fn);
        wid_set_on_mouse_over_begin(b, wid_menu_mouse_over);
        wid_set_on_key_down(b, wid_menu_button_key_event);
        wid_set_on_mouse_down(b, wid_menu_button_mouse_event);

        wid_set_client_context(b, ctx);
        wid_set_client_context2(b, (void*) (uintptr_t) i);
    }

    va_end(ap);

    wid_move_to_pct_centered(wrapper, 0.5, 0.5);
    wid_menu_update(wrapper);
    wid_set_do_not_lower(wrapper, 1);
    wid_raise(wrapper);

    return (wrapper);
}
