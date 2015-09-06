/*
 * Copyright (C) 2011-2015 goblinhack@gmail.com
 *
 * See the LICENSE file for license.
 */

#include <SDL.h>

#include "main.h"
#include "wid.h"
#include "wid_intro.h"
#include "wid_popup.h"
#include "wid_intro_about.h"
#include "wid_intro_settings.h"
#include "wid_menu.h"
#include "wid_game_over.h"
#include "wid_game_map_client.h"
#include "wid_choose_game_type.h"
#include "wid_hiscore.h"
#include "wid_notify.h"
#include "wid_server_join.h"
#include "wid_choose_pclass.h"
#include "wid_server_create.h"
#include "thing_template.h"
#include "music.h"
#include "level.h"
#include "timer.h"
#include "socket_util.h"
#include "server.h"
#include "glapi.h"
#include "wid_keyboard.h"
#include "thing_template.h"
#include "wid_editor.h"
#include "wid_map.h"
#include "wid_cmap.h"
#include "wid_tooltip.h"

static widp wid_intro;
static widp wid_intro_menu;
static widp wid_intro_background;
static widp wid_intro_title;
static widp wid_intro_man;
static widp wid_intro_treasure_chest;
static widp wid_intro_eyes;

static uint8_t wid_intro_is_hidden;
static uint8_t wid_intro_is_visible;
static uint8_t wid_intro_init_done;

static void wid_intro_quit_selected(void);
static void wid_intro_create(void);
static void wid_intro_menu_create(void);
static void wid_intro_menu_destroy(void);

static int intro_effect_delay = 500;
static int intro_effect_delay_zoom = 1000;

static int saved_focus = 3;

uint8_t wid_intro_init (void)
{
    if (!wid_intro_init_done) {
        wid_intro_create();
    }

    wid_intro_init_done = true;

    return (true);
}

void wid_intro_fini (void)
{
    FINI_LOG("%s", __FUNCTION__);

    if (wid_intro_init_done) {
        wid_intro_init_done = false;

        if (wid_intro) {
            wid_destroy(&wid_intro);
            wid_intro_menu_destroy();
            wid_destroy_in(wid_intro_background, wid_hide_delay * 2);
            wid_destroy_in(wid_intro_title, wid_hide_delay * 2);
            wid_destroy_in(wid_intro_man, wid_hide_delay * 2);
            wid_destroy_in(wid_intro_treasure_chest, wid_hide_delay * 2);
            wid_destroy_in(wid_intro_eyes, wid_hide_delay * 2);
        }
    }
}

void wid_intro_hide (void)
{
    if (wid_intro_is_hidden) {
        return;
    }

    LOG("Client: Intro screen hide");

    wid_intro_is_hidden = true;
    wid_intro_is_visible = false;

    music_halt();

    if (!wid_intro) {
        return;
    }

    wid_scaling_to_pct_in(wid_intro_background, 1.0, 10.01, 
                          intro_effect_delay_zoom, 0);
    wid_fade_out(wid_intro_background, intro_effect_delay_zoom);

    wid_scaling_to_pct_in(wid_intro_title, 1.0, 5.01, 
                          intro_effect_delay_zoom, 0);
    wid_fade_out(wid_intro_title, intro_effect_delay_zoom);
    wid_move_delta_pct_in(wid_intro_title, -2.0f, -4.0f,
                          intro_effect_delay_zoom);

    wid_fade_out(wid_intro_man, intro_effect_delay_zoom);
    wid_move_delta_pct_in(wid_intro_man, -2.0f, -0.0f,
                          intro_effect_delay_zoom);

    wid_fade_out(wid_intro_treasure_chest, intro_effect_delay_zoom);
    wid_move_delta_pct_in(wid_intro_treasure_chest, 2.0f, -0.0f,
                          intro_effect_delay_zoom);

    wid_fade_out(wid_intro_eyes, intro_effect_delay_zoom);

    wid_hide(wid_intro, 0);
    wid_raise(wid_intro);
    wid_update(wid_intro);

    wid_destroy_in(wid_intro, wid_hide_delay * 2);

    wid_intro_menu_destroy();
    wid_destroy_in(wid_intro_background, wid_hide_delay * 2);
    wid_destroy_in(wid_intro_title, wid_hide_delay * 2);
    wid_destroy_in(wid_intro_man, wid_hide_delay * 2);
    wid_destroy_in(wid_intro_treasure_chest, wid_hide_delay * 2);
    wid_destroy_in(wid_intro_eyes, wid_hide_delay * 2);

    wid_intro = 0;
    wid_intro_background = 0;
    wid_intro_title = 0;
    wid_intro_man = 0;
    wid_intro_treasure_chest = 0;
    wid_intro_eyes = 0;
}

void wid_intro_visible (void)
{
    music_halt();

    if (!wid_intro || wid_intro_is_visible) {
        /*
         * Make sure the menu is visible.
         */
        wid_intro_menu_create();
    }

    wid_intro_is_visible = true;
    wid_intro_is_hidden = false;

    wid_intro_create();
    wid_intro_menu_create();

    wid_notify_flush();

    LOG("Client: Intro screen show");

    music_play_intro();

    if (global_config.game_over) {
        global_config.game_over = false;
        wid_game_over_visible();
        return;
    }

    if (!wid_intro) {
        return;
    }

    wid_visible(wid_intro, 0);
    wid_raise(wid_intro);
    wid_update(wid_intro);

    wid_fade_in(wid_intro_background, intro_effect_delay);
    wid_fade_in(wid_intro_title, intro_effect_delay);
    wid_fade_in(wid_intro_man, intro_effect_delay);
    wid_fade_in(wid_intro_treasure_chest, intro_effect_delay);
    wid_fade_in(wid_intro_eyes, intro_effect_delay);
}

static void wid_intro_single_play_selected_cb (void *context)
{
    LOG("Play selected callback");

    on_server = true;
    single_player_mode = 1;

    wid_server_join_hide();
    wid_server_create_hide();

    /*
     * Let the server map load itself when the client connects.
    wid_game_map_server_visible();
     */
    server_start(server_address);

    wid_game_map_client_visible();

    wid_intro_hide();
}

void wid_intro_single_play_selected (level_pos_t level_pos)
{
    LOG("Client: Play selected, choose random stats, start at level %d.%d",
        level_pos.y, level_pos.x);

    /*
     * Start with some random junk.
     */
    thing_statsp s;
    s = &global_config.stats;
    thing_stats_get_random(s, false /* new_random_name_and_class */);

    global_config.stats.level_pos = level_pos;

    action_timer_create(
            &wid_timers,
            (action_timer_callback)wid_intro_single_play_selected_cb,
            (action_timer_destroy_callback)0,
            0, /* context */
            "start game",
            intro_effect_delay,
            0 /* jitter */);

    wid_intro_hide();
}

static double y;
static double dy;

static void wid_intro_tick_reset (void)
{
    y = -0.20;
    dy = 0.00026;
}

static void wid_intro_tick (widp wid)
{
    if (!wid_intro_title) {
        return;
    }

    wid_move_to_pct_centered(wid_intro_title, 0.5f, y);

    y += dy;

    static const double wall_start = 0.38;
    static const double accell_down = 1.06;
    static const double friction_up = 0.9;
    static const double elasticity = 0.09;

    if (y > wall_start) {
        y = wall_start;
        dy = -dy * elasticity;
        y += dy;

        wid_rotate_to_pct_in(wid, 0.1, 1.2, 300, 1);
    }

    if (dy < 0) {
        dy *= friction_up;

        if (dy > -0.0001) {
            dy = 0.0001;
        }
    } else {
        dy *= accell_down;
    }
}

static void wid_intro_bg_create (void)
{
    if (!wid_intro_title) {
        widp wid = wid_intro_title = wid_new_window("bg");
        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, 0.76 };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "main_title");

        wid_lower(wid);

        color c;
        c = WHITE;
        wid_set_mode(wid, WID_MODE_NORMAL);
        wid_set_color(wid, WID_COLOR_TL, c);
        wid_set_color(wid, WID_COLOR_BR, c);
        wid_set_color(wid, WID_COLOR_BG, c);

        wid_update(wid);
        wid_move_to_pct_centered(wid_intro_title, 0.5f, -4.1f);
        wid_set_on_tick(wid, wid_intro_tick);
        wid_intro_tick_reset();
    }

    if (!wid_intro_man) {
        widp wid = wid_intro_man = wid_new_window("bg");

        fpoint tl = { 0.0, 0.36 };
        fpoint br = { 0.2, 1.0 };
        double x = 0.08;

        if (!global_config.stats.pclass[0]) {
            wid_set_tex(wid, 0, "player1");
        } else {
            if (!strcmp("warrior", global_config.stats.pclass)) {
                wid_set_tex(wid, 0, "player1");
            } else if (!strcmp("dwarf", global_config.stats.pclass)) {
                wid_set_tex(wid, 0, "player2");
            } else if (!strcmp("valkyrie", global_config.stats.pclass)) {
                wid_set_tex(wid, 0, "player3");
            } else if (!strcmp("elf", global_config.stats.pclass)) {
                tl.y = 0.1;
                br.y = 0.9;
                wid_set_tex(wid, 0, "player4");
            } else if (!strcmp("wizard", global_config.stats.pclass)) {
                br.x = 0.4;
                x = 0.20;
                wid_set_tex(wid, 0, "player5");
            } else {
                wid_set_tex(wid, 0, "player1");
            }
        }

        wid_raise(wid);

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_mode(wid, WID_MODE_NORMAL);

        color c;
        c = WHITE;
        wid_set_mode(wid, WID_MODE_NORMAL);
        wid_set_color(wid, WID_COLOR_TL, c);
        wid_set_color(wid, WID_COLOR_BR, c);
        wid_set_color(wid, WID_COLOR_BG, c);

        wid_update(wid);
        wid_move_to_pct_centered(wid_intro_man, -0.8f, 0.70f);
        wid_move_to_pct_centered_in(wid_intro_man, x, 0.70f, 1000);
    }

    if (!wid_intro_treasure_chest) {
        widp wid = wid_intro_treasure_chest = wid_new_window("bg");

        fpoint tl = { 0.0, 0.56 };
        fpoint br = { 0.3, 1.0 };

        wid_set_tl_br_pct(wid, tl, br);

        tpp tp = tp_find("data/things/anim_treasure_chest");
        wid_set_thing_template(wid, tp);

        wid_raise(wid);

        wid_set_mode(wid, WID_MODE_NORMAL);

        wid_update(wid);
        wid_move_to_pct_centered(wid_intro_treasure_chest, 0.96f, 0.75f);
    }

    if (!wid_intro_eyes) {
        widp wid = wid_intro_eyes = wid_new_window("bg");

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 0.1, 0.12 };

        wid_set_tl_br_pct(wid, tl, br);

        tpp tp = tp_find("data/things/anim_eyes");
        wid_set_thing_template(wid, tp);

        wid_raise(wid);

        wid_set_mode(wid, WID_MODE_NORMAL);

        wid_update(wid);
        wid_move_to_pct_centered(wid_intro_eyes, 0.53f, 0.74f);
    }

    if (!wid_intro_background) {
        widp wid = wid_intro_background = wid_new_window("bg");

        float f;

        f = (1024.0 / 680.0);

        fpoint tl = { 0.0, 0.0 };
        fpoint br = { 1.0, f };

        wid_set_tl_br_pct(wid, tl, br);

        wid_set_tex(wid, 0, "title");

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

static widp wid_intro_quit_popup;

static void wid_intro_quit_callback_yes (widp wid)
{
    wid_destroy(&wid_intro_quit_popup);

    FINI_LOG("quit yes selected");
    sdl_exit();
}

static void wid_intro_quit_callback_no (widp wid)
{
    wid_destroy(&wid_intro_quit_popup);

    wid_intro_visible();
}

static void wid_intro_quit_selected (void)
{
    LOG("Intro quit selected");

    if (wid_intro_quit_popup) {
        return;
    }

    wid_intro_menu_destroy();

    wid_intro_quit_popup = 
        wid_menu(0,
                 vvlarge_font,
                 large_font,
                 0,
                 0.5, /* x */
                 0.5, /* y */
                 1, /* columns */
                 2, /* focus */
                 3, /* items */

                 (int) 0, "Quit game?", (void*)0,

                 (int) 'y', "Yes", wid_intro_quit_callback_yes,

                 (int) 'n', "No",  wid_intro_quit_callback_no);
}

static uint8_t wid_menu_settings_selected (widp w, 
                                           int32_t x, int32_t y,
                                           uint32_t button)
{
    wid_intro_menu_destroy();
    wid_intro_settings_visible();

    return (true);
}

static uint8_t wid_menu_level_editor_selected (widp w,
                                               int32_t x, int32_t y,
                                               uint32_t button)
{
    wid_intro_hide();
    wid_map("Choose epic level", 0, 0);

    return (true);
}

static uint8_t wid_menu_play_game_selected (widp w,
                                            int32_t x, int32_t y,
                                            uint32_t button)
{
    wid_intro_hide();
    wid_choose_pclass_visible();

    return (true);
}

static uint8_t wid_menu_quick_start_selected (widp w,
                                              int32_t x, int32_t y,
                                              uint32_t button)
{
    wid_intro_menu_destroy();

    level_pos_t level_pos;
    level_pos.x = (myrand() % LEVEL_INITIAL_RANDOM) + 1;
    level_pos.y = 1;

    wid_intro_single_play_selected(level_pos);

    return (true);
}

static uint8_t wid_menu_past_legends_selected (widp w,
                                               int32_t x, int32_t y,
                                               uint32_t button)
{
    wid_intro_menu_destroy();
    wid_hiscore_visible();

    return (true);
}

static uint8_t wid_menu_credits_selected (widp w,
                                          int32_t x, int32_t y,
                                          uint32_t button)
{
    wid_intro_menu_destroy();
    wid_intro_about_visible();

    return (true);
}

static void wid_intro_create (void)
{
    if (wid_intro) {
        return;
    }

    music_play_intro();

    wid_intro = wid_new_window("intro buttons");

    wid_set_no_shape(wid_intro);

    fpoint tl = {0.0f, 0.0f};
    fpoint br = {1.0f, 1.0f};
    wid_set_tl_br_pct(wid_intro, tl, br);

    color col = BLACK;
    col.a = 0;
    glcolor(col);

    wid_set_mode(wid_intro, WID_MODE_NORMAL);
    wid_set_color(wid_intro, WID_COLOR_TL, col);
    wid_set_color(wid_intro, WID_COLOR_BR, col);
    wid_set_color(wid_intro, WID_COLOR_BG, col);

    wid_intro_bg_create();
    wid_update(wid_intro);

    wid_move_to_pct_centered(wid_intro, 0.5f, 0.5f);
    wid_fade_in(wid_intro_background, intro_effect_delay*2);
    wid_fade_in(wid_intro_title, intro_effect_delay*2);
    wid_fade_in(wid_intro_man, intro_effect_delay*2);
    wid_fade_in(wid_intro_treasure_chest, intro_effect_delay*2);
    wid_fade_in(wid_intro_eyes, intro_effect_delay*2);

    wid_intro_menu_create();
}

static void wid_intro_menu_create (void)
{
    wid_destroy_delay_ms = 500;

    if (wid_intro_menu) {
        return;
    }

    wid_intro_menu = 
        wid_menu(0,
                 vvlarge_font,
                 large_font,
                 0, // on_update
                 0.5, /* x */
                 0.6, /* y */
                 1, /* columns */
                 saved_focus, /* focus */
                 7, /* items */

                 (int) 'e', "Editor", wid_menu_level_editor_selected,

                 (int) 'c', "Credits", wid_menu_credits_selected,

                 (int) 'S', "Settings", wid_menu_settings_selected,

                 (int) ' ', "Play game", wid_menu_play_game_selected,

                 (int) 's', "Quick start", wid_menu_quick_start_selected,

                 (int) 'h', "Hiscores", wid_menu_past_legends_selected,

                 (int) 'q', "Quit", wid_intro_quit_selected);

    {
        widp w = wid_popup(
                  "Version 0.0.1 uber alpha",
                  "",               /* title */
                  0.9f, 0.90f,      /* x,y postition in percent */
                  0,                /* title font */
                  vsmall_font,      /* body font */
                  0,                /* button font */
                  0);               /* number args */
        wid_set_no_shape(w);
        wid_destroy_in(w, 60000);
    }
}

static void wid_intro_menu_destroy (void)
{
    if (!wid_intro_menu) {
        return;
    }

    wid_menu_ctx *ctx = 
                    (typeof(ctx)) wid_get_client_context(wid_intro_menu);
    verify(ctx);

    /*
     * Save the focus so when we remake the menu we are at the same entry.
     */
    saved_focus = ctx->focus;

    wid_destroy(&wid_intro_menu);
}
