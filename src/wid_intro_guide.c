/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "tex.h"
#include "color.h"
#include "wid_intro.h"
#include "wid_intro_guide.h"

static widp wid_intro_guide;
static uint8_t wid_intro_guide_init_done;
static void wid_intro_guide_create(void);
static void wid_intro_guide_destroy(void);
static void wid_intro_guide_finished(widp wid);

uint8_t wid_intro_guide_init (void)
{
    if (!wid_intro_guide_init_done) {
    }

    wid_intro_guide_init_done = true;

    return (true);
}

void wid_intro_guide_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_intro_guide_init_done) {
        wid_intro_guide_init_done = false;

        wid_intro_guide_destroy();
    }
}

void wid_intro_guide_hide (void)
{
    wid_intro_guide_destroy();
}

void wid_intro_guide_visible (void)
{
    wid_intro_guide_create();
}

static void wid_intro_guide_destroy (void)
{
    wid_destroy(&wid_intro_guide);
}

static uint8_t wid_intro_guide_common (widp w)
{
    widp top = wid_get_top_parent(w);
    wid_destroy_in(top, 500);
    wid_set_on_destroy(top, wid_intro_guide_finished);

    return (true);
}

static uint8_t wid_intro_guide_key_down (widp w,
                                        const SDL_KEYSYM *key)
{
    return (wid_intro_guide_common(w));
}

static uint8_t wid_intro_guide_mouse_down (widp w,
                                          int32_t x,
                                          int32_t y,
                                          uint32_t button)
{
    return (wid_intro_guide_common(w));
}

static void wid_intro_guide_finished (widp wid)
{
    if (sdl_is_exiting()) {
        return;
    }

    wid_intro_guide_hide();

    wid_intro_visible();
}

static void wid_intro_guide_begin_3 (widp wid)
{
    if (sdl_is_exiting()) {
        return;
    }

    widp w = wid_new_window("guide3");
    texp tex = tex_find("help3");
    uint32_t tw = tex_get_width(tex);
    uint32_t th = tex_get_height(tex);

    fpoint tl = { 0, 0 };
    fpoint br = { (double) tw, (double) th };

    wid_set_tl_br(w, tl, br);

    wid_set_tex(w, 0, "help3");

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, WHITE);
    wid_set_color(w, WID_COLOR_BR, WHITE);
    wid_set_color(w, WID_COLOR_BG, WHITE);

    wid_destroy_in(w, 5000);
    wid_set_on_destroy(w, wid_intro_guide_finished);
    wid_set_on_mouse_down(w, wid_intro_guide_mouse_down);
    wid_set_on_key_down(w, wid_intro_guide_key_down);
    wid_move_to_pct_centered(w, 0.5f, 0.5f);
    wid_fade_in(w, wid_visible_delay * 2);
    wid_raise(w);

    wid_update(w);
}

static void wid_intro_guide_begin_2 (widp wid)
{
    if (sdl_is_exiting()) {
        return;
    }

    widp w = wid_new_window("guide2");
    texp tex = tex_find("help2");
    uint32_t tw = tex_get_width(tex);
    uint32_t th = tex_get_height(tex);

    fpoint tl = { 0, 0 };
    fpoint br = { (double) tw, (double) th };

    wid_set_tl_br(w, tl, br);

    wid_set_tex(w, 0, "help2");

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, WHITE);
    wid_set_color(w, WID_COLOR_BR, WHITE);
    wid_set_color(w, WID_COLOR_BG, WHITE);

    wid_destroy_in(w, 5000);
    wid_set_on_destroy(w, wid_intro_guide_begin_3);
    wid_set_on_mouse_down(w, wid_intro_guide_mouse_down);
    wid_set_on_key_down(w, wid_intro_guide_key_down);
    wid_move_to_pct_centered(w, 0.5f, 0.5f);
    wid_fade_in(w, wid_visible_delay * 2);
    wid_raise(w);

    wid_update(w);
}

void wid_intro_guide_create (void)
{
    if (sdl_is_exiting()) {
        return;
    }

    widp w = wid_new_window("guide1");
    texp tex = tex_find("help1");
    uint32_t tw = tex_get_width(tex);
    uint32_t th = tex_get_height(tex);

    fpoint tl = { 0, 0 };
    fpoint br = { (double) tw, (double) th };

    wid_set_tl_br(w, tl, br);

    wid_set_tex(w, 0, "help1");

    wid_set_mode(w, WID_MODE_NORMAL);
    wid_set_color(w, WID_COLOR_TL, WHITE);
    wid_set_color(w, WID_COLOR_BR, WHITE);
    wid_set_color(w, WID_COLOR_BG, WHITE);

    wid_destroy_in(w, 5000);
    wid_set_on_destroy(w, wid_intro_guide_begin_2);
    wid_set_on_mouse_down(w, wid_intro_guide_mouse_down);
    wid_set_on_key_down(w, wid_intro_guide_key_down);
    wid_move_to_pct_centered(w, 0.5f, 0.5f);
    wid_fade_in(w, wid_visible_delay * 2);
    wid_raise(w);

    wid_update(w);
}
