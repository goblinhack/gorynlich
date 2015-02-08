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
} wid_menu_ctx;

static void wid_menu_update (widp w)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    int items = ctx->items;

    double total_height_needed = 
                    ctx->focus_height + 
                    ctx->normal_height * (items - 1);

    double fh = ctx->focus_height / total_height_needed;
    double nh = (ctx->normal_height * ((double)items - 1)) / 
                    total_height_needed;
    nh /= (double)(items - 1);

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
            y += fh;
            c = GREEN;
        } else {
            font = ctx->normal_font;
            y += nh;
            c = WHITE;
        }

        br.y = y;

        wid_set_tl_br_pct(b, tl, br);

        color transparent = BLACK;
        transparent.a = 50;

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

static uint8_t wid_menu_wid_key_event (widp w, const SDL_KEYSYM *key)
{
    wid_menu_ctx *ctx = wid_get_client_context(w);
    verify(ctx);

    switch (key->sym) {
        case SDLK_BACKSPACE:
            break;

        case SDLK_TAB:
            ctx->focus++;
            if (ctx->focus > ctx->items - 1) {
                ctx->focus = 0;
            }
            return (true);

        case ' ':
        case SDLK_RETURN:
            return (true);

        case SDLK_LEFT:
            break;

        case SDLK_RIGHT:
            break;

        case SDLK_UP:
            ctx->focus--;
            if (ctx->focus < 0) {
                ctx->focus = ctx->items - 1;
            }
            break;

        case SDLK_DOWN:
            ctx->focus++;
            if (ctx->focus > ctx->items - 1) {
                ctx->focus = 0;
            }
            break;

        case SDLK_HOME:
            ctx->focus = 0;
            break;

        case SDLK_END:
            ctx->focus = ctx->items - 1;
            break;
    }

    wid_menu_update(ctx->w);

    return (false);
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
    double total_height_needed = 
                    ctx->focus_height + 
                    ctx->normal_height * (items - 1);

    /*
     * Padding
     */
    ctx->focus_height *= padding;
    ctx->normal_height *= padding;

    /*
     * Create the button container
     */
    widp w = wid_new_container(parent, "wid menu");

    fpoint tl = { 0.0, 0.0};
    fpoint br = { 0.0, 0.0};
    br.x = global_config.video_gl_width;
    br.y = total_height_needed;
    wid_set_tl_br(w, tl, br);

    wid_set_no_shape(w);
    wid_set_on_key_down(w, wid_menu_wid_key_event);
    wid_move_to_pct_centered(w, 0.5, 0.5);
    wid_set_on_destroy(w, wid_menu_destroy);

    wid_set_client_context(w, ctx);
    ctx->items = items;
    ctx->focus = focus;
    ctx->w = w;
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
        wid_set_text(b, text);

        wid_set_on_mouse_down(b, fn);

    }

    va_end(ap);

    wid_menu_update(w);
    wid_set_do_not_lower(w, 1);
    wid_raise(w);

    return (w);
}
