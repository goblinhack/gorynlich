/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */


#include "wid.h"
#include "wid_choose_name.h"
#include "wid_choose_pclass.h"
#include "wid_choose_stats.h"
#include "wid_intro.h"
#include "wid_server_join.h"
#include "wid_keyboard.h"
#include "wid_server_create.h"
#include "timer.h"
#include "glapi.h"
#include "server.h"
#include "wid_menu.h"
#include "name.h"

static widp wid_choose_name;
static widp wid_choose_name_background;

static uint8_t wid_choose_name_init_done;
static void wid_choose_name_create(void);
static widp menu_keyboard;

static int intro_effect_delay = 200;

uint8_t wid_choose_name_init (void)
{
    if (!wid_choose_name_init_done) {
    }

    wid_choose_name_init_done = true;

    return (true);
}

void wid_choose_name_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_choose_name_init_done) {
        wid_choose_name_init_done = false;

        if (wid_choose_name) {
            wid_destroy(&wid_choose_name);
            wid_destroy_in(wid_choose_name_background, wid_hide_delay * 2);
            wid_choose_name_background = 0;
        }

        if (menu_keyboard) {
            wid_destroy(&menu_keyboard);
        }
    }
}

static uint8_t wid_choose_name_is_hidden;
static uint8_t wid_choose_name_is_visible;

void wid_choose_name_hide (void)
{
    if (wid_choose_name_is_hidden) {
        return;
    }

    wid_choose_name_is_hidden = true;
    wid_choose_name_is_visible = false;

    if (!wid_choose_name) {
        return;
    }

    wid_fade_out(wid_choose_name_background, intro_effect_delay);

    wid_hide(wid_choose_name, 0);
    wid_raise(wid_choose_name);
    wid_update(wid_choose_name);

    wid_destroy(&wid_choose_name);
    wid_destroy_in(wid_choose_name_background, wid_hide_delay * 2);
    wid_choose_name_background = 0;

    if (menu_keyboard) {
        wid_destroy(&menu_keyboard);
    }
}

void wid_choose_name_visible (void)
{
    if (wid_choose_name_is_visible) {
        return;
    }

    wid_choose_name_create();

    wid_choose_name_is_visible = true;
    wid_choose_name_is_hidden = false;

    if (!wid_choose_name) {
        ERR("no wid intro");
    }

    if (global_config.game_over) {
        global_config.game_over = false;
        return;
    }

    wid_visible(wid_choose_name, 0);
    wid_raise(wid_choose_name);
    wid_update(wid_choose_name);

    wid_fade_in(wid_choose_name_background, intro_effect_delay);
}

static void wid_keyboard_event_selected (widp w, const char *text)
{
    thing_statsp s = &global_config.stats;
    strlcpy(s->pname, text, sizeof(s->pname) - 1);

    wid_choose_name_hide();
    wid_choose_stats_visible();
}

static void wid_keyboard_event_cancelled (widp w, const char *text)
{
    wid_choose_name_hide();
    wid_choose_pclass_visible();
}

static uint8_t wid_choose_name_play_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case SDLK_ESCAPE:
            wid_choose_name_hide();
            wid_choose_pclass_visible();
            return (true);

        default:
            break;
    }

    return (false);
}

static void wid_choose_name_bg_create (void)
{
    widp wid;

    if (wid_choose_name_background) {
        return;
    }

    {
        wid = wid_choose_name_background = wid_new_window("bg");

        float f = (1024.0 / 680.0);

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, f };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title4");

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

static void wid_choose_name_create (void)
{
    if (wid_choose_name) {
        return;
    }

    wid_choose_name = wid_new_window("intro buttons");

    wid_set_no_shape(wid_choose_name);

    fpoint tl = {0.0f, 0.0f};
    fpoint br = {1.0f, 1.0f};
    wid_set_tl_br_pct(wid_choose_name, tl, br);
    wid_set_on_key_down(wid_choose_name, wid_choose_name_play_key_event);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(wid_choose_name, WID_MODE_NORMAL);
    wid_set_color(wid_choose_name, WID_COLOR_TL, col);
    wid_set_color(wid_choose_name, WID_COLOR_BR, col);
    wid_set_color(wid_choose_name, WID_COLOR_BG, col);

    wid_choose_name_bg_create();
    wid_update(wid_choose_name);

    wid_move_to_pct_centered(wid_choose_name, 0.5f, 0.5f);

    /*
     * If there is an existing pclass then use that and set the focus to it.
     */
    thing_statsp s = &global_config.stats;
    if (!strlen(s->pname)) {
        strlcpy(s->pname, name_random(s->pclass), sizeof(s->pname) - 1);
    }

    /*
     * Start with some random junk.
     */
    thing_stats_get_random(s, false /* new_random_name_and_class */);

    menu_keyboard = wid_keyboard(s->pname, "Choose your title",
                                 wid_keyboard_event_selected,
                                 wid_keyboard_event_cancelled);
}
