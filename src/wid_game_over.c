/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include "wid.h"
#include "wid_game_over.h"
#include "wid_choose_stats.h"
#include "wid_choose_name.h"
#include "wid_intro.h"
#include "wid_server_join.h"
#include "wid_server_create.h"
#include "wid_game_map_server.h"
#include "wid_game_map_client.h"
#include "timer.h"
#include "glapi.h"
#include "server.h"
#include "wid_menu.h"
#include "thing_template.h"
#include "music.h"
#include "client.h"

static widp wid_game_over;
static widp wid_game_over_background;

static void wid_game_over_play_selected(void);

static uint8_t wid_game_over_init_done;
static void wid_game_over_create(void);
static widp menu;

static int intro_effect_delay = 200;

uint8_t wid_game_over_init (void)
{
    if (!wid_game_over_init_done) {
    }

    wid_game_over_init_done = true;

    return (true);
}

void wid_game_over_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_game_over_init_done) {
        wid_game_over_init_done = false;

        if (wid_game_over) {
            wid_destroy(&wid_game_over);
            wid_destroy_in(wid_game_over_background, wid_hide_delay * 2);
            wid_game_over_background = 0;
        }

        if (menu) {
            wid_destroy(&menu);
        }
    }
}

static uint8_t wid_game_over_is_hidden;
static uint8_t wid_game_over_is_visible;

void wid_game_over_hide (void)
{
    if (wid_game_over_is_hidden) {
        return;
    }

    wid_game_over_is_hidden = true;
    wid_game_over_is_visible = false;

    if (!wid_game_over) {
        ERR("no wid intro");
    }

    wid_fade_out(wid_game_over_background, intro_effect_delay);

    wid_hide(wid_game_over, 0);
    wid_raise(wid_game_over);
    wid_update(wid_game_over);

    wid_destroy(&wid_game_over);
    wid_destroy_in(wid_game_over_background, wid_hide_delay * 2);
    wid_game_over_background = 0;

    if (menu) {
        wid_destroy(&menu);
    }
}

void wid_game_over_visible (void)
{
    if (wid_game_over_is_visible) {
        wid_game_over_hide();
    }

    wid_game_over_create();

    wid_game_over_is_visible = true;
    wid_game_over_is_hidden = false;

    if (!wid_game_over) {
        ERR("no wid intro");
    }

    if (global_config.game_over) {
        global_config.game_over = false;
        return;
    }

    wid_visible(wid_game_over, 0);
    wid_raise(wid_game_over);
    wid_update(wid_game_over);

    wid_fade_in(wid_game_over_background, intro_effect_delay);
}

static void wid_server_create_selected (void)
{
    LOG("Server create selected");

    wid_server_create_visible();
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

    wid_game_over_hide();
}

static void wid_game_over_single_play_selected_cb (void *context)
{
    single_player_mode = 1;

    wid_server_join_hide();
    wid_server_create_hide();

    wid_choose_player_play_selected();
}

static void wid_game_over_start_server_selected_cb (void *context)
{
    single_player_mode = 0;

    wid_server_create_selected();
}

static void wid_game_over_start_server_selected (void)
{
    LOG("Start server selected");

    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_game_over_start_server_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start server",
            intro_effect_delay,
            0 /* jitter */);

    wid_destroy(&menu);
}

static void wid_game_over_play_selected (void)
{
    LOG("Single player selected");

    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_game_over_single_play_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start game",
            intro_effect_delay,
            0 /* jitter */);

    wid_game_over_hide();
}

static uint8_t wid_game_over_play_mouse_event (widp w, int32_t x, int32_t y,
                                           uint32_t button)
{
    wid_game_over_play_selected();

    return (true);
}

static uint8_t 
wid_game_over_go_back_mouse_event (widp w, int32_t x, int32_t y,
                                          uint32_t button)
{

    LOG("Client: Game over completed");

    client_socket_close(0, 0);
    wid_game_map_client_hide();
    wid_game_map_client_wid_destroy();

    wid_intro_visible();
    wid_game_over_hide();

    return (true);
}

static uint8_t wid_game_over_play_key_event (widp w, const SDL_KEYSYM *key)
{
    switch (key->sym) {
        case ' ':
        case 's':
            wid_game_over_play_selected();
            return (true);

        case 'm':
            wid_game_over_start_server_selected();
            return (true);

        case 'b':
        case 'q':
        case SDLK_ESCAPE:
            wid_game_over_hide();
            wid_intro_visible();
            return (true);

        default:
            break;
    }

    return (false);
}

static void wid_game_over_bg_create (void)
{
    widp wid;

    if (wid_game_over_background) {
        return;
    }

    {
        wid = wid_game_over_background = wid_new_window("bg");

        float f = (1024.0 / 880.0);

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, f };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title6");

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

static void wid_game_over_create (void)
{
    if (wid_game_over) {
        return;
    }

    wid_game_over = wid_new_window("intro buttons");

    wid_set_no_shape(wid_game_over);

    fpoint tl = {0.0f, 0.0f};
    fpoint br = {1.0f, 1.0f};
    wid_set_tl_br_pct(wid_game_over, tl, br);
    wid_set_on_mouse_down(wid_game_over, wid_game_over_play_mouse_event);
    wid_set_on_key_down(wid_game_over, wid_game_over_play_key_event);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(wid_game_over, WID_MODE_NORMAL);
    wid_set_color(wid_game_over, WID_COLOR_TL, col);
    wid_set_color(wid_game_over, WID_COLOR_BR, col);
    wid_set_color(wid_game_over, WID_COLOR_BG, col);

    wid_game_over_bg_create();
    wid_update(wid_game_over);

    wid_move_to_pct_centered(wid_game_over, 0.5f, 0.5f);

    music_play_game_over();

    menu = wid_menu(0,
                 vvlarge_font,
                 large_font,
                 0, // on_update
                 0.5, /* x */
                 0.4, /* y */
                 1, /* columns */
                 1, /* focus */
                 6, /* items */

                 0, "Game over!", 0,
                 0, "", 0,
                 0, "The golden cheese is returned", 0,
                 0, "Gorynlich is vanquished!", 0,
                 0, "", 0,

                 (int) 'b', "Back to main window",
                 wid_game_over_go_back_mouse_event);

    int i;

    for (i = 0; i < 500; i++) {
        fpoint tl;
        fpoint br;

        tl.x = 0.01 * ((float)(myrand() % 100));
        tl.y = 0.01 * ((float)(myrand() % 100));

        br.x = tl.x + 0.05;
        br.y = tl.y + 0.05;

        float dy = (float)(myrand() % 100) / 10.0;
        tl.y -= dy;
        br.y -= dy;

        widp child = wid_new_square_button(wid_game_over, "cheese");

        wid_set_tl_br_pct(child, tl, br);
        wid_set_mode(child, WID_MODE_NORMAL);
        wid_set_color(child, WID_COLOR_TEXT, RED);
        wid_set_color(child, WID_COLOR_TL, BLACK);
        wid_set_color(child, WID_COLOR_BG, BLACK);
        wid_set_color(child, WID_COLOR_BR, BLACK);
        wid_set_no_shape(child);

        wid_rotate_to_pct_in(child,
                             0.0, 
                             myrand() % 1000,
                             (myrand() % 5000) + 1000, 999);

        tpp thing_template;
        
        if ((myrand() % 10) < 9) {
            thing_template = tp_find("cheese");
        } else {
            thing_template = random_treasure(0);
        }

        wid_move_delta_pct_in(child, 0.0, 50.0, 200000);

        wid_set_thing_template(child, thing_template);
    }
}
