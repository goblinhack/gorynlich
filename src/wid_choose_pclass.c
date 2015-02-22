/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */


#include "wid.h"
#include "wid_choose_name.h"
#include "wid_choose_pclass.h"
#include "wid_choose_player.h"
#include "wid_choose_game_type.h"
#include "wid_intro.h"
#include "wid_server_join.h"
#include "wid_server_create.h"
#include "timer.h"
#include "glapi.h"
#include "server.h"
#include "wid_menu.h"
#include "name.h"

static widp wid_choose_pclass;
static widp wid_choose_pclass_background;

static uint8_t wid_choose_pclass_init_done;
static void wid_choose_pclass_create(void);
static widp menu;

static int intro_effect_delay = 200;

uint8_t wid_choose_pclass_init (void)
{
    if (!wid_choose_pclass_init_done) {
    }

    wid_choose_pclass_init_done = true;

    return (true);
}

void wid_choose_pclass_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_choose_pclass_init_done) {
        wid_choose_pclass_init_done = false;

        if (wid_choose_pclass) {
            wid_destroy(&wid_choose_pclass);
            wid_destroy_in(wid_choose_pclass_background, wid_hide_delay * 2);
            wid_choose_pclass_background = 0;
        }

        if (menu) {
            wid_destroy(&menu);
        }
    }
}

static uint8_t wid_choose_pclass_is_hidden;
static uint8_t wid_choose_pclass_is_visible;

void wid_choose_pclass_hide (void)
{
    if (wid_choose_pclass_is_hidden) {
        return;
    }

    wid_choose_pclass_is_hidden = true;
    wid_choose_pclass_is_visible = false;

    if (!wid_choose_pclass) {
        return;
    }

    wid_fade_out(wid_choose_pclass_background, intro_effect_delay);

    wid_hide(wid_choose_pclass, 0);
    wid_raise(wid_choose_pclass);
    wid_update(wid_choose_pclass);

    wid_destroy(&wid_choose_pclass);
    wid_destroy_in(wid_choose_pclass_background, wid_hide_delay * 2);
    wid_choose_pclass_background = 0;

    if (menu) {
        wid_destroy(&menu);
    }
}

void wid_choose_pclass_visible (void)
{
    if (wid_choose_pclass_is_visible) {
        return;
    }

    wid_choose_pclass_create();

    wid_choose_pclass_is_visible = true;
    wid_choose_pclass_is_hidden = false;

    if (!wid_choose_pclass) {
        DIE("no wid intro");
    }

    if (global_config.game_over) {
        global_config.game_over = false;
        return;
    }

    wid_visible(wid_choose_pclass, 0);
    wid_raise(wid_choose_pclass);
    wid_update(wid_choose_pclass);

    wid_fade_in(wid_choose_pclass_background, intro_effect_delay);
}

static void wid_choose_pclass_callback (widp w)
{
    wid_menu_ctx *ctx = (typeof(ctx)) wid_get_client_context(w);
    verify(ctx);

    int32_t row = (typeof(row)) (intptr_t) wid_get_client_context2(w);

    /*
     * Retrieve the selected class.
     */
    thing_statsp s = &global_config.stats;
    const char *new_pclass = pclass_nth(row);

    if (strlen(s->pclass)) {
        if (!strcasecmp(s->pclass, new_pclass)) {
            /*
             * No change.
             */
        } else {
            /*
             * Changed; start with a new random name.
             */
            s->pname[0] = '\0';
        }
    } else {
        /*
         * New class.
         */
    }

    strncpy(s->pclass, new_pclass, sizeof(s->pclass) - 1);

    wid_choose_pclass_hide();
    wid_choose_name_visible();
}

static uint8_t 
wid_choose_pclass_go_back (void)
{
    wid_choose_pclass_hide();
    wid_intro_visible();

    return (true);
}

static uint8_t wid_choose_pclass_play_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case SDLK_ESCAPE:
            wid_choose_pclass_hide();
            wid_intro_visible();
            return (true);

        default:
            break;
    }

    return (false);
}

static void wid_choose_pclass_bg_create (void)
{
    widp wid;

    if (wid_choose_pclass_background) {
        return;
    }

    {
        wid = wid_choose_pclass_background = wid_new_window("bg");

        float f = (1024.0 / 680.0);

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, f };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title3");

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

static void wid_choose_pclass_create (void)
{
    if (wid_choose_pclass) {
        return;
    }

    wid_choose_pclass = wid_new_window("intro buttons");

    wid_set_no_shape(wid_choose_pclass);

    fpoint tl = {0.0f, 0.0f};
    fpoint br = {1.0f, 1.0f};
    wid_set_tl_br_pct(wid_choose_pclass, tl, br);
    wid_set_on_key_down(wid_choose_pclass, wid_choose_pclass_play_key_event);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(wid_choose_pclass, WID_MODE_NORMAL);
    wid_set_color(wid_choose_pclass, WID_COLOR_TL, col);
    wid_set_color(wid_choose_pclass, WID_COLOR_BR, col);
    wid_set_color(wid_choose_pclass, WID_COLOR_BG, col);

    wid_choose_pclass_bg_create();
    wid_update(wid_choose_pclass);

    wid_move_to_pct_centered(wid_choose_pclass, 0.5f, 0.5f);

    thing_statsp s;
    s = &global_config.stats;

    int focus;
    const char *current_pclass = s->pclass;
    int n = pclass_find(current_pclass);
    if (n != -1) {
        focus = n;
    } else {
        focus = pclass_count() / 2;
    }

    menu = wid_menu(0,
                 vvlarge_font,
                 large_font,
                 0.5, /* x */
                 0.7, /* y */
                 0.95, /* padding between buttons */
                 focus, /* focus */
                 pclass_count() + 1, /* items */

                 (int) 'z', "wizard",   wid_choose_pclass_callback,
                 (int) 'd', "dwarf",    wid_choose_pclass_callback,
                 (int) 'w', "warrior",  wid_choose_pclass_callback,
                 (int) 'v', "valkyrie", wid_choose_pclass_callback,
                 (int) 'e', "elf",      wid_choose_pclass_callback,
                 (int) 'b', "back",     wid_choose_pclass_go_back);
}
