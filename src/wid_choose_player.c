/*
 * Copyright (C) 2011 Neil McGill
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "wid_intro.h"
#include "wid_choose_player.h"
#include "wid_choose_game_type.h"
#include "level.h"
#include "timer.h"
#include "time_util.h"
#include "client.h"
#include "thing.h"
#include "wid_notify.h"
#include "wid_player_stats.h"
#include "wid_player_info.h"
#include "wid_player_inventory.h"
#include "wid_player_action.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"
#include "name.h"
#include "glapi.h"

static widp wid_choose_player;
static widp wid_choose_player_background;

static void wid_choose_player_play_selected(void);

static uint8_t wid_choose_player_init_done;
static void wid_choose_player_create(void);

static int intro_effect_delay = 200;

uint8_t wid_choose_player_init (void)
{
    if (!wid_choose_player_init_done) {
    }

    wid_choose_player_init_done = true;

    return (true);
}

void wid_choose_player_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_choose_player_init_done) {
        wid_choose_player_init_done = false;

        if (wid_choose_player) {
            wid_destroy(&wid_choose_player);
            wid_destroy_in(wid_choose_player_background, wid_hide_delay * 2);
        }
    }
}

static uint8_t wid_choose_player_is_hidden;
static uint8_t wid_choose_player_is_visible;

void wid_choose_player_hide (void)
{
    if (wid_choose_player_is_hidden) {
        return;
    }

    wid_choose_player_is_hidden = true;
    wid_choose_player_is_visible = false;

    if (!wid_choose_player) {
        DIE("no wid intro");
    }

    wid_fade_out(wid_choose_player_background, intro_effect_delay);

    wid_hide(wid_choose_player, 0);
    wid_raise(wid_choose_player);
    wid_update(wid_choose_player);
    
    wid_player_stats_hide();
    wid_player_info_hide();
    wid_player_inventory_hide();
    wid_player_action_hide();

    thing_statsp s;
    s = &global_config.stats;

    /*
     * If no name was chosen by the user and they removed the default, then 
     * add one back.
     */
    if (!strlen(s->pname)) {
        strncpy(s->pname, name_random(s->pclass), sizeof(s->pname) - 1);
    }

    client_socket_set_name(s->pname);
    client_socket_set_pclass(s->pclass);
}

void wid_choose_player_visible (void)
{
    if (wid_choose_player_is_visible) {
        return;
    }

    wid_notify_flush();

    wid_choose_player_create();

    wid_choose_player_is_visible = true;
    wid_choose_player_is_hidden = false;

    if (!wid_choose_player) {
        DIE("no wid intro");
    }

    if (game_over) {
        game_over = false;
        return;
    }

    wid_visible(wid_choose_player, 0);
    wid_raise(wid_choose_player);
    wid_update(wid_choose_player);

    wid_fade_in(wid_choose_player_background, intro_effect_delay);

    /*
     * Start with some random junk.
     */
    thing_statsp s;
    s = &global_config.stats;
    thing_stats_get_random(s, false /* new_random_name_and_class */);

    wid_player_stats_visible(s);
    wid_player_info_visible(s);
    wid_player_inventory_visible(s);
    wid_player_action_visible(s, false);
}

static void wid_choose_player_play_selected_cb (void *context)
{
    wid_game_map_server_visible();
    wid_game_map_client_visible();

    wid_intro_hide();
}

static void wid_choose_player_play_selected (void)
{
    LOG("Play from stats selection");

    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_choose_player_play_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start game",
            intro_effect_delay,
            0 /* jitter */);

    wid_choose_player_hide();
}

static uint8_t wid_choose_player_play_mouse_event (widp w, int32_t x, int32_t y,
                                           uint32_t button)
{
    wid_choose_player_play_selected();

    return (true);
}

static void wid_intro_intro_all_done_selected (void)
{
    LOG("All done on stats selection");

    wid_choose_player_play_selected();
}

static uint8_t wid_intro_go_back_selected (void)
{
    LOG("Go back from intro");

    wid_choose_player_hide();
    wid_intro_visible();

    return (true);
}

static uint8_t wid_choose_player_go_back_mouse_event (widp w, int32_t x, int32_t y,
                                               uint32_t button)
{
    wid_intro_go_back_selected();

    return (true);
}

static uint8_t wid_choose_player_play_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case ' ':
            wid_choose_player_play_selected();
            return (true);

        case 'b':
        case 'q':
        case SDLK_ESCAPE:
            wid_intro_go_back_selected();
            return (true);

        case 'a':
            wid_intro_intro_all_done_selected();
            return (true);

        default:
            break;
    }

    return (false);
}

static void wid_choose_player_bg_create (void)
{
    widp wid;

    if (wid_choose_player_background) {
        return;
    }

    {
        wid = wid_choose_player_background = wid_new_window("bg");

        float f;

        f = (1024.0 / 680.0);

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, f };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title2");

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

static uint8_t wid_choose_player_intro_all_done_mouse_event (widp w, 
                                                      int32_t x, int32_t y,
                                                      uint32_t button)
{
    wid_intro_intro_all_done_selected();

    return (true);
}

static void wid_choose_player_create (void)
{
    if (wid_choose_player) {
        return;
    }

    wid_choose_player = wid_new_window("intro buttons");

    wid_set_no_shape(wid_choose_player);

    fpoint tl = {0.0f, 0.0f};
    fpoint br = {1.0f, 1.0f};
    wid_set_tl_br_pct(wid_choose_player, tl, br);
    wid_set_on_mouse_down(wid_choose_player, wid_choose_player_play_mouse_event);
    wid_set_on_key_down(wid_choose_player, wid_choose_player_play_key_event);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(wid_choose_player, WID_MODE_NORMAL);
    wid_set_color(wid_choose_player, WID_COLOR_TL, col);
    wid_set_color(wid_choose_player, WID_COLOR_BR, col);
    wid_set_color(wid_choose_player, WID_COLOR_BG, col);
    wid_set_on_mouse_down(wid_choose_player, wid_choose_player_play_mouse_event);
    wid_set_on_key_down(wid_choose_player, wid_choose_player_play_key_event);

    {
        widp child;

        child = wid_new_square_button(wid_choose_player, "All done");
        wid_set_font(child, med_font);

        fpoint tl = {0.0f, 0.95f};
        fpoint br = {0.3f, 1.00f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "%%fmt=left$%%tile=button_a$All done");

        wid_set_no_shape(child);
        wid_set_color(child, WID_COLOR_TEXT, GRAY90);
        wid_set_mode(child, WID_MODE_OVER);
        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_choose_player_intro_all_done_mouse_event);
        wid_raise(child);
        wid_set_do_not_lower(child, true);

        wid_fade_in_out(child, 1000, 1000, false /* fade out first */);
    }

    {
        widp child;

        child = wid_new_square_button(wid_choose_player, "Go back");
        wid_set_font(child, med_font);

        fpoint tl = {0.85f, 0.95f};
        fpoint br = {1.0f, 1.00f};

        wid_set_tl_br_pct(child, tl, br);
        wid_set_text(child, "%%fmt=left$%%tile=button_b$Go back");

        wid_set_no_shape(child);
        wid_set_color(child, WID_COLOR_TEXT, GRAY90);
        wid_set_mode(child, WID_MODE_OVER);
        wid_set_color(child, WID_COLOR_TEXT, WHITE);
        wid_set_mode(child, WID_MODE_NORMAL);

        wid_set_on_mouse_down(child, wid_choose_player_go_back_mouse_event);
        wid_raise(child);
        wid_set_do_not_lower(child, true);
    }

    wid_choose_player_bg_create();
    wid_update(wid_choose_player);

    wid_move_to_pct_centered(wid_choose_player, 0.5f, 0.5f);
}
